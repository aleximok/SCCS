#include "stdafx.h"

#include "CChangeSetBuilder.h"
#include "CDataSourceTextFile.h"


//
//	class CChangeSetBuilder
//
//  Keeps functionality to build change set
//

CChangeSetBuilder::CChangeSetBuilder (
	FILE *inOutFile,
	CDataSourceTextFile &inSource,
	CDataSourceTextFile &inDest) :

	mOutFile (inOutFile), mSource (inSource), mDest (inDest)
{
	THROW_IF_NOT(mOutFile  &&  &mSource  &&  &mDest, XBadParameter);
}


CChangeSetBuilder::~CChangeSetBuilder ()
{
	mData.clear ();
}


void
CChangeSetBuilder::insertLine (size_t inIndex)
{
	// Do not operate immediately, just store the change
	mToInsert.extend (inIndex);
}


void
CChangeSetBuilder::deleteLine (size_t inIndex)
{
	// Do not operate immediately, just store the change
	mToDelete.extend (inIndex);
}


void
CChangeSetBuilder::skipLine ()
{
	// Check if we have pending operations

	pendingOps ();

	mPosition ++;
}


// Output string to file with end of line

void
CChangeSetBuilder::outputString (const char *inStr, bool isCommand)
{
	THROW_IF (inStr == NULL, XBadParameter);
	
	if (! isCommand)
	{
		THROW_IF (fputs ("> ", mOutFile) == EOF, XCantWrite);
	}

	THROW_IF (fputs (inStr, mOutFile) == EOF, XCantWrite);
	THROW_IF (fputc ('\n', mOutFile) == EOF, XCantWrite);
}


void
CChangeSetBuilder::startConstruction ()
{
	mPosition = 0;

	// Fill it with source content

	for (size_t i = 0; i < mSource.getSize (); i++)
	{
		const CHashedString *data;
		THROW_IF_NOT_W (mSource.getAt (i, &data), XUnknown);

		mData.push_back (data);
	}

	outputString ("[BEGIN]", true);
}


void
CChangeSetBuilder::endConstruction ()
{
	// Check if we have pending operations
	
	pendingOps ();

	outputString ("[END]", true);
}


// Check pattern for unambiguety and uniqueness

bool
CChangeSetBuilder::isUnique (CRange &inRange)
{
	size_t dataSize = mData.size ();
	size_t rangeSize = inRange.size ();

	for (size_t i = 0; i <= dataSize - rangeSize; i++)
	{
		if (i == inRange.mL)
		{
			continue;
		}
		
		size_t j;

		for (j = 0; j < rangeSize  &&
			mData [inRange.mL + j]->compare (* mData [i + j] ) == 0; j++)
		{
		}

		if (j == rangeSize)
		{
			return false;		// matched!
		}
	}

	return true;
}


// Detect minimum and unique (unambiguous in context of source)
// pattern enclosing mPosition

void
CChangeSetBuilder::detectPattern (CRange &outRange)
{
	size_t dataSize = mData.size ();
	
	CRange initRange (outRange);

	bool extRight = true;
	
	// Try to extend pattern range to the left/right by moving bound on 1
	// until we found unique one
	
	while (! isUnique (outRange))
	{
		if ((extRight  ||  outRange.mL == 0)  &&  outRange.mR < dataSize)
		{
			outRange.mR ++;
			extRight = false;
		}
		else if (outRange.mL > 0)
		{
			outRange.mL --;
			extRight = true;
		}
		else THROW_WINFO (XRuntime, "detectPattern: bad flag flipping state");
	}
	
	if (outRange.size () <= 2)
	{
		// Nothing to do anymore

		return;
	}

	// Now we have unique pattern, then try to make it as small as possible
	
	CRange tmpRange (outRange);

	do 
	{
		// Store result
		
		outRange = tmpRange;

		// Which operation was the last?
		
		if (extRight)
		{
			// Last change happen on left bound, shrink it back
			
			tmpRange.mR --;
		}
		else
		{
			tmpRange.mL ++;
		}
	}
	while (tmpRange.isEnclose (initRange)  &&  isUnique (tmpRange));
}


void
CChangeSetBuilder::pendingOps ()
{
	CRange target;
	const CHashedString *data;
	size_t i;

	if (mToInsert.isValid ())
	{
		if (mToDelete.isValid ())
		{
			// Replace

			target.set (mPosition, mPosition + mToDelete.size ());
			
			detectPattern (target);
			
			outputString ("[REPLACE]", true);
			
			for (i = target.mL; i < target.mR; i++)
			{
				outputString (mData [i]->c_str ());
			}
		
			mData.erase (mData.begin () + mPosition, 
				mData.begin () + mPosition + mToDelete.size ());
			
			for (i = mToInsert.mL; i < mToInsert.mR; i++)
			{
				THROW_IF_NOT_W (mDest.getAt (i, &data), XUnknown);
				
				mData.insert (mData.begin () + mPosition, data);

				mPosition ++;
			}
			
			outputString ("[WITH]", true);

			target.mR += mToInsert.size () - mToDelete.size ();

			for (i = target.mL; i < target.mR; i++)
			{
				outputString (mData [i]->c_str ());
			}
		}
		else
		{
			// Insert

			target.mL = (mPosition > 0) ? mPosition - 1 : mPosition;
			target.mR = (mPosition < mData.size ()) ? mPosition + 1 : mPosition;
			
			detectPattern (target);

			outputString ("[INSERT]", true);
			
			// Note, that "before-after" is a single unique context

			CRange before (target.mL, mPosition);
			CRange after (mPosition, target.mR);
				
			for (i = mToInsert.mL; i < mToInsert.mR; i++)
			{
				THROW_IF_NOT_W (mDest.getAt (i, &data), XUnknown);
				
				outputString (data->c_str ());
				mData.insert (mData.begin () + mPosition, data);

				mPosition ++;
			}
			
			after.shift (mToInsert.size ());
			
			outputString ("[BETWEEN]", true);
			
			for (i = before.mL; i < before.mR; i++)
			{
				outputString (mData [i]->c_str ());
			}
			
			outputString ("[AND]", true);
			
			for (i = after.mL; i < after.mR; i++)
			{
				outputString (mData [i]->c_str ());
			}
		}
	}
	else if (mToDelete.isValid ())
	{
		// Delete
		
		target.mL = (mPosition > 0) ? mPosition - 1 : mPosition;
		target.mR = mPosition + mToDelete.size ();
		target.mR += (target.mR < mData.size ()) ? 1 : 0;
		
		detectPattern (target);
		
		outputString ("[DELETE]", true);
		
		// Note, that "before-delete-after" is a single unique context
		// and delete is a legal part of it!
		
		CRange before (target.mL, mPosition);
		CRange after (mPosition, target.mR - mToDelete.size ());
		
		for (i = mToDelete.mL; i < mToDelete.mR; i++)
		{
			THROW_IF_NOT_W (mSource.getAt (i, &data), XUnknown);
			
			outputString (data->c_str ());
		}
		
		mData.erase (mData.begin () + mPosition, 
			mData.begin () + mPosition + mToDelete.size ());
		
		outputString ("[BETWEEN]", true);
		
		for (i = before.mL; i < before.mR; i++)
		{
			outputString (mData [i]->c_str ());
		}
		
		outputString ("[AND]", true);
		
		for (i = after.mL; i < after.mR; i++)
		{
			outputString (mData [i]->c_str ());
		}
	}
	
	mToDelete.clear ();
	mToInsert.clear ();
}
