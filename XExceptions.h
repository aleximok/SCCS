//
//	2018  Oleksii Mokrintsev
//
//	MIT License
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files(the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions :
//
//	The above copyright notice and this permission notice shall be included in all
//	copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//	SOFTWARE.
//

#ifndef __XEXCEPTIONS_H
#define __XEXCEPTIONS_H

#include <iostream>
#include <ios>
#include <exception>
#include <functional>

#if _DEBUG
	#define LOG_LINE(s) std::cerr << __FILE__ << ':' << __LINE__ << " \"" << __func__ << "\" " << s << std::endl;
	#define LOG_STR(s) std::cerr << s;
#else
	#define LOG_LINE(s)
	#define LOG_STR(s)
#endif


// Macro X_SOURCE_DEBUG defines whether we have to keep source debugging info
// like __FILE__, __LINE__ and __func__ within exception object

#if !defined(X_SOURCE_DEBUG)
	#define X_SOURCE_DEBUG _DEBUG
#endif

#if X_SOURCE_DEBUG
	#define X_EXC_PARAMS __FILE__, __LINE__,  __func__
	#define X_EXC_PARAMS_AUX , __FILE__, __LINE__,  __func__
#else
	#define X_EXC_PARAMS
	#define X_EXC_PARAMS_AUX
#endif


#define TRY try

#define THROW(x) \
{ \
	LOG_LINE (x::sExceptionName << " raised, cause: " << x::sExceptionMessage << std::endl); \
	throw x (X_EXC_PARAMS); \
}

#define THROW_NODEBUG(x) \
{ \
	throw x (X_EXC_PARAMS); \
}

#define THROW_WINFO(x, str)\
{ \
	LOG_LINE (x::sExceptionName << " raised, cause: " << \
		x::sExceptionMessage << ", info: " << (str) << std::endl); \
	throw x (str X_EXC_PARAMS_AUX);\
}

// Catch exception  without debugging messages
#define CATCH_NODEBUG(x, ex) catch (x &ex) \
{ \

// Casual catch
#define CATCH(x, ex) catch (x &ex) \
{ \
	LOG_LINE (ex.who () << " caught" << std::endl); \

// std::exception catch
#define CATCH_STD(ex) catch (std::exception &ex) \
{ \
	LOG_LINE (typeid (ex).name () << " caught" << std::endl); \

// End catch definition
#define END_CATCH }


// Though large character arrays kept within exception
// object, we'll going to engage only stack, not heap space

#define DEFAULT_EXCEPTION_INFO_STR_SIZE 2048
#define DEFAULT_EXCEPTION_FILE_STR_SIZE 512
#define DEFAULT_EXCEPTION_FUNC_STR_SIZE 256


//
//	class XException
//
//	Base root exceptions class declaration
//

class XException : public std::exception
{
public:

	XException () noexcept
	{
		mInfoString [0] = '\0';
	}

	explicit XException (const char * const infoStr) noexcept
	{
		strncpy_s (mInfoString, sizeof (mInfoString), infoStr, sizeof (mInfoString) - 1);
	}

	XException (XException const &other) noexcept
		: std::exception (other)
	{
		strncpy_s (mInfoString, sizeof (mInfoString), other.mInfoString, sizeof (mInfoString) - 1);

#if X_SOURCE_DEBUG
		strncpy_s (mFileString, sizeof (mFileString), other.mFileString, sizeof (mFileString) - 1);
		strncpy_s (mFuncString, sizeof (mFuncString), other.mFuncString, sizeof (mFuncString) - 1);
		mLine = other.mLine;
#endif
	}

	XException &operator= (XException const &other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		
		std::exception::operator= (other);
		strncpy_s (mInfoString, sizeof (mInfoString), other.mInfoString, sizeof (mInfoString) - 1);

#if X_SOURCE_DEBUG
		strncpy_s (mFileString, sizeof (mFileString), other.mFileString, sizeof (mFileString) - 1);
		strncpy_s (mFuncString, sizeof (mFuncString), other.mFuncString, sizeof (mFuncString) - 1);
		mLine = other.mLine;
#endif

		return *this;
	}
	
#if X_SOURCE_DEBUG
	explicit XException (const char * const inFile, int inLine, const char * const inFunc) noexcept
	{
		mInfoString[0] = '\0';
		strncpy_s (mFileString, sizeof (mFileString), inFile, sizeof (mFileString) - 1);
		strncpy_s (mFuncString, sizeof (mFuncString), inFunc, sizeof (mFuncString) - 1);
		mLine = inLine;
	}

	explicit XException (const char * const infoStr, const char * const inFile, int inLine,
		const char * const inFunc) noexcept
	{
		strncpy_s (mInfoString, sizeof (mInfoString), infoStr, sizeof (mInfoString) - 1);
		strncpy_s (mFileString, sizeof (mFileString), inFile, sizeof (mFileString) - 1);
		strncpy_s (mFuncString, sizeof (mFuncString), inFunc, sizeof (mFuncString) - 1);
		mLine = inLine;
	}
#endif

	virtual ~XException () noexcept {}

	virtual char const *who () const noexcept { return sExceptionName; }
	virtual char const *what () const noexcept { return sExceptionMessage; }
	virtual const char *const info () const noexcept { return mInfoString; }

#if X_SOURCE_DEBUG
	virtual const char *file () const noexcept { return mFileString; }
	virtual const char *func () const noexcept { return mFuncString; }
	virtual int line () const noexcept { return mLine; }
#endif

	virtual operator const char * const () const noexcept { return what (); }

	static constexpr const char *sExceptionName = "XExceptionExc";
	static constexpr const char *sExceptionMessage = "Generic failure";

protected:

	char mInfoString [DEFAULT_EXCEPTION_INFO_STR_SIZE];

#if X_SOURCE_DEBUG
	char mFileString [DEFAULT_EXCEPTION_FILE_STR_SIZE];
	char mFuncString [DEFAULT_EXCEPTION_FUNC_STR_SIZE];
	int mLine;
#endif
};


//
//	Following let us declare/implement exception derived classes and
//  build tree-like class structure, which is provide easy and powerful
//  way to develop and control exception hadling
//

template <class XBase, class XStrTraits>
class TXException : public XBase, public XStrTraits
{
public:

	TXException () : XBase ()
	{
	}

	explicit TXException (const char * const infoStr) noexcept :
		XBase (infoStr)
	{
	}

	TXException (TXException const &other) noexcept :
		XBase (other)
	{
	}

#if X_SOURCE_DEBUG
	explicit TXException(const char * const inFile, int inLine, const char * const inFunc) noexcept :
		XBase (inFile, inLine, inFunc)
	{
	}

	explicit TXException(const char * const infoStr, const char * const inFile, int inLine,
		const char * const inFunc) noexcept :
			XBase (infoStr, inFile, inLine, inFunc)
	{
	}
#endif

	using XStrTraits::sExceptionName;
	using XStrTraits::sExceptionMessage;

	virtual char const *who () const noexcept { return XStrTraits::sExceptionName; }
	virtual char const *what () const noexcept { return XStrTraits::sExceptionMessage; }
};


#define DECLARE_EXCEPTION(exceptionName, exceptionBase, errorCause) \
struct __Str##exceptionName { \
	static constexpr const char *sExceptionName = #exceptionName; \
	static constexpr const char *sExceptionMessage = errorCause; }; \
typedef TXException<exceptionBase, __Str##exceptionName> exceptionName;


//
//	Exception "caged" catching
//
//	Somewhere/sometimes you have to provide	global catch
//	for all exceptions which possibly could arise in your app.
//	It looks like a 'try { ... }' with a number of
//	catch-blocks handling all kind of exceptions.
//	Usually, it is lengthy and verbose. Following stuff
//	let you minimize all these manipulations providing
//	a templated solution to handle specific/standart
//	kinds of catched exceptions.
//
//	USAGE: (minimalistic)
//
//		XTRY
//		{
//			... // your code
//		}
//		XEND
//
//	USAGE: (extended)
//
//		bool bExceptionThrown = XTRY
//		{
//			... // your code
//		}
//		XCATCH (XMyException1, ex)
//		{
//			... // your code
//		}
//		XCATCH_END
//		XCATCH (XMyException2, ex)
//		{
//			... // your code
//		}
//		XCATCH_END
//		XEND
//


#if X_CUSTOM_HOOKS

// Custom handlers for catched exceptions.
// Here's just declarations, so you have to
// provide your own implementation somewhere

void handleXException(const XException &ex);
void handleStdException(const std::exception &ex);
void handleAllException();

#else

// Default catched exception handlers

inline void handleXException(const XException &ex)
{
	std::cerr << "Unhandled exception has been caught: " << ex.who() << " (" << ex.what();

	if (*ex.info() != '\0')
	{
		std::cerr << ": " << ex.info();
	}

	std::cerr << ")" << std::endl;
}

inline void handleStdException(const std::exception &ex)
{
	std::cerr << "std::exception has been caught: " << typeid (ex).name();

	if (*ex.what() != '\0')
	{
		std::cerr << " (" << ex.what() << ")";
	}

	std::cerr << std::endl;
}

inline void handleAllException()
{
	std::cerr << "Unknown exception (...) has been caught" << std::endl;
}

#endif


template<typename TFuncDo, typename TExc1, typename TExc2, typename TExc3, typename TExc4, typename TExc5>
bool cageException(
	TFuncDo doFunc,
	std::function<void(const TExc1&)> catchFunc1,
	std::function<void(const TExc2&)> catchFunc2,
	std::function<void(const TExc3&)> catchFunc3,
	std::function<void(const TExc4&)> catchFunc4,
	std::function<void(const TExc5&)> catchFunc5)
{
	try
	{
		doFunc();
		return false;
	}
	catch (const TExc1 &ex)
	{
		if (catchFunc1)
		{
			catchFunc1(ex);
		}
	}
	catch (const TExc2 &ex)
	{
		if (catchFunc2)
		{
			catchFunc2(ex);
		}
	}
	catch (const TExc3 &ex)
	{
		if (catchFunc3)
		{
			catchFunc3(ex);
		}
	}
	catch (const TExc4 &ex)
	{
		if (catchFunc4)
		{
			catchFunc4(ex);
		}
	}
	catch (const TExc5 &ex)
	{
		if (catchFunc5)
		{
			catchFunc5(ex);
		}
	}
	catch (const XException &ex)
	{
		handleXException(ex);
	}
	catch (const std::exception &ex)
	{
		handleStdException(ex);
	}
	catch (...)
	{
		handleAllException();
	}
	return true;
}


template<typename TFuncDo, typename TExc1, typename TExc2, typename TExc3, typename TExc4>
bool cageException(
	TFuncDo doFunc,
	std::function<void(const TExc1&)> catchFunc1,
	std::function<void(const TExc2&)> catchFunc2,
	std::function<void(const TExc3&)> catchFunc3,
	std::function<void(const TExc4&)> catchFunc4)
{
	try
	{
		doFunc();
		return false;
	}
	catch (const TExc1 &ex)
	{
		if (catchFunc1)
		{
			catchFunc1(ex);
		}
	}
	catch (const TExc2 &ex)
	{
		if (catchFunc2)
		{
			catchFunc2(ex);
		}
	}
	catch (const TExc3 &ex)
	{
		if (catchFunc3)
		{
			catchFunc3(ex);
		}
	}
	catch (const TExc4 &ex)
	{
		if (catchFunc4)
		{
			catchFunc4(ex);
		}
	}
	catch (const XException &ex)
	{
		handleXException(ex);
	}
	catch (const std::exception &ex)
	{
		handleStdException(ex);
	}
	catch (...)
	{
		handleAllException();
	}
	return true;
}


template<typename TFuncDo, typename TExc1, typename TExc2, typename TExc3>
bool cageException(
	TFuncDo doFunc,
	std::function<void(const TExc1&)> catchFunc1,
	std::function<void(const TExc2&)> catchFunc2,
	std::function<void(const TExc3&)> catchFunc3)
{
	try
	{
		doFunc();
		return false;
	}
	catch (const TExc1 &ex)
	{
		if (catchFunc1)
		{
			catchFunc1(ex);
		}
	}
	catch (const TExc2 &ex)
	{
		if (catchFunc2)
		{
			catchFunc2(ex);
		}
	}
	catch (const TExc3 &ex)
	{
		if (catchFunc3)
		{
			catchFunc3(ex);
		}
	}
	catch (const XException &ex)
	{
		handleXException(ex);
	}
	catch (const std::exception &ex)
	{
		handleStdException(ex);
	}
	catch (...)
	{
		handleAllException();
	}
	return true;
}


template<typename TFuncDo, typename TExc1, typename TExc2>
bool cageException(
	TFuncDo doFunc,
	std::function<void(const TExc1&)> catchFunc1,
	std::function<void(const TExc2&)> catchFunc2)
{
	try
	{
		doFunc();
		return false;
	}
	catch (const TExc1 &ex)
	{
		if (catchFunc1)
		{
			catchFunc1(ex);
		}
	}
	catch (const TExc2 &ex)
	{
		if (catchFunc2)
		{
			catchFunc2(ex);
		}
	}
	catch (const XException &ex)
	{
		handleXException(ex);
	}
	catch (const std::exception &ex)
	{
		handleStdException(ex);
	}
	catch (...)
	{
		handleAllException();
	}
	return true;
}


template<typename TFuncDo, typename TExc1>
bool cageException(
	TFuncDo doFunc,
	std::function<void(const TExc1&)> catchFunc1)
{
	try
	{
		doFunc();
		return false;
	}
	catch (const TExc1 &ex)
	{
		if (catchFunc1)
		{
			catchFunc1(ex);
		}
	}
	catch (const XException &ex)
	{
		handleXException(ex);
	}
	catch (const std::exception &ex)
	{
		handleStdException(ex);
	}
	catch (...)
	{
		handleAllException();
	}
	return true;
}


template<typename TFuncDo>
bool cageException(
	TFuncDo doFunc)
{
	try
	{
		doFunc();
		return false;
	}
	catch (const XException &ex)
	{
		handleXException(ex);
	}
	catch (const std::exception &ex)
	{
		handleStdException(ex);
	}
	catch (...)
	{
		handleAllException();
	}
	return true;
}


#define XTRY cageException ([&]()

#define XEND );

#define XCATCH(x,ex) ,std::function<void(const x &)>([&](const x &ex) {\
	LOG_LINE("catched: " << ex.what ())

#define XCATCH_END })


//
//	Some nice handy defines for daily use
//

#define THROW_IF(c, x) { if (c) THROW(x); }
#define THROW_IF_NOT(c, x) { if (! (c)) THROW(x); }

#define THROW_IF_NULL(x) if((x) == NULL) THROW(XNullPointer);

#define THROW_IF_W(c, x) { if (c) THROW_WINFO(x, #c); }
#define THROW_IF_NOT_W(c, x) { if (! (c)) THROW_WINFO(x, #c); }

#define THROW_IF_NULL_W(c) if((c) == NULL) THROW_WINFO(XNullPointer, #c);

#define THROW_IF_WINFO(c, x, i) { if (c) THROW_WINFO(x, i); }
#define THROW_IF_NOT_WINFO(c, x, i) { if (! (c)) THROW_WINFO(x, i); }

#define THROW_IF_NULL_WINFO(c, i) if((c) == NULL) THROW_WINFO(XNullPointer, i);


// Generic exception 
DECLARE_EXCEPTION(XUnknown, XException, "Unknown generic exception");


//
// Memory exceptions
//

DECLARE_EXCEPTION(XMemory, XException, "Unknown");
DECLARE_EXCEPTION(XNotEnoughMemory, XMemory, "Not enough memory");
DECLARE_EXCEPTION(XNullPointer, XMemory, "NULL pointer dereferencing");


//
// I/O exceptions
//

DECLARE_EXCEPTION(XIO, XException, "Input/output operation failed");

DECLARE_EXCEPTION(XCantOpen, XIO, "Can't open file");
DECLARE_EXCEPTION(XCantRead, XIO, "Can't read from");
DECLARE_EXCEPTION(XCantWrite, XIO, "Can't write to");

DECLARE_EXCEPTION(XEndOfFile, XIO, "Reached end of file");
DECLARE_EXCEPTION(XBadContent, XIO, "Bad content of file");
DECLARE_EXCEPTION(XCantEncode, XIO, "Data encoding failed")
DECLARE_EXCEPTION(XCantDecode, XIO, "Data decoding failed")


//
// Runtime exceptions
//

DECLARE_EXCEPTION(XRuntime, XException, "Run-time error occured");

DECLARE_EXCEPTION(XUnimplementedCode, XRuntime, "Execution flow reaches code that is not implemented.");
DECLARE_EXCEPTION(XIllegalUsage, XRuntime, "Illegal command line has been given.");

DECLARE_EXCEPTION(XAlreadyInited, XRuntime, "Already initialized");
DECLARE_EXCEPTION(XNotInited, XRuntime, "Not initialized");
DECLARE_EXCEPTION(XCantInit, XRuntime, "Can not initialize");

DECLARE_EXCEPTION(XOperationAborted, XRuntime, "Operation was aborted");
DECLARE_EXCEPTION(XOperationCanceled, XRuntime, "Operation was canceled");

DECLARE_EXCEPTION(XBadParameter, XRuntime, "Bad input parameter")
DECLARE_EXCEPTION(XBadValue, XRuntime, "Variable value malformed")
DECLARE_EXCEPTION(XUndefinedValue, XRuntime, "Variable value undefined")
DECLARE_EXCEPTION(XDuplicatedValue, XRuntime, "Value duplication")
DECLARE_EXCEPTION(XOutOfRangeValue, XRuntime, "Value out of range")
DECLARE_EXCEPTION(XOutOfRangeIndex, XRuntime, "Index is out of range.");
DECLARE_EXCEPTION(XBadType, XRuntime, "Variable type not compliant")

DECLARE_EXCEPTION(XBadOperation, XRuntime, "Wrong operation happens")
DECLARE_EXCEPTION(XBadState, XRuntime, "Incorrect object state")

#define UNIMPLEMENTED_CODE THROW(XUnimplementedCode);

#endif	// __XEXCEPTIONS_H
