#ifndef DRAC_STRING_H
#define DRAC_STRING_H

#ifndef FUNCTION
#define FUNCTION 
#endif

#include "platform.h"

#define STACK_STRBUFFER_NAME  CONCAT(_tempstring_, __LINE__)
#define STACK_STRING(stringName, bufferLength) \
char* STACK_STRBUFFER_NAME = (char*)alloca(bufferLength); \
String stringName = string_from_buffer(bufferLength, STACK_STRBUFFER_NAME)

// needs to be void bc we cannot do pointer math on a utf8 string
// a utf8 character can be 1, 2, 3, or 4 bytes long
typedef void utf8;


struct String
{
    s64 capacity;     // the number of characters available for writing
    s64 length;       // number of characters until the NULL-terminator
    bool32 isDynamic; // if this String is able to grow/reallocate allocate_string
    char* str;        // start of the string buffer for writing
    FORCE_INLINE char& operator[](s32 i) { return str[i]; }
};

struct StringReader
{
    char* string;
    s32 cursor;
    s32 length;
};
#define WHOLE_STRING -1

enum StringReaderDirection
{
    STRING_READER_FORWARD   = 0,
    STRING_READER_BACKWARDS = 1
};

enum NewLineType
{
    LINE_FEED       = 0x1,
    NEWLINE_LF      = LINE_FEED,
    CARRIAGE_RETURN = 0x2,
    NEWLINE_CR      = CARRIAGE_RETURN,
    END_OF_LINE     = LINE_FEED | CARRIAGE_RETURN,
    NEWLINE_CR_LF   = NEWLINE_CR | NEWLINE_LF
};

template<typename T> FORCE_INLINE
String& operator<<(String& s, T item)
{
    append(&s, item);
    return s;
}

FUNCTION String  allocate_string(u64 bufferInitialLength = 0);
FUNCTION void    free_string(String* s);
FUNCTION String  string_from_buffer(u64 bufferSize, void* buffer);
FUNCTION void    string_clear(String* s);
FUNCTION String  clone(const String* s);

FUNCTION void append_nchars(String* s, const char* str, u32 nchars);
FUNCTION void append(String* s, const char* str);
FUNCTION void append(String* s, const String& str);
FUNCTION void append(String* s, char c);
FUNCTION void append(String* s, s32 i);
FUNCTION void append(String* s, u32 u);
FUNCTION void append(String* s, s64 i);
FUNCTION void append(String* s, u64 u);
FUNCTION void append(String* s, f32 f, u32 precision = 8);
FUNCTION void append(String* s, f64 d, u32 precision = 8);
FUNCTION void append(String* s, void* ptr);

FUNCTION bool32 string_is_null_or_empty(const String* s);
FUNCTION bool32 string_is_null_or_whitespace(const String* s);

FUNCTION inline StringReader make_string_reader(char* string, 
                                                s32 readLength = WHOLE_STRING);
FUNCTION inline StringReader make_string_reader(const char* string,
                                                s32 readLength = WHOLE_STRING)
{ return make_string_reader((char*)string, readLength); }
FUNCTION inline char* string_at_reader_cursor(StringReader* reader)
{ return reader->string + reader->cursor; }
FUNCTION inline char   char_at_cursor(StringReader* sr);
FUNCTION inline char   next_char(StringReader* sr);
FUNCTION inline char   prev_char(StringReader* sr);
FUNCTION inline s32    seek_to_beginning(StringReader* sr);
FUNCTION inline s32    seek_to_end(StringReader* sr);
FUNCTION inline bool32 at_beginning(StringReader* sr);
FUNCTION inline bool32 at_end(StringReader* sr);
FUNCTION inline bool32 valid_index(StringReader *sr, s32 index);
FUNCTION s32 seek_next_char(StringReader* sr, char c,
                            StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_string(StringReader* sr, const char* s,
                              StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_string(StringReader* sr, const String* s, 
                              StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_past_whitespace(StringReader* sr,
                                  StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_whitespace(StringReader* sr,
                                  StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_alpha(StringReader* sr,
                             StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_number(StringReader* sr,
                              StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_alphanumeric(StringReader* sr,
                                    StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_non_alphanumeric(StringReader* sr,
                                        StringReaderDirection dir = STRING_READER_FORWARD);
FUNCTION s32 seek_next_line(StringReader* sr,
                            NewLineType newLineType = END_OF_LINE,
                            StringReaderDirection dir = STRING_READER_FORWARD);

// @TODO utf8 variants (utf8_is_upper, etc)
FUNCTION inline bool32 char_is_upper(char c);
FUNCTION inline bool32 char_is_lower(char c);
FUNCTION inline char   char_to_upper(char c);
FUNCTION inline char   char_to_lower(char c);
FUNCTION inline bool32 char_is_whitespace(char c);
FUNCTION inline bool32 char_is_number(char c);
FUNCTION inline bool32 char_is_alpha(char c);
FUNCTION inline bool32 char_is_alphanumeric(char c);
FUNCTION inline bool32 char_equals_ignore_case(char a, char b);

FUNCTION bool32        equals(const char* a, const char* b);
FUNCTION inline bool32 equals(String* a, String* b);
FUNCTION inline bool32 equals(const char* a, String* b);
FUNCTION inline bool32 equals(String* a, const char* b);

FUNCTION bool32        equals_ignore_case(const char* a, const char* b);
FUNCTION inline bool32 equals_ignore_case(String* a, String* b);
FUNCTION inline bool32 equals_ignore_case(const char* a, String* b);
FUNCTION inline bool32 equals_ignore_case(String* a, const char* b);

FUNCTION bool32        match_nbytes(const char* a, const char* b, s32 nbytes);
FUNCTION inline bool32 match(String* a, String* b);
FUNCTION inline bool32 match(const char* a, String* b);
FUNCTION inline bool32 match(String* a, const char* b);
FUNCTION inline bool32 match(const char* a, const char* b);

FUNCTION bool32        match_nbytes_ignore_case(const char* a, const char* b, s32 nbytes);
FUNCTION inline bool32 match_ignore_case(const String* a, const String* b);
FUNCTION inline bool32 match_ignore_case(const char* a, const String* b);
FUNCTION inline bool32 match_ignore_case(const String* a, const char* b);
FUNCTION inline bool32 match_ignore_case(const char* a, const char* b);

FUNCTION s32        find_in_nbytes(const char* s,     s32 seekLen, 
                                   const char* thing, s32 thingLen);
FUNCTION s32        find(const String* s, char c);
FUNCTION s32        find(const char* s, char c);
FUNCTION inline s32 find(const String* s, const char* thing);
FUNCTION inline s32 find(const char* s,   const String* thing);
FUNCTION inline s32 find(const String* s, const String* thing);
FUNCTION inline s32 find(const char* s,   const char* thing);

FUNCTION s32        find_in_nbytes_ignore_case(const char* s,     s32 seekLen, 
                                               const char* thing, s32 thingLen);
FUNCTION inline s32 find_ignore_case(const String* s, const char* thing);
FUNCTION inline s32 find_ignore_case(const char* s,   const String* thing);
FUNCTION inline s32 find_ignore_case(const String* s, const String* thing);
FUNCTION inline s32 find_ignore_case(const char* s,   const char* thing);

FUNCTION inline bool32 contains(const String* s, char c);
FUNCTION inline bool32 contains(const char* s, char c);
FUNCTION inline bool32 contains(const String* s, const char* thing);
FUNCTION inline bool32 contains(const char* s,   String* thing);
FUNCTION inline bool32 contains(const String* s, String* thing);
FUNCTION inline bool32 contains(const char* s,   const char* thing);

FUNCTION inline bool32 contains_ignore_case(const String* s, const char* thing);
FUNCTION inline bool32 contains_ignore_case(const char* s,   String* thing);
FUNCTION inline bool32 contains_ignore_case(const String* s, String* thing);
FUNCTION inline bool32 contains_ignore_case(const char* s,   const char* thing);

FUNCTION void           to_lower(String* s);
FUNCTION void           to_upper(String* s);
FUNCTION void           to_lower(char* s);
FUNCTION void           to_upper(char* s);
FUNCTION inline String  clone_to_lower(const String* s);
FUNCTION inline String  clone_to_upper(const String* s);
FUNCTION void           trim_whitespace(String* s);
FUNCTION inline String  clone_trim_whitespace(const String* s);



FUNCTION u64 parse_u64(const String* s);
FUNCTION u64 parse_u64(const char* s);


#pragma region Inline String Functions

FUNCTION inline StringReader 
make_string_reader(char* string, s32 readLength)
{
    StringReader ret;
    
    ret.string = string;
    ret.cursor = 0;
    ret.length = (readLength == WHOLE_STRING) ? (s32)strlen(string) : readLength;
    
    return ret;
}

FUNCTION inline char   
char_at_cursor(StringReader* sr)
{
    return sr->string[sr->cursor];
}

FUNCTION inline char
next_char(StringReader* sr)
{
    char ret = (sr->cursor == sr->length) ? 0 : char_at_cursor(sr);
    sr->cursor = min(sr->cursor + 1, sr->length);
    return ret;
}

FUNCTION inline char
prev_char(StringReader* sr)
{
    char ret = (sr->cursor == -1) ? 0 : char_at_cursor(sr);
    sr->cursor = max(sr->cursor - 1, -1);
    return ret;
}

FUNCTION inline s32 
seek_to_beginning(StringReader* sr)
{
    sr->cursor = 0;
    return sr->cursor;
}

FUNCTION inline s32
seek_to_end(StringReader* sr)
{
    sr->cursor = sr->length - 1;
    return sr->cursor;
}

FUNCTION inline bool32 
at_beginning(StringReader* sr)
{
    return sr->cursor <= 0;
}

FUNCTION inline bool32 
at_end(StringReader* sr)
{
    return sr->cursor >= sr->length;
}

FUNCTION inline bool32
valid_index(StringReader *sr, s32 index)
{
    return (index < sr->length) && (index > -1);
}

FUNCTION inline bool32
equals(String* a, String* b)
{
    if (a->length != b->length) return BOOL_FALSE;
    return match_nbytes(a->str, b->str, (s32)a->length);
}

FUNCTION inline bool32 
equals(const char* a, String* b)
{
    if (strlen(a) != b->length) return BOOL_FALSE;
    return match_nbytes(a, b->str, (s32)b->length);
}

FUNCTION inline bool32 
equals(String* a, const char* b)
{
    if (a->length != strlen(b)) return BOOL_FALSE;
    return match_nbytes(a->str, b, (s32)a->length);
}

FUNCTION inline bool32
equals_ignore_case(String* a, String* b)
{
    if (a->length != b->length) return BOOL_FALSE;
    return match_nbytes_ignore_case(a->str, b->str, (s32)a->length);
}

FUNCTION inline bool32
equals_ignore_case(const char* a, String* b)
{
    if (strlen(a) != b->length) return BOOL_FALSE;
    return match_nbytes_ignore_case(a, b->str, (s32)b->length);
}

FUNCTION inline bool32
equals_ignore_case(String* a, const char* b)
{
    if (a->length != strlen(b)) return BOOL_FALSE;
    return match_nbytes_ignore_case(a->str, b, (s32)a->length);
}

FUNCTION inline bool32 
match(String* a, String* b)
{
    return match_nbytes(a->str, b->str, (s32)min(a->length, b->length));
}

FUNCTION inline bool32 
match(const char* a, String* b)
{
    auto lenA = (s32)strlen(a);
    return match_nbytes(a, b->str, (s32)min(lenA, b->length));
}

FUNCTION inline bool32 
match(String* a, const char* b)
{
    auto lenB = (s32)strlen(b);
    return match_nbytes(a->str, b, (s32)min(a->length, lenB));
}

FUNCTION inline bool32 
match(const char* a, const char* b)
{
    u32 lenA = (u32)strlen(a);
    u32 lenB = (u32)strlen(b);
    return match_nbytes(a, b, min(lenA, lenB));
}

FUNCTION inline bool32 
match_ignore_case(const String* a, const String* b)
{
    return match_nbytes_ignore_case(a->str, b->str, (s32)min(a->length, b->length));
}

FUNCTION inline bool32 
match_ignore_case(const char* a, const String* b)
{
    u32 lenA = (u32)strlen(a);
    return match_nbytes_ignore_case(a, b->str, (s32)min(lenA, b->length));
}

FUNCTION inline bool32 
match_ignore_case(const String* a, const char* b)
{
    u32 lenB = (u32)strlen(b);
    return match_nbytes_ignore_case(a->str, b, (s32)min(a->length, lenB));
}

FUNCTION inline bool32 
match_ignore_case(const char* a, const char* b)
{
    u32 lenA = (u32)strlen(a);
    u32 lenB = (u32)strlen(b);
    return match_nbytes_ignore_case(a, b, min(lenA, lenB));
}

FUNCTION inline bool32 
char_is_upper(char c)
{
    return (c >= 'A') && (c <= 'Z');
}

FUNCTION inline bool32 
char_is_lower(char c)
{
    return (c >= 'a') && (c <= 'z');
}

FUNCTION inline char 
char_to_upper(char c)
{
    return char_is_lower(c) ? (c + ('A' - 'a')) : (c); 
}

FUNCTION inline char
char_to_lower(char c)
{
    // @NOTE must use -('A'-'a') instead of +('a'-'A') because there is no guarantee
    //       that char is a signed number, if its unsigned there is a risk of overflow
    return char_is_upper(c) ? (c - ('A' - 'a')) : (c); 
}

FUNCTION inline bool32
char_is_whitespace(char c)
{
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

FUNCTION inline bool32
char_is_number(char c)
{
    return (c >= '0') && (c <= '9');
}

FUNCTION inline bool32
char_is_alpha(char c)
{
    return char_is_upper(c) || char_is_lower(c);
}

FUNCTION inline bool32
char_is_alphanumeric(char c)
{
    return char_is_number(c) || char_is_alpha(c) || (c == '_');
}

FUNCTION inline bool32 
char_equals_ignore_case(char a, char b)
{
    return char_to_upper(a) == char_to_upper(b);
}

FUNCTION inline String
clone_to_lower(const String* s)
{
    String ret = clone(s);
    to_lower(&ret);
    return ret;
}

FUNCTION inline String
clone_to_upper(const String* s)
{
    String ret = clone(s);
    to_upper(&ret);
    return ret;
}

FUNCTION inline String
clone_trim_whitespace(const String* s)
{
    String ret = clone(s);
    trim_whitespace(&ret);
    return ret;
}

FUNCTION inline s32
find(const String* s, const char* thing)
{
    s32 lenThing = (s32)strlen(thing);
    return find_in_nbytes(s->str, s->length, thing, lenThing);
}

FUNCTION inline s32
find(const char* s, const String* thing)
{
    s32 lenS = (s32)strlen(s);
    return find_in_nbytes(s, lenS, thing->str, thing->length);
}

FUNCTION inline s32
find(const String* s, const String* thing)
{
    return find_in_nbytes(s->str, s->length, thing->str, thing->length);
}

FUNCTION inline s32
find(const char* s, const char* thing)
{
    s32 lenS = (s32)strlen(s);
    s32 lenThing = (s32)strlen(thing);
    return find_in_nbytes(s, lenS, thing, lenThing);
}

FUNCTION inline s32
find_ignore_case(const String* s, const char* thing)
{
    s32 lenThing = (s32)strlen(thing);
    return find_in_nbytes_ignore_case(s->str, s->length, thing, lenThing);
}

FUNCTION inline s32
find_ignore_case(const char* s, const String* thing)
{
    s32 lenS = (s32)strlen(s);
    return find_in_nbytes_ignore_case(s, lenS, thing->str, thing->length);
}

FUNCTION inline s32
find_ignore_case(const String* s, const String* thing)
{
    return find_in_nbytes_ignore_case(s->str, s->length, thing->str, thing->length);
}

FUNCTION inline s32
find_ignore_case(const char* s, const char* thing)
{
    s32 lenS = (s32)strlen(s);
    s32 lenThing = (s32)strlen(thing);
    return find_in_nbytes_ignore_case(s, lenS, thing, lenThing);
}

FUNCTION inline bool32
contains(const String* s, char c)
{
    return -1 != find(s, c);
}

FUNCTION inline bool32
contains(const char* s, char c)
{
    return -1 != find(s, c);
}

FUNCTION inline bool32 
contains(const String* s, const char* thing)
{
    return -1 != find(s, thing);
}

FUNCTION inline bool32
contains(const char* s, String* thing)
{
    return -1 != find(s, thing);
}

FUNCTION inline bool32
contains(const String* s, String* thing)
{
    return -1 != find(s, thing);
}

FUNCTION inline bool32
contains(const char* s, const char* thing)
{
    return -1 != find(s, thing);
}

FUNCTION inline bool32
contains_ignore_case(const String* s, const char* thing)
{
    return -1 != find_ignore_case(s, thing);
}

FUNCTION inline bool32
contains_ignore_case(const char* s, String* thing)
{
    return -1 != find_ignore_case(s, thing);
}

FUNCTION inline bool32
contains_ignore_case(const String* s, String* thing)
{
    return -1 != find_ignore_case(s, thing);
}

FUNCTION inline bool32 
contains_ignore_case(const char* s, const char* thing)
{
    return -1 != find_ignore_case(s, thing);
}

#pragma endregion

//
// static implementations of my string library
//

#ifndef DRAC_STRING_IMPLEMENTATIONS

#pragma region String Implementations

#define MIN_ALLOCATED 32

FUNCTION bool32
reserve(String* s, s64 stringLength)
{
    if (s->capacity >= stringLength) 
    {
        return BOOL_TRUE;
    }
    
    if (!s->isDynamic) 
    {
        return BOOL_FALSE;
    }
    
    // find next power of 2 bigger than requested length
    s64 newLength = 1;
    s64 lg = s->capacity;
    while(lg >>= 1) newLength *= 2;
    while(newLength < stringLength) newLength *= 2;
    
    char* newBuf = (char*)PLATFORM_REALLOC(s->str, newLength);
    if (!newBuf)
    {
        return BOOL_FALSE;
    }
    
    s->capacity = newLength;
    s->str = newBuf;
    return BOOL_TRUE;
}

inline FUNCTION bool32 
reserve_n_more_bytes(String* s, s64 n)
{
    return reserve(s, n + s->length);
}

FUNCTION String
allocate_string(u64 bufferInitialLength)
{
    bufferInitialLength = max(MIN_ALLOCATED, bufferInitialLength);
    char* buf = (char*)PLATFORM_MALLOC(bufferInitialLength);
    assert_msg(buf, "Tried to allocate a String but failed");
    
    String s;
    
    s.length = 0;
    s.capacity = bufferInitialLength;
    s.isDynamic = BOOL_TRUE;
    s.str = buf;
    s.str[0] = NULL;
    
    return s;
}

FUNCTION void 
free_string(String* s)
{
    assert(s->isDynamic);
    s->capacity = 0;
    s->length = 0;
    s->isDynamic = BOOL_FALSE;
    PLATFORM_FREE(s->str);
}

FUNCTION String
string_from_buffer(u64 bufferLength, void* buffer)
{
    char* alignedBuffer = (char*)align_8((uptr)buffer);
    assert((uptr)alignedBuffer > 0);
    
    s64 diff = abs((char*)buffer - alignedBuffer);
    
    String s;
    
    s.length = 0;
    s.capacity = bufferLength - diff;
    s.isDynamic = BOOL_FALSE;
    s.str = alignedBuffer;
    s.str[0] = NULL;
    
    return s;
}

FUNCTION void
string_clear(String* s)
{
    memzero(s->str, s->length);
    s->length = 0;
}

FUNCTION String
clone(const String* s)
{
    String ret = allocate_string(s->capacity);
    
    memcpy(ret.str, s->str, s->length);
    ret.length = s->length;
    ret.isDynamic = BOOL_TRUE;
    ret.str[ret.length] = 0;
    
    return ret;
}

FUNCTION void 
append_nchars(String* s, const char* str, u32 nchars)
{
    bool32 result = reserve_n_more_bytes(s, nchars);
    assert_msg(result, "tried to overwrite string buffer");
    
    for(u32 i = 0;
        str[i] && (i < nchars) && (i < (s->capacity-1));
        ++i)
    {
        s->str[s->length] = str[i];
        s->length++;
    }
    s->str[s->length] = NULL; 
}

FUNCTION void 
append(String* s, const char* str)
{
    append_nchars(s, str, (u32)strlen(str));
}

FUNCTION void
append(String* s, const String& str)
{
    append_nchars(s, str.str, str.length);
}

FUNCTION void
append(String* s, char c)
{
    append_nchars(s, &c, 1);
}

FUNCTION void 
append(String* s, s32 i)
{
    append(s, (s64)i);
}

FUNCTION void
append(String* s, u32 u)
{
    append(s, (u64)u);
}

FUNCTION void 
append(String* s, s64 i)
{
    char buf[32];
    u32 temp_cursor = 0;
    u32 start_cursor = 0;
    s8 positive_adjuster = 1;
    const s8 ten = 10;
    
    if (i < 0)
    {
        buf[temp_cursor++] = '-';
        start_cursor++; 
        positive_adjuster = -1;
    }
    
    do 
    {
        buf[temp_cursor++] = '0' + (positive_adjuster*(i % ten));
        i /= ten;
    } while (i);
    
    char* start_s32_str = buf + start_cursor;
    char* end_s32_str = buf + temp_cursor - 1;
    
    while (end_s32_str > start_s32_str) 
    {
        swap_bitwise(*start_s32_str, *end_s32_str);
        --end_s32_str;
        ++start_s32_str;
    }
    
    append_nchars(s, buf, temp_cursor);
}

FUNCTION void 
append(String* s, u64 u)
{
    char buf[32];
    u32 temp_cursor = 0;
    u32 start_cursor = 0;
    
    const s32 ten = 10;
    do
    {
        buf[temp_cursor++] = (u % ten) + '0';
        u /= ten;
    } while (u);
    
    char* start_s32_str = buf + start_cursor;
    char* end_s32_str = buf + temp_cursor - 1;
    
    while (end_s32_str > start_s32_str)
    {
        swap_bitwise(*start_s32_str, *end_s32_str);
        --end_s32_str;
        ++start_s32_str;
    }
    
    append_nchars(s, buf, temp_cursor);
}

FUNCTION void 
append(String* s, f32 f, u32 precision)
{
    char buf[32];
    
    s64 int_portion = (s64)f;
    f32 float_portion = f - int_portion;
    
    u32 temp_cursor = 0;
    u32 start_cursor = 0;
    
    if (f < 0)
    {
        buf[temp_cursor++] = '-';
        start_cursor++;
        int_portion = -int_portion;
        float_portion = -float_portion;
    }
    
    const s32 ten = 10;
    
    // integer portion
    do
    {
        buf[temp_cursor++] = (int_portion % ten) + '0';
        int_portion /= ten;
    } while (int_portion);
    
    char* start_s32_str = buf + start_cursor;
    char* end_s32_str = buf + temp_cursor - 1;
    
    while (end_s32_str > start_s32_str) 
    {
        swap_bitwise(*start_s32_str, *end_s32_str);
        --end_s32_str;
        ++start_s32_str;
    }
    
    buf[temp_cursor++] = '.';
    
    s64 dec_multiplier = 1;
    u32 temp_precision = precision;
    while (temp_precision--) {
        dec_multiplier *= ten;
    }
    
    u64 float_print_portion = (u64)(float_portion * dec_multiplier);
    while (float_portion > f32_EPSILON && float_portion < 0.1f) {
        buf[temp_cursor++] = '0';
        float_portion *= ten;
    }
    
    start_cursor = temp_cursor;
    
    do
    {
        buf[temp_cursor++] = (float_print_portion % ten) + '0';
        float_print_portion /= ten;
    } while (float_print_portion);
    
    start_s32_str = buf + start_cursor;
    end_s32_str = buf + temp_cursor - 1;
    while (end_s32_str > start_s32_str)
    {
        swap_bitwise(*start_s32_str, *end_s32_str);
        --end_s32_str;
        ++start_s32_str;
    }
    
    buf[temp_cursor] = NULL;
    
    append_nchars(s, buf, temp_cursor);
}

FUNCTION void 
append(String* s, f64 d, u32 precision)
{
    char buf[32];
    
    s64 int_portion = (s64)d;
    f64 double_portion = d - int_portion;
    
    u32 temp_cursor = 0;
    u32 start_cursor = 0;
    
    if (d < 0) {
        buf[temp_cursor++] = '-';
        start_cursor++;
        int_portion = -int_portion;
        double_portion = -double_portion;
    }
    
    const s32 ten = 10;
    
    // integer portion
    do 
    {
        buf[temp_cursor++] = (int_portion % ten) + '0';
        int_portion /= ten;
    } while (int_portion);
    
    char* start_s32_str = buf + start_cursor;
    char* end_s32_str = buf + temp_cursor - 1;
    
    while (end_s32_str > start_s32_str) {
        swap_bitwise(*start_s32_str, *end_s32_str);
        --end_s32_str;
        ++start_s32_str;
    }
    
    buf[temp_cursor++] = '.';
    
    s64 dec_multiplier = 1;
    u32 temp_precision = precision;
    while (temp_precision--) {
        dec_multiplier *= ten;
    }
    
    u64 float_print_portion = (u64)(double_portion * dec_multiplier);
    while (double_portion > f32_EPSILON && double_portion < 0.1)
    {
        buf[temp_cursor++] = '0';
        double_portion *= ten;
    }
    
    start_cursor = temp_cursor;
    
    do 
    {
        buf[temp_cursor++] = (float_print_portion % ten) + '0';
        float_print_portion /= ten;
    } while (float_print_portion);
    
    start_s32_str = buf + start_cursor;
    end_s32_str = buf + temp_cursor - 1;
    while (end_s32_str > start_s32_str)
    {
        swap_bitwise(*start_s32_str, *end_s32_str);
        --end_s32_str;
        ++start_s32_str;
    }
    
    buf[temp_cursor] = NULL;
    append_nchars(s, buf, temp_cursor);
}

FUNCTION void 
append(String* s, void* ptr)
{
    LOCAL_STATIC const u32 hex = 16;
    LOCAL_STATIC const char* hexdigit = "0123456789ABCDEF";
    
    char buf[32];
    
    buf[0] = '0';
    buf[1] = 'x';
    u32 temp_cursor = 2;
    
    u32 num_digits;
    u64 hex_ptr = (u64)ptr;
    if (sizeof(void*) == 4) 
    {
        num_digits = 8;
    } 
    else 
    {
        num_digits = 16;
    }
    
    do 
    {
        buf[temp_cursor++] = hexdigit[hex_ptr % hex];
        hex_ptr /= hex;
    } while (hex_ptr || --num_digits);
    
    char* start_s32_str = buf + 2;
    char* end_s32_str = buf + temp_cursor - 1;
    
    while (end_s32_str > start_s32_str)
    {
        swap_bitwise(*start_s32_str, *end_s32_str);
        --end_s32_str;
        ++start_s32_str;
    }
    
    buf[temp_cursor] = NULL;
    append_nchars(s, buf, temp_cursor);    
}

#undef MIN_ALLOCATED

FUNCTION bool32
string_is_null_or_empty(const String* s)
{
    return !(s && s->str && s->str[0]);
}

FUNCTION bool32
string_is_null_or_whitespace(const String* s)
{
    if (string_is_null_or_empty(s)) return BOOL_TRUE;
    for(s32 i = 0; i < s->length; ++i)
    {
        if (char_is_whitespace(s->str[0])) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION s32
seek_next_char(StringReader* sr, char c, StringReaderDirection dir)
{
    register s32 nextCursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            if (sr->cursor >= sr->length)
            {
                sr->cursor = sr->length;
                return sr->length;
            }
            
            nextCursor = max(0, sr->cursor);
            while (nextCursor < sr->length && 
                   sr->string[nextCursor] != c)
            {
                ++nextCursor;
            }
            sr->cursor = min(nextCursor + 1, sr->length); 
        } break;
        
        case STRING_READER_BACKWARDS: 
        {
            if (sr->cursor <= -1)
            {
                sr->cursor = -1;
                return -1;
            }
            
            nextCursor = min(sr->length - 1, sr->cursor);
            while (nextCursor > -1 && 
                   sr->string[nextCursor] != c)
            {
                --nextCursor;
            }
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32
seek_next_string(StringReader* sr, const char* s, StringReaderDirection dir)
{
    register s32 seekLength = (s32)strlen(s);
    register s32 nextCursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            if (sr->cursor >= sr->length)
            {
                sr->cursor = sr->length;
                return sr->length;
            }
            
            nextCursor = max(0, sr->cursor);
            while (nextCursor < sr->length - (seekLength - 1) && 
                   !match_nbytes(sr->string + nextCursor, s, seekLength))
            {
                ++nextCursor;
            }
            // if not found
            if (!(nextCursor < sr->length - (seekLength - 1))) 
                nextCursor = sr->length;
            sr->cursor = min(nextCursor + seekLength, sr->length);
        } break;
        
        case STRING_READER_BACKWARDS:
        {
            if (sr->cursor < 0)
            {
                sr->cursor = -1;
                return -1;
            }
            
            nextCursor = min(sr->cursor, sr->length - seekLength);
            while (nextCursor > -1 &&
                   !match_nbytes(sr->string + nextCursor, s, seekLength))
            {
                --nextCursor;
            }
            // if not found
            if (!(nextCursor > -1))
                nextCursor = -1;
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32
seek_next_string(StringReader* sr, const String* s, StringReaderDirection dir)
{
    register s32 seekLength = s->length;
    register s32 nextCursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            if (sr->cursor > sr->length - seekLength)
            {
                sr->cursor = sr->length;
                return sr->length;
            }
            
            nextCursor = sr->cursor;
            while (nextCursor < sr->length - (seekLength - 1) && 
                   !match_nbytes(sr->string + nextCursor, s->str, seekLength))
            {
                ++nextCursor;
            }
            // if not found
            if (!(nextCursor < sr->length - (seekLength - 1))) 
                nextCursor = sr->length;
            sr->cursor = min(nextCursor + seekLength, sr->length);
        } break;
        
        case STRING_READER_BACKWARDS:
        {
            if (sr->cursor < 0)
            {
                sr->cursor = -1;
                return -1;
            }
            
            nextCursor = min(sr->cursor, sr->length - seekLength);
            while (nextCursor > -1 &&
                   !match_nbytes(sr->string + nextCursor, s->str, seekLength))
            {
                --nextCursor;
            }
            // if not found
            if (!(nextCursor > -1))
                nextCursor = -1;
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32
seek_past_whitespace(StringReader* sr, StringReaderDirection dir)
{
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            while (sr->cursor < sr->length &&
                   char_is_whitespace(sr->string[sr->cursor]))
            {
                sr->cursor++;
            }
        } break;
        
        case STRING_READER_BACKWARDS:
        {
            while (sr->cursor > -1 &&
                   char_is_whitespace(sr->string[sr->cursor]))
            {
                sr->cursor--;
            }
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return sr->cursor;
}

FUNCTION s32
seek_next_whitespace(StringReader* sr, StringReaderDirection dir)
{
    register s32 nextCursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            if (sr->cursor >= sr->length)
            {
                sr->cursor = sr->length;
                return sr->length;
            }
            
            nextCursor = max(0, sr->cursor);
            while (nextCursor < sr->length && 
                   !char_is_whitespace(sr->string[nextCursor]))
            {
                ++nextCursor;
            }
            sr->cursor = min(nextCursor + 1, sr->length); 
        } break;
        
        case STRING_READER_BACKWARDS: 
        {
            if (sr->cursor <= -1)
            {
                sr->cursor = -1;
                return -1;
            }
            
            nextCursor = min(sr->length - 1, sr->cursor);
            while (nextCursor > -1 && 
                   !char_is_whitespace(sr->string[nextCursor]))
            {
                --nextCursor;
            }
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32
seek_next_alpha(StringReader* sr, StringReaderDirection dir)
{
    register s32 nextCursor = sr->cursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            while (nextCursor < sr->length &&
                   !char_is_alpha(sr->string[nextCursor]))
            {
                nextCursor++;
            }
            sr->cursor = min(nextCursor + 1, sr->length);
        } break;
        
        case STRING_READER_BACKWARDS:
        {
            while (nextCursor > -1 &&
                   !char_is_alpha(sr->string[nextCursor]))
            {
                nextCursor--;
            }
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32
seek_next_number(StringReader* sr, StringReaderDirection dir)
{
    register s32 nextCursor = sr->cursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            while (nextCursor < sr->length &&
                   !char_is_number(sr->string[nextCursor]))
            {
                sr->cursor++;
            }
            sr->cursor = min(nextCursor + 1, sr->length);
        } break;
        
        case STRING_READER_BACKWARDS:
        {
            while (nextCursor > -1 &&
                   !char_is_number(sr->string[nextCursor]))
            {
                nextCursor--;
            }
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32 
seek_next_alphanumeric(StringReader* sr, StringReaderDirection dir)
{
    register s32 nextCursor = sr->cursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            while (nextCursor < sr->length &&
                   !char_is_alphanumeric(sr->string[nextCursor]))
            {
                nextCursor++;
            }
            sr->cursor = min(nextCursor + 1, sr->length);
        } break;
        
        case STRING_READER_BACKWARDS:
        {
            while (nextCursor > -1 &&
                   !char_is_alphanumeric(sr->string[nextCursor]))
            {
                nextCursor--;
            }
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32
seek_next_non_alphanumeric(StringReader* sr, StringReaderDirection dir)
{
    register s32 nextCursor = sr->cursor;
    switch (dir)
    {
        case STRING_READER_FORWARD:
        {
            while (nextCursor < sr->length &&
                   char_is_alphanumeric(sr->string[nextCursor]))
            {
                nextCursor++;
            }
            sr->cursor = min(nextCursor + 1, sr->length);
        } break;
        
        case STRING_READER_BACKWARDS:
        {
            while (nextCursor > -1 &&
                   char_is_alphanumeric(sr->string[nextCursor]))
            {
                nextCursor--;
            }
            sr->cursor = max(nextCursor - 1, -1);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking through a string with invalid direction designator");
            return -1;
        } break;
    }
    return nextCursor;
}

FUNCTION s32
seek_next_line(StringReader* sr, NewLineType newLineType, StringReaderDirection dir)
{
    switch (newLineType)
    {
        case CARRIAGE_RETURN:
        {
            seek_next_char(sr, '\r', dir);
        } break;
        
        case LINE_FEED:
        {
            seek_next_char(sr, '\n', dir);
        } break;
        
        case END_OF_LINE:
        {
            seek_next_string(sr, "\r\n", dir);
        } break;
        
        default:
        {
            assert_msg(0, "Tried seeking a string for a newline character, "
                       "but asked for an invalid newline type");
            return -1;
        } break;
    }
    return sr->cursor;
}

FUNCTION bool32 
equals(const char* a, const char* b)
{
    size_t lenA = strlen(a);
    size_t lenB = strlen(b);
    if (lenA != lenB) return BOOL_FALSE;
    return match_nbytes(a, b, (s32)lenA);
}

FUNCTION bool32
equals_ignore_case(const char* a, const char* b)
{
    size_t lenA = strlen(a);
    size_t lenB = strlen(b);
    if (lenA != lenB) return BOOL_FALSE;
    return match_nbytes_ignore_case(a, b, (s32)lenA);
}

FUNCTION bool32 
match_nbytes(const char* a, const char* b, s32 nbytes)
{
    s32 bytesPerCompare = 1;
#ifdef DRAC_SSE2
    if (nbytes >= 16 && ((uptr)a % 16) == ((uptr)b % 16))
    {
        bytesPerCompare = 16;
    }
    else
#endif
        if (nbytes >= 8 && ((uptr)a % 8) == ((uptr)b % 8))
    {
        bytesPerCompare = 8;
    }
    else if (nbytes >= 4 && ((uptr)a % 4) == ((uptr)b % 4))
    {
        bytesPerCompare = 4;
    }
    
    size_t extraFront = (1 == bytesPerCompare) ? 
        nbytes : ((uptr)a) % bytesPerCompare;
    size_t sseCount = (nbytes - extraFront) / bytesPerCompare;
    size_t extraBack = nbytes - (sseCount * bytesPerCompare) - extraFront;
    
    { 
        while (extraFront--) 
        {
            if (*a != *b) return BOOL_FALSE;
            a++; b++;
        }
    }
    
#ifdef DRAC_SSE2
    if (16 == bytesPerCompare)
    {
        for (register size_t sseIt = 0; sseIt < sseCount; ++sseIt) 
        {
            __m128i xmmA = _mm_load_si128((__m128i*)a);
            __m128i xmmB = _mm_load_si128((__m128i*)b);
            
            int equalityMask = _mm_movemask_epi8(_mm_cmpeq_epi8(xmmA, xmmB));
            
            if (equalityMask != 0xFFFF) return BOOL_FALSE;
            
            a += 16; b += 16;
        }
    }
    else 
#endif
        if (8 == bytesPerCompare)
    {
        for (register size_t sseIt = 0; sseIt < sseCount; ++sseIt) 
        {
            register u64 a64 = *(u64*)a;
            register u64 b64 = *(u64*)b;
            
            if (a64 != b64) return BOOL_FALSE;
            
            a += 8; b += 8;
        }
    }
    else if (4 == bytesPerCompare)
    {
        for (register size_t sseIt = 0; sseIt < sseCount; ++sseIt) 
        {
            register u32 a32 = *(u32*)a;
            register u32 b32 = *(u32*)b;
            
            if (a32 != b32) return BOOL_FALSE;
            
            a += 4; b += 4;
        }
    }
    
    { 
        while (extraBack--) 
        {
            if (*a != *b) return BOOL_FALSE;
            a++; b++;
        }
    }
    
    return BOOL_TRUE;
}

FUNCTION bool32 
match_nbytes_ignore_case(const char* a, const char* b, s32 nbytes)
{
    for(s32 i = 0; i < nbytes; ++i)
    {
        if (!char_equals_ignore_case(a[i], b[i]))
            return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION void           
to_lower(String* string)
{
    for(u32 i = 0; i < string->length; ++i)
    {
        string->str[i] = char_to_lower(string->str[i]);
    }
}

FUNCTION void
to_upper(String* string)
{
    for(u32 i = 0; i < string->length; ++i)
    {
        string->str[i] = char_to_upper(string->str[i]);
    }
}

FUNCTION void           
to_lower(char* s)
{
    while(*s != 0)
    {
        *s = char_to_lower(*s);
        s++;
    }
}

FUNCTION void
to_upper(char* s)
{
    while(*s != 0)
    {
        *s = char_to_upper(*s);
        s++;
    }
}

FUNCTION void
trim_whitespace(String* string)
{
    u32 leadingNonWhitespaceIndex;
    u32 trailingNonWhitespaceIndex;
    
    if (string->length == 0) return;
    
    { 
        leadingNonWhitespaceIndex = 0;
        while(leadingNonWhitespaceIndex < string->length &&
              char_is_whitespace(string->str[leadingNonWhitespaceIndex]))
        {
            ++leadingNonWhitespaceIndex;
        }
        if(leadingNonWhitespaceIndex >= string->length)
        {
            string->length = 0;
            memzero(string->str, string->capacity);
            return;
        }
        trailingNonWhitespaceIndex = string->length - 1;
        while(trailingNonWhitespaceIndex > 0 &&
              char_is_whitespace(string->str[trailingNonWhitespaceIndex]))
        {
            --trailingNonWhitespaceIndex;
        }
    }
    
    s32 newLength = trailingNonWhitespaceIndex - leadingNonWhitespaceIndex + 1;
    memmove(&string->str[0], 
            &string->str[leadingNonWhitespaceIndex], 
            newLength);
    string->length = newLength;
    memzero(&string->str[newLength], string->capacity - newLength);
}

FUNCTION s32
find(const String* s, char c)
{
    for_str(s->str)
    {
        if (*it == c) return (s32)((u64)it - (u64)s);
    }
    return -1;
}

FUNCTION s32
find(const char* s, char c)
{
    for_str(s)
    {
        if (*it == c) return (s32)((u64)it - (u64)s);
    }
    return -1;
}

FUNCTION s32
find_in_nbytes(const char* s, s32 seekLen, const char* thing, s32 thingLen)
{
    for(s32 i = 0; i < seekLen - thingLen + 1; ++i)
    {
        if (match_nbytes(s + i, thing, thingLen)) return i;
    }
    return -1;
}

FUNCTION s32        
find_in_nbytes_ignore_case(const char* s, s32 seekLen, const char* thing, s32 thingLen)
{
    for(s32 i = 0; i < seekLen - thingLen + 1; ++i)
    {
        if (match_nbytes_ignore_case(s + i, thing, thingLen)) return i;
    }
    return -1;
}


FUNCTION u64 
parse_u64(const String* s)
{
    u64 ret = 0;
    u32 i = 0;
    while(i < s->length && char_is_number(s->str[i]))
    {
        ret *= 10;
        ret += s->str[i] - '0';
        ++i;
    }
    return ret;
}

FUNCTION u64
parse_u64(const char* s)
{
    u64 ret = 0;
    s32 i = 0;
    while(s[i] && char_is_number(s[i]))
    {
        ret *= 10;
        ret += s[i] - '0';
        ++i;
    }
    return ret;
}


#endif

#endif /* DRAC_STRING_H */