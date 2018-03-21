#ifndef __CChangeSetBuilder_h
#define __CChangeSetBuilder_h

#include <vector>

#include "CCompare.h"
#include "CDataSourceTextFile.h"

//
//	class CRange
//
//	Basic helper class, no virtual functions, no desctructor.
//

class CRange
{
public:

	CRange () : mL (0), mR (0) { }
	CRange (size_t from, size_t to) : mL (from), mR(to) { }
	CRange (const CRange &range) : mL (range.mL), mR (range.mR) { }

	inline void clear ()  { mL = mR = 0; }
	
	inline void set (size_t left, size_t right) { mL = left; mR = right; }

	inline bool isValid ()  { return (mL < mR); }

	inline size_t size () { return mR - mL; }

	inline bool isEnclose (const CRange &inRange) 
		{ return (mL <= inRange.mL  &&  mR >= inRange.mR);  }
			
	inline void shift (int offset) { mL += offset; mR += offset; }
	
	void extend (size_t inIndex)
	{
		// range naturally could be extended just by single line

		if (isValid ())
		{
			THROW_IF_NOT (mR == inIndex, XRangeError);
			mR ++; 
		}
		else
		{
			mL = inIndex;
			mR = inIndex + 1;
		}
	}

public:

	size_t mL, mR;
};


//
//	class CChangeSetBuilder
//
//  Keeps functionality to build change set
//

class CChangeSetBuilder
{
public:

	CChangeSetBuilder (
		FILE *inOutFile,
		CDataSourceTextFile &inSource,
		CDataSourceTextFile &inDest);
	
	virtual ~CChangeSetBuilder ();
	
	// Productive methods

	void insertLine (size_t inIndex);
	void deleteLine (size_t inIndex);
	void skipLine ();
	
	void outputString (const char *inStr, bool isCommand = false);
	
	void startConstruction ();
	void endConstruction ();
	
	// Check pattern for unambiguety and uniqueness

	bool isUnique (CRange &inRange);
	
	// Detect minimum and unique (unambiguous in context of source)
	// pattern enclosing mPosition

	void detectPattern (CRange &outRange);
	
	void pendingOps ();

protected:

	FILE *mOutFile;
    
	std::vector<const CHashedString *>  mData;
	
	CDataSourceTextFile &mSource;
	CDataSourceTextFile &mDest;
	
	size_t mPosition;

	CRange mToInsert;
	CRange mToDelete;
};

#endif	// __CChangeSetBuilder_h
