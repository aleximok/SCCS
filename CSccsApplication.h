#ifndef __CSccsApplication_h
#define __CSccsApplication_h

#include "CApplication.h"

#include "stdio.h"

#include "XExceptions.h"

//
// User exceptions declaration part
//

DECLARE_EXCEPTION(XComparisonFail, XRuntime, "Comparison failed");

DECLARE_EXCEPTION(XEmptySource, XRuntime, "Empty source file");
DECLARE_EXCEPTION(XFilesIdentical, XRuntime, "Files are identical");


//
//	class CSccsApplication
//

class CSccsApplication : public CApplication
{
public:

    CSccsApplication (int argc, char *argv []);

    virtual ~CSccsApplication ();

    // Initialize/destroy affairs
    virtual void initialize ();
    virtual void destroy ();

    // Configurate application via options
    virtual void checkOption (const char *inOption);
    
    // Output usage line
    virtual void outputUsage ();
	
    // Handles main affairs happened between initialize () & detroy ()
    virtual void execute ();

protected:

	bool mApply;
	
	FILE *mFile1;
	FILE *mFile2;
	FILE *mFileDiff;

	std::string mFile1Name;
	std::string mFile2Name;
	std::string mFileDiffName;
};


#endif	// __CSccsApplication_h
