#include "stdafx.h"

#include "CDataSourceTextFile.h"

//
//	class CDataSourceTextFile
//

CDataSourceTextFile::CDataSourceTextFile(FILE *file)
	: mFile(file)
{
}


CDataSourceTextFile::~CDataSourceTextFile()
{
	clearData();
}


// Get the data at a specified offset
bool
CDataSourceTextFile::getAt(size_t index, const data_type **data) const
{
	// the caller will request one beyond the end, so return NULL
	if (index >= mData.size())
		*data = NULL;
	else
		*data = &mData.at(index);

	return true;
}


// clear the data buffer
void
CDataSourceTextFile::clearData()
{
	mData.erase(mData.begin(), mData.end());
}


// Read the data into the buffer
void CDataSourceTextFile::retrieveData()
{
	char buffer[4096];

	THROW_IF_NOT(mData.size() == 0, XRuntime);
	while (!feof(mFile))
	{
		buffer[0] = '\0';

		if (fgets(buffer, sizeof(buffer) / sizeof(buffer[0]), mFile) == NULL)
		{
			THROW_IF_NOT(feof(mFile), XCantRead);
			break;
		}

		// remove trailing \n or \r

		char c;
		int len;

		while (((c = buffer[(len = strlen(buffer)) - 1]) == '\n') || (c == '\r'))
		{
			buffer[len - 1] = '\0';
		}

		mData.push_back(buffer);
	}
}
