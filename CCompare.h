#ifndef __CCompare_h
#define __CCompare_h

#include <string>
#include <vector>
#include <algorithm>

#include "XExceptions.h"

namespace cmp {

// template function to determine if a pointer to a type points to
// a null value, we also return true if the pointer itself is null

template<typename T>
inline bool isNull (T *t)
{
    return t == 0  ||  *t == NULL;
}


// template class to determine if the contents of two pointer are equivalent

template<typename T>
inline bool isEqualTo (const T * const t1, const T * const t2)
{
    THROW_IF (t1 == nullptr  ||  t2 == nullptr, XBadParameter);
    return (*t1 == *t2);
}


//
//	class CDataSource
//
//	Basic data provider template class for integral types.
//	This class can be used to provide the CCompare class with
//	data from an array of integral types to be compared
//
//     const char str[] = "a string to compare";
//     cmp::CDataSource<const char> compareData1(str, strlen(str));
//

template<typename T>
class CDataSource
{
  public:

    // public typedef to define the data type
    typedef T data_type;

  private:
    const T *mData;         // pointer to the data block
    const T *mSource;       // pointer to the source data block
    size_t   mSize;         // length of the data block

  private:

	// prevent compiler autogeneration
	CDataSource();
    CDataSource(const CDataSource &);
	CDataSource &operator=(const CDataSource &);

  public:

    // taking a pointer to the data and the size of the data
    CDataSource(const T * const data, size_t size) :
		mData(0),
		mSource(data),
		mSize(size)
	{
	}

	virtual ~CDataSource() { }

    // all data source classes must define the following interface
    void clearData() { }
    
	bool getAt(size_t index, const data_type **data) const 
	{
		*data = &mData[index];
		return true;
	}

    const T * const getBaseData() const   { return mData; }
    size_t getSize() const { return (mData == 0)? 0 : mSize; }

	// fill the data buffer
    void retrieveData()
	{
		mData = mSource;
		mSource = 0;
	}
};


// public typedefs for the record types

enum CRecordType
{
    kUndefined=0,
    kKeep,       // record is in both
    kRemove,     // record is in the first, but not the second
    kInsert      // record is in the second, but not the first
};


//
//	class CResultType
//
//	Used to describe each result
//

template<typename T>
class CResultType
{
  private:
    size_t              mRecNum;  // record number
    CRecordType         mType;    // record type (see enum above)
    typename const T::data_type &mData;     // record data

  protected:

	// prevent compiler autogeneration
	CResultType();
    CResultType(const CResultType&);
	CResultType &operator=(const CResultType&);

  public:

    // just stores the data
    CResultType(size_t rec_num, CRecordType type, typename const T::data_type &data)
      : mRecNum(rec_num),
        mType(type),
        mData(data)
    {
    }

    ~CResultType() { }

    // public accessors
    size_t             recNum()  const  { return mRecNum; }
    CRecordType         type()    const  { return mType; }
	typename const T::data_type &data()   const  { return mData; }
};


//
//	class CCompare
//
//	Primary class that does all the work
//

template<typename T>
class CCompare
{
  public:

    // define a typedef for the results
    typedef std::vector<CResultType<T> *> CResultSet;

  private:
    short  *mArray;    // lcs working array
    T      *mSource;   // first data source
    T      *mDest;     // second data source

  private:

	// prevent compiler autogeneration
	CCompare();
    CCompare(const CCompare &);
	CCompare &operator=(const CCompare &);

  protected:
    short getResult(int col, int row) const
		{ return mArray[(row * mSource->getSize()) + col]; }

    bool  getResultSet(CCompare<T>::CResultSet *pseq) const;
    void  setResult(int col, int row, short v)
		{ mArray[(row * mSource->getSize ()) + col] = v; }

  public:

    CCompare(T *source, T *dest);
    ~CCompare();

    int  process(CCompare<T>::CResultSet *pseq);
};


// just stores the data pointers

template<typename T>
CCompare<T>::CCompare(T *source, T *dest)
  : mArray(NULL),
    mSource(source),
    mDest(dest)
{
}


// free allocated memory

template<typename T>
CCompare<T>::~CCompare<T>()
{
    if (mArray != NULL)
    {
        delete mArray;
        mArray = NULL;
    }
}


// this is the main function
// we calculate the lcs array and return the lcs length

template<typename T>
int CCompare<T>::process(CCompare<T>::CResultSet *pseq)
{
    mDest->clearData();
    mDest->retrieveData();
    mSource->clearData();
    mSource->retrieveData();

    // if we're at the end of both data streams,
    // then return -1 to indicate the end

    if (mSource->getSize () == 0  &&  mDest->getSize () == 0)
    {
        return -1;
    }

    // if the two data sources are equal size, then try
    // a quick memcmp first. this avoids unnecessary
    // processing using the slower lcs algorithm

    const void * const baseData1 = mSource->getBaseData ();
    const void * const baseData2 = mDest->getBaseData ();

    if (baseData1 != NULL  &&  baseData2 != NULL
	    &&  mSource->getSize() == mDest->getSize()
		&&  memcmp(baseData1, baseData2, mSource->getSize()) == 0)
    {
		LOG_LINE ("Identical sources");
        return mSource->getSize();
    }

    // allocate an array for the results. if the allocation
    // fails, return -1 to indicate failure

    if (mArray != NULL)
    {
        delete mArray;
        mArray = NULL;
    }

    // calculate the size of the lcs working array

    size_t size = (1 + mSource->getSize()) * (1 + mDest->getSize());

    if (size > 1)
    {
		LOG_LINE("Allocating "  << size << " bytes");
        mArray = new short[size];
        THROW_IF (mArray == NULL, XNotEnoughMemory);
		
        LOG_LINE("Compare< " << typeid(T).name() << "> processing type \'" <<
			typeid(T::data_type).name() << "\'");

        // initialise the array
        memset(mArray, 0x0, size);

        // work through the array, right to left, bottom to top
        int col,row;
        for (col = mSource->getSize(); col >= 0; --col)
        {
            for (row=mDest->getSize(); row >= 0; --row)
            {
                const T::data_type *data1, *data2;

                // get the data at the current col,row for each data source
                if (mSource->getAt(col, &data1)  &&  mDest->getAt(row, &data2))
                {
                    if (isNull(data1)  ||  isNull(data2))
                    {
                        // if either data is null, set the array entry to zero
                        this->setResult(col, row, 0);
                    }
                    else if (cmp::isEqualTo(data1, data2))
                    {
                        // if the data for each source is equal, then add one
                        // to the value at the previous diagonal location - to
                        // the right and below - and store it in the current location
                        this->setResult(col, row, short(1) + this->getResult(col+1, row+1));
                    }
                    else
                    {
                        // if the data is not null and not equal, then copy
                        // the maximum value from the two cells to the right
                        // and below, into the current location
                        this->setResult(col, row, std::max(this->getResult(col + 1, row),
                                                                 this->getResult(col, row + 1)));
                    }

                }
                else
                    return -1;      // something went wrong
            }   // each row
        }       // each column
    }

    // return the length of the LCS
    return this->getResultSet(pseq)? this->getResult(0, 0) : -1;
}


// construct result set and return to the caller

template<typename T>
bool CCompare<T>::getResultSet(CCompare<T>::CResultSet *pseq) const
{
    size_t col = 0;
    size_t row = 0;
    const T::data_type *data1, *data2;

	size_t ncols = mSource->getSize();
	size_t nrows = mDest->getSize();
	
    while (col < ncols  ||  row < nrows)
    {
		mSource->getAt(col, &data1);
		mDest->getAt(row, &data2);
		
        if (col < ncols  &&  row < nrows  &&  cmp::isEqualTo(data1, data2))
		{
			THROW_IF(isNull(data1)  ||  isNull(data2), XRuntime);
			
			col ++;
			row ++;

			pseq->push_back(new CResultType<T>(col, cmp::kKeep, *data1));
		}
		else if (col < ncols  &&
			(row == nrows  ||  this->getResult(col+1, row) > this->getResult(col, row+1)))
		{
			THROW_IF(isNull(data1), XRuntime);
			col ++;

			pseq->push_back(new CResultType<T>(col, cmp::kRemove, *data1));
		}
		else if (row < nrows && 
			(col == ncols  ||  this->getResult(col+1, row) <= this->getResult(col, row+1)))
		{
			THROW_IF(isNull(data2), XRuntime);
			row ++;

			pseq->push_back(new CResultType<T>(row, cmp::kInsert, *data2));
		}
        else
            return false;       // something went wrong
	}

    return true;
}


// small test function to calculate the difference
// between two character strings

inline void testCharacterDiff(const char * const str1, const char * const str2)
{
    // instantiate data source objects for each text string
    cmp::CDataSource<const char> compare_data1(str1, strlen(str1));
    cmp::CDataSource<const char> compare_data2(str2, strlen(str2));

    // we need to instantiate a template compare object
    // create a typedef first so that we can use a short-handed
    // version later
    typedef cmp::CCompare< cmp::CDataSource<const char> > compare_t;
    compare_t compare(&compare_data1, &compare_data2);

    size_t                lcs;
    std::string           result_str1, result_str2;
    compare_t::CResultSet seq;

    // process the data sources
    if ((lcs = compare.process(&seq)) != -1)
    {
        printf("Comparing: \"%s\" & \"%s\"\n"
               "Longest Common Subsequence length is %d\n", str1, str2, lcs);

        compare_t::CResultSet::iterator it   = seq.begin();
        compare_t::CResultSet::iterator ite  = seq.end();
        for (; it != ite; ++it)
        {
            compare_t::CResultSet::value_type res = *it;
            if (res->type() == cmp::kRemove)
            {
                result_str1 += res->data();
                result_str2 += '_';
            }
            else if (res->type() == cmp::kInsert)
            {
                result_str1 += '_';
                result_str2 += res->data();
            }
            else
            {
                result_str1 += res->data();
                result_str2 += res->data();
            }
        }
        seq.erase(seq.begin(), seq.end());
    }

    printf("%s\n%s\n\n", result_str1.c_str(), result_str2.c_str());
}

}   // namespace cmp

#endif  // __CCompare_h
