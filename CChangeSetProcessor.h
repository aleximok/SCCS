#ifndef __CChangeSetProcessor_h
#define __CChangeSetProcessor_h

#include <vector>

#include "CCompare.h"
#include "CDataSourceTextFile.h"

DECLARE_EXCEPTION(XBadDiff, XRuntime, "Corrupted change set file");
DECLARE_EXCEPTION(XContextNotFound, XRuntime, "Context not found");
DECLARE_EXCEPTION(XAmbiguousContext, XRuntime, "Context not unique");


//
//	class CChangeSetProcessor
//
//  Keeps functionality to process change set and create Tb via Ta->(Cab)->Tb
//

class CChangeSetProcessor
{
public:

	enum
	{
		kNone = 0,
		kBegin = 1,
		kEnd = 2,
		kInsert = 3,
		kReplace = 4,
		kDelete = 5,
		kBetween = 6,
		kAnd = 7,
		kWith = 8
	};

	CChangeSetProcessor(
		FILE *inFile1,		// reference file
		FILE *inFile2,		// file to write to
		FILE *inSetFile		// instruction changeset file
	);

	virtual ~CChangeSetProcessor();

	void addPattern(std::vector<CHashedString> &inBuffer);

	bool readString(FILE *inFile, CHashedString &outString);

	short readCommandPart(std::vector<CHashedString> *outBuffer = NULL);

	// Check pattern for uniqueness and presence,
	// locate it position in source, throw an exception if something wrong

	size_t checkPattern();

	void insertContext(size_t position, std::vector<CHashedString> &inBuffer);
	void deleteContext(size_t position, size_t nlines);

	// Main procession

	void process();

	// Output current content of mData

	void outputResult();

protected:

	FILE * mFile1;
	FILE *mFile2;
	FILE *mSetFile;

	std::vector<CHashedString>  mData;	// processed data (from source to dest)

	std::vector<CHashedString>  mWhat;	// command buffer
	std::vector<CHashedString>  mBefore;	// note, 'before' comes before insert position
	std::vector<CHashedString>  mAfter;

	std::vector<const CHashedString *>  mPattern;
};

#endif	// __CChangeSetProcessor_h
