#ifndef __XEXCEPTIONS_H
#define __XEXCEPTIONS_H

#include <iostream>
#include <ios>
#include <exception>

#if _DEBUG
	#define DEBUG_LINE(s) std::cerr << __FILE__ << ':' << __LINE__ << '\t' << s << std::endl;
	#define DEBUG_STR(s) std::cerr << s;
#else
	#define DEBUG_LINE(s)
	#define DEBUG_STR(s)
#endif

#define TRACE_LINE(s) std::cerr << s << std::endl;
#define TRACE_STR(s) std::cerr << s;

#define TRY try

#define THROW(x) \
{ \
	DEBUG_LINE (x::sExceptionName << " raised, cause: " << x::sExceptionMessage << std::endl); \
	throw x (); \
}

#define THROW_NODEBUG(x) \
{ \
	throw x (); \
}

#define THROW_WINFO(x,str)\
{ \
	DEBUG_LINE (x::sExceptionName << " raised, cause: " << \
		x::sExceptionMessage << ", info: " << str << std::endl); \
	throw x (str);\
}

// Catch exception  without debugging messages
#define CATCH_NODEBUG(x, ex) catch (x &ex) \
{ \

// Casual catch
#define CATCH(x, ex) catch (x &ex) \
{ \
	DEBUG_LINE (ex.who () << " caught" << std::endl); \

// std::exception catch
#define CATCH_STD(ex) catch (std::exception &ex) \
{ \
	DEBUG_LINE (typeid (ex).name () << " caught" << std::endl); \

// End catch definition
#define END_CATCH }

#define DEFAULT_EXCEPTION_STR_SIZE 2048

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
		mInfoString [0] = 0;
	};

	explicit XException (const char * const infoStr) noexcept
	{
		strncpy_s (mInfoString, infoStr, sizeof (mInfoString) - 1);
	};

	XException (XException const &other) noexcept
		: std::exception (other)
	{
		strncpy_s (mInfoString, sizeof (mInfoString), other.mInfoString, sizeof (mInfoString) - 1);
	}

	XException &operator= (XException const &other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		
		std::exception::operator= (other);
		strncpy_s (mInfoString, sizeof (mInfoString), other.mInfoString, sizeof (mInfoString) - 1);
		return *this;
	}
	
	virtual ~XException () noexcept {}

	virtual char const *who () const noexcept { return sExceptionName; }
	virtual char const *what () const noexcept { return sExceptionMessage; }
	virtual const char *const info () const noexcept { return mInfoString; }

	virtual operator const char * const () const noexcept { return what (); }

	static constexpr const char *sExceptionName = "XExceptionExc";
	static constexpr const char *sExceptionMessage = "Generic failure";

protected:

	char mInfoString [DEFAULT_EXCEPTION_STR_SIZE];
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

	TXException () : XBase()
	{
	}

	explicit TXException (const char * const infoStr) noexcept
	{
		strncpy_s (mInfoString, infoStr, sizeof (mInfoString) - 1);
	};

	TXException (TXException const &other) noexcept
		: XBase (other)
	{
	}

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


//
// Runtime exceptions
//

DECLARE_EXCEPTION(XRuntime, XException, "Run-time error occured");

DECLARE_EXCEPTION(XRangeError, XRuntime, "Index is out of range.");
DECLARE_EXCEPTION(XUnimplementedCode, XRuntime, "Execution flow reaches code that is not implemented.");
DECLARE_EXCEPTION(XIllegalUsage, XRuntime, "Illegal command line has been given.");

DECLARE_EXCEPTION(XAlreadyInited, XRuntime, "Already initialized");
DECLARE_EXCEPTION(XNotInited, XRuntime, "Not initialized");
DECLARE_EXCEPTION(XCantInit, XRuntime, "Can not initialize");

DECLARE_EXCEPTION(XOperationCanceled, XRuntime, "Operation was canceled");

DECLARE_EXCEPTION(XBadParameter, XRuntime, "Bad input parameter")
DECLARE_EXCEPTION(XBadValue, XRuntime, "Variable value malformed")
DECLARE_EXCEPTION(XUndefinedValue, XRuntime, "Variable value undefined")
DECLARE_EXCEPTION(XDuplicatedValue, XRuntime, "Value duplication")
DECLARE_EXCEPTION(XOutOfRangeValue, XRuntime, "Value out of range")
DECLARE_EXCEPTION(XBadType, XRuntime, "Variable type not compliant")
DECLARE_EXCEPTION(XCantDecode, XRuntime, "Data decoding failed")

DECLARE_EXCEPTION(XBadOperation, XRuntime, "Wrong operation happens")
DECLARE_EXCEPTION(XBadState, XRuntime, "Incorrect object state")

#define UNIMPLEMENTED_CODE THROW(XUnimplementedCode);

#endif	// __XEXCEPTIONS_H
