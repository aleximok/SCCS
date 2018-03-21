#ifndef __CDataSourceTextFile_h
#define __CDataSourceTextFile_h

#include "CCompare.h"


//
//	class CHashedString
//
//	Special aux lightweight case of std::string with fast comparing via hash-value
//

class CHashedString : public std::string
{
public:

	CHashedString()
	{
		mHashValue = 0;
	}

	CHashedString(const char *inData) :
		std::string(inData)
	{
		recalcHashValue();
	}

	CHashedString(const CHashedString &inData) :
		std::string(inData)
	{
		recalcHashValue();
	}

	virtual ~CHashedString()
	{
	}

	CHashedString& operator=(const CHashedString& _Right)
	{
		mHashValue = _Right.mHashValue;
		std::string::assign(_Right);
		return *this;
	}

	CHashedString& operator=(const char *_Right)
	{
		std::string::assign(_Right);
		recalcHashValue();
		return *this;
	}

	// accelerated compare func
	int compare(const CHashedString& _Right) const
	{
		return (_Right.getHashValue() == mHashValue) ?
			std::string::compare(_Right) : 1;
	}

	inline size_t getHashValue() const
	{
		return mHashValue;
	}

protected:

	void recalcHashValue()
	{
		mHashValue = std::hash<std::string>{}(*this);
	}

	size_t mHashValue;
};


// special case for a string pointer uses the length() method
// to determine a zero length string

inline bool isNull(const CHashedString *t)
{
	return (t == NULL);
}


// special case for a string pointer uses the compare() method
// to compare the two hash strings

inline bool isEqualTo(const CHashedString * const t1, const CHashedString * const t2)
{
	THROW_IF(t1 == nullptr || t2 == nullptr, XBadParameter);
	return (t1->compare(*t2) == 0);
}


//
//	class CDataSourceTextFile
//

class CDataSourceTextFile
{
public:
	// public typedef to define the data type
	typedef CHashedString data_type;

private:
	FILE                     *mFile;
	std::vector<CHashedString>  mData;

protected:

	// prevent compiler autogeneration
	CDataSourceTextFile();
	CDataSourceTextFile(const CDataSourceTextFile &);
	CDataSourceTextFile &operator=(const CDataSourceTextFile &);

public:

	explicit CDataSourceTextFile(FILE *file);

	virtual ~CDataSourceTextFile();

	// all data source classes must define the following interface
	void clearData ();
	bool getAt (size_t index, const data_type **data) const;
	const data_type *getBaseData () const { return NULL; }
	size_t getSize () const { return mData.size(); }
	void retrieveData ();
};


#endif  // __CDataSourceTextFile_h
