#include "stdafx.h"

#include "CSccsApplication.h"

#include <iostream>
#include <iomanip>
#include <string>

#include "CCompare.h"
#include "CDataSourceTextFile.h"

#include "CChangeSetBuilder.h"
#include "CChangeSetProcessor.h"


//
//	class CSccsApplication
//

CSccsApplication::CSccsApplication (int argc, char *argv []) :
		CApplication (argc, argv),
	mApply (false),
	mFile1 (NULL),
	mFile2 (NULL),
	mFileDiff (NULL)
{
}


CSccsApplication::~CSccsApplication ()
{
}


void
CSccsApplication::checkOption (const char *inOption)
{
	THROW_IF (strcmpi (inOption, "/apply"), XIllegalUsage);

	mApply = true;
}


void
CSccsApplication::outputUsage ()
{
	std::cout << "Usage 1:" << std::endl <<
		mArgv[0] << " input_file_1 input_file_2 changeset_file" << std::endl << std::endl <<
		"Usage 2:" << std::endl <<
		mArgv[0] << " input_file output_file changeset_file /apply" << std::endl << std::endl;
}


void
CSccsApplication::initialize ()
{
	// Here we have setted working mode, just check for proper arguments number/positions
	
	THROW_IF ((mApply) ? (mFirstKey != 4) : 
		(mFirstKey != 0  ||  mArgc != 4), XIllegalUsage);
	
	mFile1Name = mArgv [1];
	mFile2Name = mArgv [2];
	mFileDiffName = mArgv [3];

    mFile1 = fopen (mFile1Name.c_str (), "r");
	THROW_IF_NOT_WINFO (mFile1, XCantOpen, mFile1Name.c_str());
	
	mFile2 = fopen (mFile2Name.c_str (), (mApply) ? "w" : "r");
	THROW_IF_NOT_WINFO (mFile2, XCantOpen, mFile2Name.c_str());
	
	mFileDiff = fopen (mFileDiffName.c_str (), (mApply) ? "r" : "w");
	THROW_IF_NOT_WINFO (mFileDiff, XCantOpen, mFileDiffName.c_str ());
}


void
CSccsApplication::destroy ()
{
	// Clean up properly, close opened files & whatever
	
	if (mFile1 != NULL)
	{
		fclose (mFile1);
		mFile1 = NULL;
	}
	
	if (mFile2 != NULL)
	{
		fclose (mFile2);
		mFile2 = NULL;

		if (mReturnCode != RC_OK  &&  mApply)
		{
			// Output file generation failed, delete it
			
			unlink (mFile2Name.c_str ());
		}
	}
	
	if (mFileDiff != NULL)
	{
		fclose (mFileDiff);
		mFileDiff = NULL;
		
		if (mReturnCode != RC_OK  &&  ! mApply)
		{
			// Changeset file generation failed, delete it
			
			unlink (mFileDiffName.c_str ());
		}
	}
}


void
CSccsApplication::execute ()
{
//	cmp::testCharacterDiff ("abceghj", "abdbfehj");	// quick algo test

	if (mApply)
	{
		// Generate output file basing on changeset diff
		
		CChangeSetProcessor set_processor (mFile1, mFile2, mFileDiff);

		set_processor.process ();
	}
	else
	{
		// Generate changeset diff file basing on two reference input files
		
        // instantiate two text file data sources;one for each file

		CDataSourceTextFile compare_data1 (mFile1);
        CDataSourceTextFile compare_data2 (mFile2);
		
        // We need to instantiate a template compare object
        // create a typedef first so that we can use a short-handed
        // version later

		typedef cmp::CCompare<CDataSourceTextFile> CompareT;
        CompareT compare (&compare_data1, &compare_data2);
		
        int lcs;
        CompareT::CResultSet seq;
		
        // Process the data sources
        
		THROW_IF ((lcs = compare.process (&seq)) == -1, XComparisonFail);
		
		if (compare_data1.getSize () == 0)
		{
			THROW_WINFO (XEmptySource, mFile1Name.c_str());
		}
        else
        {
			CChangeSetBuilder set_builder (mFileDiff, compare_data1, compare_data2);
			
			set_builder.startConstruction ();

			// Loop through the result set and output the differing lines
			CompareT::CResultSet::iterator it  = seq.begin();
			CompareT::CResultSet::iterator ite = seq.end();
			
			bool b_identical = true;
			int line = 1;

			for (; it != ite; ++it)
			{
				CompareT::CResultSet::value_type res = *it;

				if (res->type () == cmp::kRemove)
				{
					DEBUG_STR (" -: ");

					set_builder.deleteLine (res->recNum () - 1);
					b_identical = false;
				}
				else if (res->type () == cmp::kInsert)
				{
					DEBUG_STR (" +: ");

					set_builder.insertLine (res->recNum () - 1);
					b_identical = false;
				}
				else
				{
					DEBUG_STR (" =: ");

					set_builder.skipLine ();
				}
				
				DEBUG_STR (std::setw (4) << res->recNum () << std::setw (4) <<
					line ++ << res->data().c_str() << std::endl);
			}
			
	        THROW_IF (b_identical, XFilesIdentical);
			
			set_builder.endConstruction ();
		}
	}
}

