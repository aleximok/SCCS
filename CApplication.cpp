#include "stdafx.h"

#include "CApplication.h"


// Constructor for applications that doesn't takes arguments

CApplication::CApplication () :
    mArgc (0),
    mArgv (NULL),
    mFirstKey (0),
    mReturnCode (RC_ERROR)
{
}


// Constructor for applications that do parameters parsing

CApplication::CApplication (int argc, char *argv []) :
    mArgc (argc),
    mArgv (argv),
    mFirstKey (0),
    mReturnCode (RC_ERROR)
{
}


CApplication::~CApplication ()
{
}


// Method that represent whole application lifecycle process

void
CApplication::run ()
{
    XTRY
	{
        // Check whether application uses parameters...
        
        if (mArgv != NULL)
        {
            // Iterate options list
            
			for (int i = 1; i < mArgc; i++)
			{
				THROW_IF (strncmp (mArgv [i], "/?", 2) == 0, XIllegalUsage);

				if (*mArgv [i] == '/')
				{
					if (mFirstKey == 0)
					{
						mFirstKey = i;
					}
					
					// Check command-line option
					
					checkOption (mArgv [i]);
				}
				else
				{
					// Bad parameters sequence

					THROW_IF (mFirstKey != 0, XIllegalUsage);
				}
			}
        }
        
        try
        {
            // Now, do whatever initialization
            
            initialize ();
            
            // Here's going main processing
            
            execute ();

            setReturnCode (RC_OK);
        }
        catch (...)
        {
			// Whatever thing happens during initializing/execution we have to cleanup everything.
			// Note! It must be safe to call destroy () even if there some or even whole stuff
			// remains uninitialized.

			XTRY
			{
				// Normally, destroy () shouldn't throw any exception!
				// So, we suppress any accidentally thrown exception
				// and finally restore the original ex

				destroy();
			}
			XEND
            
            // Rethrow exception caused execution termination
            
            throw;
        }

        // Destroy everything
        
		XTRY
		{
			destroy();
		}
		XEND
	}
    XCATCH (XIllegalUsage, ex)
    {
        outputUsage ();
    }
	XCATCH_END
	XEND
}


// Default output usage line

void
CApplication::outputUsage ()
{
    if (mArgv != NULL)
    {
		std::cerr << "Usage: " << mArgv [0] << " /?" << std::endl;
    }
    
    // if program doesn't takes an arguments, it has no usage line...
}
