/**
  \file G3D-base.lib/include/G3D-base/Any.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_Any_h

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Table.h"
#include "G3D-base/Array.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/G3DString.h"

// needed for Token
#include "G3D-base/TextInput.h"
#include "G3D-base/TextOutput.h"
#include <atomic>

#ifdef verify
#undef verify
#endif

namespace G3D {

class TextOutput;

/** 
\brief Easy loading and saving of human-readable configuration files.

Any encodes typed, structured data and can serialize it to a human
readable format that is very similar to the Python language's data
syntax, and fully supports Python's data syntax as well.
It is well-suited for quickly creating human-readable file
formats, especially since deserialization and serialization preserve
comments and an Any can tell you what file and line it came from.  The
syntax allows most C++ editors to properly highlight Any files, and
makes it easy to design little ad-hoc C-like languages in
configuration files.

The class is designed so that copying Anys generally is fast, even if
it is a large array or table.  This is because data is shared between
copies until it is mutated, at which point an actual copy occurs.

\section Example
A sample .Any file:
<pre>
{
   shape = "round",

   // in meters
   radius = 3.7,

   position = Vector3(1.0, -1.0, 0.0),
   video = { format = "RGB8", size = (320, 200)},

   data = [1, 7, 15, 2, -inf, 21, 0],

   material = \#include("rocks.mat")
}
</pre>

Sample code for constructing an Any at runtime:

\code
// Read from an Any format in a string
Any a = Any::parse(mydata);

// Read from a file
Any a;
a.load(filename);

// Construct a new Any
Any a = "Hello";
Any a = 7;

Any t(Any::TABLE);
t["key"] = value;

// Construct a nil value (null is a reserved word in C++)
Any a;

Any x(Any::ARRAY);
x.append(8);
\endcode

Reading from an Any:
\code
// Using AnyTableReader:
bool enabled = true, showSamples = false;
String name = "";

Any a;
a.load("occluder.Any");
AnyTableReader r(a);
// Optional properties
r.getIfPresent("enabled",     enabled);
r.getIfPresent("name",        name);

// Mandatory, error if missing
r.get("showSamples",          showSamples);

// Error if any fields were not read.
r.verifyDone();


// Manual:
Any x;
Vector2 size;
x.load("occluder.Any");

if (x["shape"] == "round") {
    const float r = x["radius"];
    size = Vector2(r, r);
} else {
    size = x["dimnsions"];
}
\endcode

Generating an Any file:
\code
Any a(Any::TABLE);
a["name"] = name;
a["showSamples"] = showSamples;
a["dimensions"] = Vector2(w, h);

Any t(Any::TABLE);
t["h"] = 3;
t["k"] = Matrix3::identity();
a["params"] = t;

a.save("parameters.Any");
\endcode

Interoperation with your own classes:

\code
class MyClass {
protected:
    float m_value = 0.0f;
public:
    ...
    explicit MyClass(const Any& any) {
        *this = MyClass();
        any.verifyName("MyClass");
        AnyTableReader r;
        r.getIfPresent("m_value", m_value);
        r.done();
    }
    
    MyClass& operator=(const Any& a) {
        return *this = MyClass(a);
    }

    Any toAny() const {
        Any any(Any::TABLE, "MyClass");
        any["value"] = m_value;
        return any;
    }
};
\endcode

The custom serialization format was chosen to be terse, easy for
humans to read, and easy for machines to parse. It was specifically
chosen over formats like XML, YAML, JSON, S-expressions, and Protocol
Buffers, although there is no reason you could not write readers and
writers for G3D::Any that support those. Any also currently supports
the JSON format.

\subsection No Cycles Allowed
G3D::Any assumes that structures do not contain cycles; it is an 
error to create a structure like:

<pre>
Any x(Any::ARRAY);
x.array().append(x);    // don't do this!
</pre>

although no exception will be thrown at runtime during that append.

\section #include

When parsing an Any from a file, the syntax 
<code>\#include(<i>filename</i>)</code> allows subsitution of the contents of
<i>filename</i> for any single expression in an Any.  The filename is interpreted
relative to the file being parsed, and inside of the included file, relative filenames
are interpreted with respect to the included file (and so on, recursively for nested
inclusion).

Filenames are resolved with System::resolve and then System::findDataFile if not found,
so they may contain environment variables.

\section Parsing

The primary use of Any is to create your own text file formats.
The Vector3 constructor is a good example of how to use the Any::verify 
methods to provide good error checking while parsing such formats:

<pre>
Vector3::Vector3(const Any& any) {
    any.verifyName("Vector3");
    any.verifyType(Any::TABLE, Any::ARRAY);
    any.verifySize(3);

    if (any.type() == Any::ARRAY) {
        x = any[0];
        y = any[1];
        z = any[2];
    } else {
        // Table
        x = any["x"];
        y = any["y"];
        z = any["z"];
    }
}
</pre>

It is often convenient to iterate through the table portion using G3D::AnyTableReader.

\section BNF
Serialized format BNF:

\htmlonly
<pre>
identifier  ::= (letter | "_") (letter | digit | "_")*
identifier-op ::= "::" | "->" | "."

identifier-exp ::= [identifier-op] identifier (identifier-op identifier)*

comment     ::= C++ single or multi-line comments
separator   ::= "," | ";"

number      ::= <legal C printf number format>
string      ::= <legal C double-quoted or unquoted string; backslashes must be escaped>
boolean     ::= "True" | "False"
nil         ::= "Nil" <not case sensitive> | "None" <case sensitive>
array       ::= ("(" | "[" | "{") [value (separator value)* [separator] ] (")" | "]" | "}")
pair        ::= (identifier | string) ("=" | ":") value
table       ::= ("(" | "[" | "{") [ pair (separator pair)* [separator] ] (")" | "]" | "}")
named-array ::= identifier-exp array
named-table ::= identifier-exp table
include     ::= "#" "include" "(" string ")"

value       ::= [comment] (nil | number | boolean | string | array | table | named-array | named-table | include)
</pre>
\endhtmlonly

Except for single-line comments, whitespace is not significant.  
All parsing is case-insensitive.

The include expression pastes the contents of the named file in as if
they appeared in the original source.  Note that an include expression
can only appear in the locations where a value is expected.  This means
that it cannot yield more than one element of an array and cannot serve
as the pair in a table.

The deserializer allows the substitution of [] or {} for () when writing
tuples and ";" for ",".  These are convenient when mimicing a
programming language, e.g., <code>"[ printf("hello world."); clearScreen();]"</code> 
parses as an array containing two named arrays within it. The 
deserializer also allows a trailing comma inside any array or table,
which also convenient when commenting out the last element.

The serializer indents four spaces for each level of nesting. 
Tables are written with the keys in alphabetic order.

\sa G3D::AnyTableReader
*/
class Any {
public:

    enum Type {NIL, BOOLEAN, NUMBER, STRING, ARRAY, TABLE, EMPTY_CONTAINER};

    static String toString(Type t);

    /** Where an Any came from in a file.  Useful for throwing parsing errors */
    class Source {
    public:
        String                 filename;
        int                         line;
        int                         character;

        Source() : line(0), character(0) {}

        void set(const TextInput& ti, const Token& t) {
            filename  = ti.filename();
            line      = t.line();
            character = t.character();
        }
    };

    typedef Array<Any> AnyArray;
    typedef Table<String, Any> AnyTable;

private:

    /** Called from deserialize() */
    static void deserializeComment(TextInput& ti, Token& token, String& comment);

    /** NIL, BOOLEAN, and NUMBER are stored directly in the Any */
    union SimpleValue {
        bool                        b;
        double                      n;

        inline SimpleValue() : n(0.0) {}
        inline SimpleValue(bool x) : b(x) {}
        inline SimpleValue(double x) : n(x) {}
    };
 
    /** The three options for Data::bracket */
    static const char* PAREN;
    static const char* BRACKET;
    static const char* BRACE;

    class Data {
    public:
        /** ARRAY, TABLE, or STRING value only.  nullptr otherwise. */
        union Value {
            String*                  s;
            Array<Any>*              a;
            AnyTable*                t;
            inline Value() : s(nullptr) {}
        };

        // Needed so that the destructor knows what is in Value
        // and can call its destructor. 
        Type                    type;
        
        /** Always points to memory that is allocated with the Data, so
           the destructor does not delete this. */
        Value                   value;
        
        String                  comment;

        String                  name;

        /** If this Any was created by parsing an #include expression and
            has not been modified since, this is the original comment and
            include statement, as 
            it originally appeared in the file (e.g., it may contain a relative 
            filename).  If this is non-empty, then when serialized, this
            Any will turn into an #include expression instead of unparsing
            its contents of the any.         
          */
        String                  includeLine;

        /** For STRING, ARRAY and TABLE types, m_value is shared between
            multiple instances.  Mutation is allowed only if the reference
            count is exactly 1, otherwise the mutating instance must copy
            the value.  This is not used for other types.
        */
        std::atomic_int         referenceCount;

        Source                  source;

        /** Two-character string of "{}", "[]", or "()"; to be used when unparsing. */
        const char*             bracket;

        /** ';' or ',' separator to be used when unparsing */
        char                    separator;

        /** True if the input was a hex integer number and should 
            be preserved as such when saved. */
        bool                    hexInteger;

    private:

        /** Called by create() */
        inline Data(Type t, const char* b, char s, bool h) : type(t), referenceCount(1), bracket(b), separator(s), hexInteger(h) {}

        /** Called by destroy */
        ~Data();

    public:

        /** Clones the argument */
        static Data* create(const Data* d);
        static Data* create(Type t, const char* b = nullptr, char s = '\0', bool h = false);

        /** Free d, invoking its destructor and freeing the memory for
            the value. */
        static void destroy(Data* d);
    };

    /** If not empty, this Any was created from operator[] on a table
        and perhaps was not intended to exist.  The name is needed to
        format the error message if it is read from before it is
        written to.

        The source of a placeholder object is that of the parent
        object until it is written.
    */
    String         m_placeholderName;

    Type                m_type;
    SimpleValue         m_simpleValue;
    mutable Data*       m_data;

    /** Called before every read operation. */
    void beforeRead() const;

    /** Called before every write operation to this Any.  Wipes the placeholder
        status and includedFrom entry. */
    void beforeWrite();

    /** Decrements the reference count (if there is one).  If the
    reference count is zero after decrement, calls delete on @a m_data
    and sets it to nullptr.
    */
    void dropReference();

    /** Allocate the Data object if it does not exist */
    void ensureData();

    /** If m_data is not nullptr, ensure that it has a unique reference
        and contains a valid m_data.  This has a race condition if two
        threads are both trying to modify the same Any
        simultaneously.*/    
    void ensureMutable();

    /** Read an unnamed a TABLE or ARRAY.  Token should be the open
        paren token; it is the next token after the close on
        return. Called from deserialize().*/
    void deserializeBody(TextInput& ti, Token& token);

    void deserialize(TextInput& ti, Token& token);

    /** Read the name of a named Array or Table. */
    static void deserializeName(TextInput& ti, Token& token, String& name);
    
    /** Read until a separator is consumed or a close paren is hit, and
     return that token.  Considers the passed in token to be the first
     value read. */
    static void readUntilSeparatorOrClose(TextInput& ti, Token& token);

    /** Construct an Any that is a proxy for a table fetch from \a data.
     This proxy can be copied exactly once on return from operator[].*/
    Any(const String& key, Data* data);

    bool isPlaceholder() const {
        return ! m_placeholderName.empty();
    }
    
    void _append(const Any& v0);
    void _append(const Any& v0, const Any& v1);
    void _append(const Any& v0, const Any& v1, const Any& v2);
    void _append(const Any& v0, const Any& v1, const Any& v2, const Any& v3);
    Any  _get(const String& key, const Any& defaultVal) const;
    void _set(const String& key, const Any& val);

    void _parse(const String& src);

public:

    /** Thrown by operator[] when a key is not present in a const table. */
    class KeyNotFound : public ParseError {
    public:
        String key;

        KeyNotFound(const Data* data) {
            if (data) {
                filename  = data->source.filename;
                line      = data->source.line;
                character = data->source.character;
            }
        }
    };

    /** Thrown by operator[] when an array index is not present. */
    class IndexOutOfBounds : public ParseError {
    public:
        int     index;
        int     size;
        IndexOutOfBounds() : index(0), size(0) {}
        IndexOutOfBounds(const Data* data, int i, int s) : index(i), size(s) {
            if (data) {
                filename  = data->source.filename;
                line      = data->source.line;
                character = data->source.character;
            }
            message = format("Index out of bounds: index = %d, array size = %d", i, s);
        }
    };

    /** NIL constructor */
    Any();

    /** Deserialize */
    explicit Any(TextInput& t);

    Any(const Any& x);

    /** NUMBER constructor */
    explicit Any(double x);

    explicit Any(float x);

    // On 64-bit linux int64 can be defined as long which causes a redefinition error.
#   if defined(G3D_32Bit) || defined(_MSC_VER) || defined(G3D_OSX)
    /** NUMBER constructor */
    explicit Any(int64 x) : m_type(NUMBER), m_simpleValue((double)x), m_data(nullptr) {}
#   endif

    /** NUMBER constructor */
    explicit Any(long x);

    /** NUMBER constructor */
    explicit Any(int x);

    /** NUMBER constructor */
    explicit Any(char x);

    /** NUMBER constructor */
    explicit Any(short x);

    /** BOOLEAN constructor */
    explicit Any(bool x);

    /** STRING constructor */
    explicit Any(const String& x);

    /** STRING constructor */
    explicit Any(const char* x);

    /** \a t must be ARRAY or TABLE
        \param brackets must be "" (defaults to {} for table, () for array), "[]", "()", or "{}"
        \param separator must be ';', ',', or '\0' (defaults to ',' for array and ';' for table)
    */  
    explicit Any(Type t, const String& name = "", const String& brackets = "", const char separator = '\0');

    /** Extensible constructor: call the toAny() method of any class. */
    template<class T>
    explicit Any(const T& v) : m_type(NIL), m_data(nullptr) {
        *this = v.toAny();
    }

    ~Any();

    /** Removes the comment and name */
    Any& operator=(const Any& x);

    /** \a t must be ARRAY, TABLE, or NIL. Removes the comment and name */
    Any& operator=(Type t);

    /** Assigns from an array.  Assumes that T can be converted to Any.  Removes the comment and name */
    template<class T>
    Any& operator=(const Array<T>& array) {
        *this = Any::ARRAY;
        resize(array.size());
        for (int i = 0; i < array.size(); ++i) {
            this->operator [](i) = array[i];
        }
        return *this;
    }

    /** Removes the comment and name */
    template<class T>
    Any& operator=(const T& v) {
        *this = Any(v);
        return *this;
    }

    Type type() const;

    /** Same as deserialize or load, but operates on a string instead
        of a stream or file.

      \sa deserialize, load, unparse
      */
    static Any parse(const String& src);

    String unparse(const TextOutput::Settings& s = TextOutput::Settings()) const;

    /** \param allowCoercion If false, throws an error if the Any uses features that are not
       supported by JSON such as named arrays.  Otherwise, silently coerces to JSON. */
    String unparseJSON(const TextOutput::Settings& s = TextOutput::Settings(), bool allowCoercion = true) const;
    
    /** Comments appear before values when they are in serialized form.*/
    const String& comment() const;
    void setComment(const String& c);

    /** True if this is the NIL value */
    bool isNil() const;

    /** Throws a ParseError exception if this is not a number */
    double number() const;
    float floatValue() const;

    const String& string() const;
    bool boolean() const;

    /** If a valid string, takes the string value and creates a fully
        qualified filename.

        The file is searched for the following ways:
        
        - In the directory from which the Any was loaded.
        - By calling System::findDataFile as you would with other data files.

        Strings that begin with '<' and end with '>' are treated as
        escape sequences and are returned unmodifed.
     */
    String resolveStringAsFilename(bool errorIfNotFound = true) const;

    /** If this is named ARRAY or TABLE, returns the name. */
    const String& name() const;

    /** If this is named ARRAY or TABLE, returns true if the name
        begins with \a s.  The comparision is case insensitive. */
    bool nameBeginsWith(const String& s) const;

    /** If this is named ARRAY or TABLE, returns true if the name
        begins with \a s.  The comparision is case insensitive. */
    bool nameBeginsWith(const char* s) const;

    /** If this is named ARRAY or TABLE, returns true if the name is
        \a s.  The comparision is case insensitive. */
    bool nameEquals(const String& s) const;

    /** If this is named ARRAY or TABLE, returns true if the name is\a
        s.  The comparision is case insensitive. */
    bool nameEquals(const char* s) const;

    /** \brief Set the name used when serializing an ARRAY or TABLE.
    
       Only legal for ARRAY or TABLE.  The \a name must begin with a letter
       and contain only letters, numbers, underscores and scope operators.

        <pre>
        a2z
        hello
        Foo::bar
        color.red
        this->that
        __x
        </pre>
        
                
        The scope operators "::", "->", and
        ".", may have spaces around them.  The name may not
        contain parentheses.
    */
    void setName(const String& name);

    /** Number of elements if this is an ARRAY or TABLE */
    int size() const;
    int length() const;

    /** For an array, returns the ith element */
    const Any& operator[](int i) const;
    Any& operator[](int i);

    const Any& last() const {
        return (*this)[size() - 1];
    }

    Any& last() {
        return (*this)[size() - 1];
    }

    /** Directly exposes the underlying data structure for an ARRAY. */
    const Array<Any>& array() const;

    template<class T0>
    void append(const T0& v0) {
        _append(Any(v0));
    }

    template<class T0, class T1>
    void append(const T0& v0, const T1& v1) {
        _append(Any(v0), Any(v1));
    }

    template<class T0, class T1, class T2>
    void append(const T0& v0, const T1& v1, const T2& v2) {
        _append(Any(v0), Any(v1), Any(v2));
    }

    template<class T0, class T1, class T2, class T3>
    void append(const T0& v0, const T1& v1, const T2& v2, const T3& v3) {
        _append(Any(v0), Any(v1), Any(v2), Any(v3));
    }

    /** Directly exposes the underlying data structure for table.
    \sa G3D::AnyTableReader*/
    const Table<String, Any>& table() const;

    /** For a table, returns the element for \a key. Throws KeyNotFound
        exception if the element does not exist.
       */ 
    const Any& operator[](const String& key) const;

    // Needed to prevent the operator[](int) overload from catching
    // string literals
    const Any& operator[](const char* key) const {
        return operator[](String(key));
    }

    /** 
        Fetch an element from a table.  This can be used as:

        <pre>
        a["key"] = value;  (create the key if it did not exist)
        </pre>
        
        or

        <pre>
        value = a["key"];  (throw an error if the key did not exist)
        </pre>

        <b>Note:</b>
        In order to cause elements to be correctly created in the
        first case while still providing "key not found" errors in the
        second case, the Any returned is a special object that delays
        the actual fetch until the following assignment or method
        call.  This means that in the event of an error, the exception
        may be thrown from a line other than the actual fetch.  Use
        the Any::get() or the const Any::operator[]() methods to avoid
        this behavior and ensure error-checking at fetch time.
     */
    Any& operator[](const String& key);

    /** \copydoc Any::operator[](const String&) */
    inline Any& operator[](const char* key) {
        return operator[](String(key));
    }
    
    /** For a table, returns the element for key \a x and \a
        defaultVal if it does not exist. */
    template<class T>
    Any get(const String& key, const T& defaultVal) const {
        return _get(key, Any(defaultVal));
    }

    /** Returns true if this key is in the TABLE.  Illegal to call on an object that is not a TABLE. */
    bool containsKey(const String& key) const;
    
    /** For a table, assigns the element for key k. */
    template<class T>
    void set(const String& key, const T& val) {
        _set(key, Any(val));
    }

    /** for an ARRAY, resizes and returns the last element */
    Any& next();

    /** The parent directory of the location from which this Any was loaded.  This is useful for 
       interpreting filenames relative to the Any's source location,
       which may not match the current directory if the Any was from an included file. */
    String sourceDirectory() const;

    /** True if the Anys are exactly equal, ignoring comments.  Applies deeply on arrays and tables. */
    bool operator==(const Any& x) const;

    bool operator==(const String& s) const {
        return *this == Any(s);
    }

    bool operator==(const double& v) const {
        return *this == Any(v);
    }

    bool operator==(int v) const {
        return *this == Any(v);
    }

    bool operator==(bool v) const {
        return *this == Any(v);
    }

    bool operator!=(const Any& x) const;

    bool operator!=(const String& s) const {
        return *this != Any(s);
    }

    bool operator!=(const double& v) const {
        return *this != Any(v);
    }

    bool operator!=(int v) const {
        return *this != Any(v);
    }

    bool operator!=(bool v) const {
        return *this != Any(v);
    }

    operator int() const;
    operator uint32() const;
    operator float() const;
    operator double() const;
    operator bool() const;
    operator String() const;

    operator char() const {
        return char(int(*this));
    }

    operator uint8() const {
        return uint8(int(*this));
    }

    operator int16() const {
        return int16(int(*this));
    }

    operator uint16() const {
        return uint16(int(*this));
    }

    /** Resize to \a n elements, where new elements are NIL 
       It is an error to call this method if this is not an Any::ARRAY */
    void resize(int n);

    /** 
       Clears all entries.
       This must be a TABLE or ARRAY */
    void clear();

    /** Parse from a file.
     \sa deserialize, parse, fromFile, loadIfExists
     */
    void load(const String& filename);

    /** Load a new Any from \a filename. \sa load, save, loadIfExists */
    static Any fromFile(const String& filename);

    /** Load \a filename file if it exists, otherwise do not modify this */
    void loadIfExists(const String& filename);

    /** Uses the serialize method. If the extension is ".json", uses JSON format with coercion, otherwise uses native Any format. */
    void save(const String& filename, bool json = false) const;

    /** \param coerce.  If json=true, should features that JSON doesn't support be coerced or produce errors?*/
    void serialize(TextOutput& to, bool json = false, bool coerce = false) const;

    void serialize(class BinaryOutput& b) const;

    /** Parse from a stream.
     \sa load, parse */
    void deserialize(TextInput& ti);

    void deserialize(class BinaryInput& b);

    const Source& source() const;

    /** Removes this key from the Any, which must be a table. */
    void remove(const String& key);

    /** Removes this key from the Any, which must be an array, and
        shifts other elements down to maintain order. */
    void remove(int index);
   
    /** Throws a ParseError if \a value is false.  Useful for quickly
        creating parse rules in classes that deserialize from Any.
    */
    void verify(bool value, const String& message = "") const;


    /** Verifies that the name is identifier \a n (case sensitive). 
        It may contain identifier operators after this */
    void verifyName(const String& n) const;

    /** Verifies that the name is identifier \a n or \a m (case sensitive). 
        It may contain identifier operators after this */
    void verifyName(const String& n, const String& m) const;

    /** Verifies that the name is identifier \a n or \a m or \a p (case sensitive). 
        It may contain identifier operators after this */
    void verifyName(const String& n, const String& m, const String& p) const;

    /** Verifies that the name is identifier \a n or \a m or \a p or \a q (case sensitive). 
        It may contain identifier operators after this */
    void verifyName(const String& n, const String& m, const String& p, const String& q) const;

    /** Verifies that the name <i>begins with</i> identifier \a n (case sensitive). 
        It may contain identifier operators after this */
    void verifyNameBeginsWith(const String& n) const;

    /** Verifies that the name <i>begins with</i> identifier \a n or \a m (case sensitive). 
        It may contain identifier operators after this */
    void verifyNameBeginsWith(const String& n, const String& m) const;

    /** Verifies that the name <i>begins with</i> identifier \a n or \a m or \a p (case sensitive). 
        It may contain identifier operators after this */
    void verifyNameBeginsWith(const String& n, const String& m, const String& p) const;

    /** Verifies that the name <i>begins with</i> identifier \a n or \a m or \a p or \a q (case sensitive). 
        It may contain identifier operators after this */
    void verifyNameBeginsWith(const String& n, const String& m, const String& p, const String& q) const;

    /** Verifies that the name <i>begins with</i> identifier \a n or \a m or \a p or \a q or \a r (case sensitive). 
        It may contain identifier operators after this */
    void verifyNameBeginsWith(const String& n, const String& m, const String& p, const String& q, const String& r) const;

    /** Verifies that the name <i>begins with</i> identifier \a n or \a m or \a p or \a q or \a r or \a s (case sensitive). 
        It may contain identifier operators after this */
    void verifyNameBeginsWith(const String& n, const String& m, const String& p, const String& q, const String& r, const String& s) const;

    /** Verifies that the name <i>begins with</i> identifier \a n or \a m or \a p or \a q or \a r or \a s or \a t(case sensitive). 
        It may contain identifier operators after this */
    void verifyNameBeginsWith(const String& n, const String& m, const String& p, const String& q, const String& r, const String& s, const String& t) const;

    /** Verifies that the type is \a t. */
    void verifyType(Type t) const;

    /** Throws an exception if the type is not \a t0 or \a t1. */
    void verifyType(Type t0, Type t1) const;

    /** Verifies that the size is between \a low and \a high, inclusive */
    void verifySize(int low, int high) const;

    /** Verifies that the size is exactly \a s */
    void verifySize(int s) const;

    /** Assumes that Any(T) is well-defined, e.g., by T defining a
        T::toAny() method. */
    template<class T>
    explicit Any(const Array<T>& array, const String& name = "") : m_type(ARRAY), m_data(nullptr) {
        setName(name);
        resize(array.size());
        for (int i = 0; i < array.size(); ++i) {
            (*this)[i] = Any(array[i]);
        }
    }

    /** Assumes that T defines T(const Any&) */
    template<class T>
    void getArray(Array<T>& array) const {
        verifyType(ARRAY);
        array.resize(size());
        for (int i = 0; i < array.size(); ++i) {
            array[i] = T((*this)[i]);
        }
    }

#if ! defined(__clang__) || (__clang_major__ > 6)
    // This overload stresses a known bug on the (obsolete) clang 6.0.0 on
    // some but not all Ubuntu 18.04 LTS installations.

    /** Template specialization to avoid defining
        G3DString(const Any&) and related methods. */
    template<>
    void getArray<String>(Array<String>& array) const {
        verifyType(ARRAY);
        array.resize(size());
        for (int i = 0; i < array.size(); ++i) {
            array[i] = ((*this)[i]).string();
        }
    }
#   endif

    /** Assumes that T defines T(const Any&) */
    template<class T>
    void getTable(Table<String, T>& table) const {
        verifyType(TABLE);
        for (Table<String, Any>::Iterator it = this->table().begin(); it.isValid(); ++it) {
            table.set(it.key(), T(it.value()));
        }
    }

private:

    void deserializeTable(TextInput& ti);
    void deserializeArray(TextInput& ti,const String& term);

    /** Turns an empty container into a table or an array */
    void become(const Type& t);

};    // class Any


/**
   Convenient iteration over the keys of a Any::TABLE, usually
   for implementing construction of an object from an Any.

   Getting an element using either iteration or explicit requests
   consumes that element from the iterator (but not from the Any!)
   It is an error to consume the same element more than once from
   the same iterator.

    \code
    AnyTableReader r(a);
    r.getIfPresent("enabled", enabled);
    r.getIfPresent("showSamples", showSamples);
    r.get("showTiles", showTiles);

    r.verifyDone();
    \endcode
*/
class AnyTableReader {
private:
   Any              m_any;
   Set<String>      m_alreadyRead;

public:
    
    /** Verifies that \a is a TABLE with the given \a name. */
    AnyTableReader(const String& name, const Any& a);

    /** Verifies that \a is a TABLE. */
    AnyTableReader(const Any& a);

    bool hasMore() const {
        return m_any.size() > m_alreadyRead.size();
    }

    /** Verifies that all keys have been read. */
    void verifyDone() const;

    /** Return the underlying Any. */
    const Any& any() const {
        return m_any;
    }

    /** Force the flag used for detecting if a field has been read */
    void setReadStatus(const String& s, bool read) {
        if (read) {
            m_alreadyRead.insert(s);
        } else {
            m_alreadyRead.remove(s);
        }
    }

#if 0
    /** Returns the current key */
    const String& key() const;

    /** Returns the current value */
    const Any& value() const;

    AnyKeyIterator& operator++();
#endif   

    /** \copydoc get(const String& s, ValueType& v, const String& errMsg = "") */
    void get(const String& s, String& v, const String& errMsg = "") {
        try {
            v = m_any[s].string();
        } catch (ParseError& e) {
            if (!errMsg.empty()) {
                e.message += "\n" + e.formatFileInfo() + errMsg;
            }
            throw e;
        }
        m_alreadyRead.insert(s);
    }

    /** \copydoc get(const String& s, ValueType& v, const String& errMsg = "") */
    void get(const String& s, uint8& v, const String& errMsg = "") {
        try {
            v = uint8(m_any[s].number());
        } catch (ParseError& e) {
            if (!errMsg.empty()) {
                e.message += "\n" + e.formatFileInfo() + errMsg;
            }
            throw e;
        }
        m_alreadyRead.insert(s);
    }

    /** \copydoc get(const String& s, ValueType& v, const String& errMsg = "") */
    void get(const String& s, uint16& v, const String& errMsg = "") {
        try {
            v = uint16(m_any[s].number());
        } catch (ParseError& e) {
            if (!errMsg.empty()) {
                e.message += "\n" + e.formatFileInfo() + errMsg;
            }
            throw e;
        }
        m_alreadyRead.insert(s);
    }

    /** Read an entire array at once. \sa get(const String& s, ValueType& v, const String& errMsg = "") */
    template<class T>
    void get(const String& s, Array<T>& v, const String& errMsg = "") {
        try {
            m_any[s].getArray(v);
        } catch (ParseError& e) {
            if (!errMsg.empty()) {
                e.message += "\n" + e.formatFileInfo() + errMsg;
            }
            throw e;
        }
        m_alreadyRead.insert(s);
    }

    /** Read an entire table at once. \sa get(const String& s, ValueType& v, const String& errMsg = "") */
    template<class T>
    void get(const String& s, Table<String, T>& v, const String& errMsg = "") {
        try {
            m_any[s].getTable(v);
        } catch (ParseError& e) {
            if (!errMsg.empty()) {
                e.message += "\n" + e.formatFileInfo() + errMsg;
            }
            throw e;
        }
        m_alreadyRead.insert(s);
    }

    /** If key \a s appears in the any, reads its value into \a v and 
        removes that key from the ones available to iterate over.

        If key \a s does not appear in the any, throws a G3D::ParseError.

        If a G3D::ParseError is thrown, \a errMsg (optional) is appended to the G3D::ParseError::message.

        Assumes that if key \a s appears in the any it has not already been extracted
        by this iterator.  If it has been read before, an assertion will fail in debug mode.

    */
    template<class ValueType>
    void get(const String& s, ValueType& v, const String& errMsg = "") {
        try {
            v = ValueType(m_any[s]);
            m_alreadyRead.insert(s);
        } catch (ParseError& e) {
            if (!errMsg.empty()) {
                e.message += "\n" + e.formatFileInfo() + errMsg;
            }
            throw e;
        } catch(const String& e) {
            ParseError p = ParseError();
            p.message = e;
            if (!errMsg.empty()) {
                p.message += "\n" + p.formatFileInfo() + errMsg;
            }
            throw p;
        }
    }

    /** \sa getIfPresent(const String& s, ValueType& v, ValueType (*factory)(const Any&))
        \sa get(const String& s, ValueType& v, const String& errMsg = "")
    */
    template<class ValueType>
    void get(const String& s, ValueType& v, ValueType (*factory)(const Any&), const String& errMsg = "") {
        try {
            v = ValueType((*factory)(m_any[s]));
            m_alreadyRead.insert(s);
        } catch (ParseError& e) {
            if (!errMsg.empty()) {
                e.message += "\n" + e.formatFileInfo() + errMsg;
            }
            throw e;
        } catch(const String& e) {
            ParseError p = ParseError();
            p.message = e;
            if (!errMsg.empty()) {
                p.message += "\n" + p.formatFileInfo() + errMsg;
            }
            throw p;
        }
    }

    /** Same as get() */
    const Any& operator[](const String& s) {
        m_alreadyRead.insert(s);
        return m_any[s];
    }

    /** Get the value associated with a key only if the key is actually present.
    
        If key \a s appears in the any, reads its value into \a v and 
        removes that key from the ones available to iterate over.

        If key \a s does not appear in the any, does nothing.

        Assumes that if key \a s appears in the any it has not already been extracted
        by this iterator.  If it has been read before, an assertion will fail in debug mode.

        \return True if the value was read.
      */
    template<class ValueType>
    bool getIfPresent(const String& s, ValueType& v) {
        if (m_any.containsKey(s)) {
            debugAssertM(! m_alreadyRead.contains(s), "read twice");

            get(s, v);
            return true;
        } else {
            return false;
        }
    }

    /**
       Helper method for reading values that depend on a factory design pattern.
       
       E.g., replaces the idiom:
       \code
       Any algorithmAny;
       if (r.getIfPresent("algorithm", algorithmAny)) {
          algorithm = Algorithm::create(algorithmAny);
       }
       \endcode

       with 
       \code
    r.getIfPresent("algorithm", algorithm, &Algorithm::create);
    \endcode

    \sa get
    */
    template<class ValueType>
    bool getIfPresent(const String& s, ValueType& v, ValueType (*factory)(const Any&)) {
        if (m_any.containsKey(s)) {
            debugAssertM(! m_alreadyRead.contains(s), "read twice");
            v = (*factory)((*this)[s]);
            return true;
        } else {
            return false;
        }
    }


    /** Uses Any::resolveStringAsFilename */
    bool getFilenameIfPresent(const String& s, String& v) {
        if (m_any.containsKey(s)) {
            debugAssertM(! m_alreadyRead.contains(s), "read twice");
            Any a;
            get(s, a);
            v = a.resolveStringAsFilename();
            return true;
        } else {
            return false;
        }
    }

    /** \return True if \a s is in the table and has not yet been read
        using get() or getIfPresent(). */
    bool containsUnread(const String& s) const {
        return m_any.containsKey(s) && ! m_alreadyRead.contains(s);
    }

};

}    // namespace G3D


/**
  \def PARSE_ANY(expression)
  
  \brief Create an G3D::Any from an unquoted string.

  e.g.,
  \code
     Any x = PARSE_ANY( { a = 3.0; b = false; } );
  \endcode
 */
#define PARSE_ANY(x) Any::parse(#x)
