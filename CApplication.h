#ifndef __CApplication_h
#define __CApplication_h

#include <string>
#include <time.h>

#include "XExceptions.h"


// Common return codes

#define RC_OK 0
#define RC_ERROR -1


//
//	Common application command-line format is:
//
//	application.exe arg1 arg2 .. argN /key1 /key2 .. /keyM
//
//  By default /? outputs command-line usage
//


//
//	class CApplication
//
//	Just base class for all other applications...
//

class CApplication
{
public:

    CApplication ();
    CApplication (int argc, char *argv []);

    virtual ~CApplication ();
    
    // Method that represent whole application lifecycle process
    virtual void run ();
    
    // Initialize/destroy affairs
    virtual void initialize () { }
    virtual void destroy () { }

	// Handles main affairs happened between initialize () & destroy ()
    virtual void execute () { }
    
    // Configurate application via options
    virtual void checkOption (const char *inOption) { }
    
    // Output usage line
    virtual void outputUsage ();

	virtual void setReturnCode (int inCode)
	{
		mReturnCode = inCode;
	}
    
	virtual int getReturnCode ()
	{
		return mReturnCode;
	}
    
private:

	void handleDestroy ();

protected:
    
	int mArgc;
	char **mArgv;
    
	int mFirstKey;
	int mReturnCode;
};


#endif // __CApplication_h
