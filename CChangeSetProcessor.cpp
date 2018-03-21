#include "stdafx.h"

#include "CChangeSetProcessor.h"

#include "CSccsApplication.h"


//
//	class CChangeSetProcessor
//
//  Keeps functionality to process change set and create Tb via Ta->(Cab)->Tb
//

CChangeSetProcessor::CChangeSetProcessor(
	FILE *inFile1,		// reference file
	FILE *inFile2,		// file to write to
	FILE *inSetFile		// instruction changeset file
) :
	mFile1(inFile1), mFile2(inFile2), mSetFile(inSetFile)
{
}


CChangeSetProcessor::~CChangeSetProcessor()
{
}


void
CChangeSetProcessor::addPattern(std::vector<CHashedString> &inBuffer)
{
	for (size_t i = 0; i < inBuffer.size(); i++)
	{
		mPattern.push_back(&inBuffer.at(i));
	}
}


bool
CChangeSetProcessor::readString(FILE *inFile, CHashedString &outString)
{
	THROW_IF(inFile == NULL, XBadParameter);

	char buffer[4096] = "";

	if (fgets(buffer, sizeof(buffer) / sizeof(buffer[0]), inFile) == NULL)
	{
		THROW_IF_NOT(feof(inFile), XCantRead);

		outString = "";
		return false;
	}

	// remove trailing \n or \r

	char c;
	int len;

	while (((c = buffer[(len = strlen(buffer)) - 1]) == '\n') || (c == '\r'))
	{
		buffer[len - 1] = '\0';
	}

	outString = buffer;

	return true;
}


short
CChangeSetProcessor::readCommandPart(std::vector<CHashedString> *outBuffer)
{
	if (outBuffer != NULL)
	{
		outBuffer->clear();
	}

	for (;;)
	{
		CHashedString str;

		THROW_IF_NOT_WINFO(readString(mSetFile, str), XBadDiff, "Expecting command part");

		if (str[0] == '[')
		{
			// Strip trailing whitespaces if any

			int pos = str.find_first_of(" \t");

			if (pos != std::string::npos)
			{
				str.erase(pos);
			}

			if (strcmp(str.c_str(), "[BEGIN]") == 0)	return kBegin;
			else if (strcmp(str.c_str(), "[END]") == 0)	return kEnd;
			else if (strcmp(str.c_str(), "[INSERT]") == 0)	return kInsert;
			else if (strcmp(str.c_str(), "[REPLACE]") == 0)	return kReplace;
			else if (strcmp(str.c_str(), "[DELETE]") == 0)	return kDelete;
			else if (strcmp(str.c_str(), "[BETWEEN]") == 0)	return kBetween;
			else if (strcmp(str.c_str(), "[AND]") == 0)	return kAnd;
			else if (strcmp(str.c_str(), "[WITH]") == 0)	return kWith;

			// Unrecognized reserved word

			str.insert(0, "Unrecognized command: ");

			THROW_WINFO(XBadDiff, str.c_str());
		}
		else
		{
			THROW_IF_NOT_WINFO(outBuffer, XBadDiff, "Command word expected");

			// Every non-command line must have prefix "> "

			THROW_IF_WINFO(strncmp(str.c_str(), "> ", 2), XBadDiff,
				"Non-command line without '> ' prefix");

			// Remove prefix and addline to buffer

			str.erase(0, 2);
			outBuffer->push_back(str);
		}
	}
}


// Check pattern for uniqueness and presence,
// locate it position in source, throw an exception if something wrong

size_t
CChangeSetProcessor::checkPattern()
{
	size_t dataSize = mData.size();
	size_t patternSize = mPattern.size();

	size_t position = -1;
	bool b_found = false;

	for (size_t i = 0; i <= dataSize - patternSize; i++)
	{
		size_t j;

		for (j = 0; j < patternSize  &&
			mPattern[j]->compare(mData[i + j]) == 0; j++)
		{
		}

		if (j == patternSize)
		{
			// Context is not unique? Output 1st line of it and bail out

			THROW_IF_WINFO(b_found, XAmbiguousContext, mPattern[0]->c_str());
			position = i;

			b_found = true;
		}
	}

	THROW_IF_NOT_WINFO(b_found, XContextNotFound, mPattern[0]->c_str());

	return position;
}


void
CChangeSetProcessor::insertContext(size_t position, std::vector<CHashedString> &inBuffer)
{
	for (size_t i = 0; i < inBuffer.size(); i++)
	{
		mData.insert(mData.begin() + position + i, inBuffer[i]);
	}
}


void
CChangeSetProcessor::deleteContext(size_t position, size_t nlines)
{
	mData.erase(mData.begin() + position, mData.begin() + position + nlines);
}


// Main procession
// [BEGIN] starts procession, and we don't care what happens after {END}

void
CChangeSetProcessor::process()
{
	// Load whole source file into memory for further operation,
	// suppose files just opened and we do not need to rewind pointer

	CHashedString str;

	while (readString(mFile1, str))
	{
		mData.push_back(str);
	}

	// OK, first line of changeset must be [BEGIN]

	THROW_IF_WINFO(readCommandPart() != kBegin, XBadDiff, "No [BEGIN] at file start");

	short cmd = readCommandPart();

	for (;;)
	{
		size_t pos;

		switch (cmd)
		{
		case kEnd:

			// This is the end, my only friend -- the end

			outputResult();
			return;

		case kInsert:

			cmd = readCommandPart(&mWhat);
			THROW_IF_NOT_WINFO(cmd == kBetween, XBadDiff, "[BETWEEN] expected");

			cmd = readCommandPart(&mBefore);
			THROW_IF_NOT_WINFO(cmd == kAnd, XBadDiff, "[AND] expected");

			// Command stored for next iteration

			cmd = readCommandPart(&mAfter);

			// Fill pattern

			mPattern.clear();

			addPattern(mBefore);
			addPattern(mAfter);

			pos = checkPattern();

			// Update output file content

			insertContext(pos + mBefore.size(), mWhat);

			break;

		case kDelete:

			cmd = readCommandPart(&mWhat);
			THROW_IF_NOT_WINFO(cmd == kBetween, XBadDiff, "[BETWEEN] expected");

			cmd = readCommandPart(&mBefore);
			THROW_IF_NOT_WINFO(cmd == kAnd, XBadDiff, "[AND] expected");

			// Command stored for next iteration

			cmd = readCommandPart(&mAfter);

			// Fill pattern

			mPattern.clear();

			addPattern(mBefore);
			addPattern(mWhat);
			addPattern(mAfter);

			pos = checkPattern();

			// Update output file content

			deleteContext(pos + mBefore.size(), mWhat.size());

			break;

		case kReplace:

			cmd = readCommandPart(&mWhat);
			THROW_IF_NOT_WINFO(cmd == kWith, XBadDiff, "[WITH] expected");

			// Command stored for next iteration

			cmd = readCommandPart(&mBefore);

			// Fill pattern

			mPattern.clear();

			addPattern(mWhat);

			pos = checkPattern();

			// Update output file content
			// mBefore keeps 'WITH' part

			deleteContext(pos, mWhat.size());
			insertContext(pos, mBefore);

			break;
		}
	}
}


// Output current content of mData

void
CChangeSetProcessor::outputResult()
{
	THROW_IF_NULL(mFile2);

	size_t dataSize = mData.size();

	// Avoid output of \n for last line

	if (dataSize > 0)
	{
		THROW_IF(fputs(mData[0].c_str(), mFile2) == EOF, XCantWrite);
	}

	for (size_t i = 1; i < dataSize; i++)
	{
		THROW_IF(fputc('\n', mFile2) == EOF, XCantWrite);
		THROW_IF(fputs(mData[i].c_str(), mFile2) == EOF, XCantWrite);
	}
}
