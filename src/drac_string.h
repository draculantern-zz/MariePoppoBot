#ifndef DRAC_STRING_H
#define DRAC_STRING_H

#ifndef FUNCTION
#define FUNCTION 
#endif

#define DRAC_STRING_IMPLEMENTATIONS 1

#include "platform.h"

#define STACK_STRBUFFER_NAME  CONCAT(_tempstring_, __LINE__)
#define STACK_STRING(stringName, bufferLength) \
char STACK_STRBUFFER_NAME [bufferLength + 1]; \
String stringName = string_from_buffer(STACK_STRBUFFER_NAME,bufferLength)

// needs to be void bc we cannot do pointer math on a utf8 string
// a utf8 character can be 1, 2, 3, or 4 bytes long
typedef void utf8;

struct String
{
    s64 capacity;     // the number of characters available for writing
    s64 length;       // number of characters until the NULL-terminator
    char* str;        // start of the string buffer for writing
    inline char& operator[](s64 i) { return str[i]; }
};

struct StringReader
{
    char* string;
    s32 cursor;
    s32 length;
};
#define WHOLE_STRING -1

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

FUNCTION String  string_from_buffer(void* buffer, u64 bufferSize);
FUNCTION void    string_clear(String* s);

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
FUNCTION bool32 string_is_null_or_empty(const char* s);
FUNCTION bool32 string_is_null_or_whitespace(const char* s);

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

FUNCTION bool32 equals(const String* a, const String* b);
FUNCTION bool32 equals(const String* a, const char* b);
FUNCTION inline bool32 equals(const char* a, const String* b);
FUNCTION bool32 equals(const char* a, const char* b);

FUNCTION bool32 equals_ignore_case(const String* a, const String* b);
FUNCTION bool32 equals_ignore_case(const String* a, const char* b);
FUNCTION inline bool32 equals_ignore_case(const char* a, const String* b);
FUNCTION bool32 equals_ignore_case(const char* a, const char* b);

FUNCTION bool32 match(const String* a, const String* b);
FUNCTION bool32 match(const String* a, const char* b);
FUNCTION inline bool32 match(const char* a, const String* b);
FUNCTION bool32 match(const char* a, const char* b);

FUNCTION bool32 match_ignore_case(const String* a, const String* b);
FUNCTION bool32 match_ignore_case(const String* a, const char* b);
FUNCTION inline bool32 match_ignore_case(const char* a, const String* b);
FUNCTION bool32 match_ignore_case(const char* a, const char* b);

FUNCTION s32 find(const String* s, s32 start, char c);
FUNCTION s32 find(const char* s, s32 start, char c);
FUNCTION s32 find(const String* s, s32 start, const String* thing);
FUNCTION s32 find(const String* s, s32 start, const char* thing);
FUNCTION s32 find(const char* s, s32 start, const String* thing);
FUNCTION s32 find(const char* s, s32 start, const char* thing);

FUNCTION inline s32 find(const String* s, char c);
FUNCTION inline s32 find(const char* s, char c);
FUNCTION inline s32 find(const String* s, const String* thing);
FUNCTION inline s32 find(const String* s, const char* thing);
FUNCTION inline s32 find(const char* s, const String* thing);
FUNCTION inline s32 find(const char* s, const char* thing);

FUNCTION s32 find_ignore_case(const String* s, s32 start, char c);
FUNCTION s32 find_ignore_case(const char* s, s32 start, char c);
FUNCTION s32 find_ignore_case(const String* s, s32 start, const String* thing);
FUNCTION s32 find_ignore_case(const String* s, s32 start, const char* thing);
FUNCTION s32 find_ignore_case(const char* s, s32 start, const String* thing);
FUNCTION s32 find_ignore_case(const char* s, s32 start, const char* thing);

FUNCTION inline s32 find_ignore_case(const String* s, char c);
FUNCTION inline s32 find_ignore_case(const char* s, char c);
FUNCTION inline s32 find_ignore_case(const String* s, const String* thing);
FUNCTION inline s32 find_ignore_case(const String* s, const char* thing);
FUNCTION inline s32 find_ignore_case(const char* s, const String* thing);
FUNCTION inline s32 find_ignore_case(const char* s, const char* thing);

FUNCTION inline bool32 contains(const String* s, char c);
FUNCTION inline bool32 contains(const char* s, char c);
FUNCTION inline bool32 contains(const String* s, const String* thing);
FUNCTION inline bool32 contains(const String* s, const char* thing);
FUNCTION inline bool32 contains(const char* s,   const String* thing);
FUNCTION inline bool32 contains(const char* s,   const char* thing);

FUNCTION inline bool32 contains_ignore_case(const String* s, String* thing);
FUNCTION inline bool32 contains_ignore_case(const String* s, const char* thing);
FUNCTION inline bool32 contains_ignore_case(const char* s,   String* thing);
FUNCTION inline bool32 contains_ignore_case(const char* s,   const char* thing);

FUNCTION void to_lower(String* s);
FUNCTION void to_upper(String* s);
FUNCTION void to_lower(char* s);
FUNCTION void to_upper(char* s);
FUNCTION void trim_whitespace(String* s);

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
FUNCTION inline bool32 reader_at_beginning(StringReader* sr);
FUNCTION inline bool32 reader_at_end(StringReader* sr);
FUNCTION inline bool32 valid_index(StringReader *sr, s32 index);
FUNCTION s32 seek_next_char(StringReader* sr, char c);
FUNCTION s32 seek_next_string(StringReader* sr, const char* s);
FUNCTION s32 seek_next_string(StringReader* sr, const String* s);
FUNCTION s32 seek_past_whitespace(StringReader* sr);
FUNCTION s32 seek_next_whitespace(StringReader* sr);
FUNCTION s32 seek_next_alpha(StringReader* sr);
FUNCTION s32 seek_next_number(StringReader* sr);
FUNCTION s32 seek_next_alphanumeric(StringReader* sr);
FUNCTION s32 seek_next_non_alphanumeric(StringReader* sr);
FUNCTION s32 seek_next_line(StringReader* sr,
                            NewLineType newLineType = END_OF_LINE);

FUNCTION u64 parse_u64(const String* s);
FUNCTION u64 parse_u64(const char* s);



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
    //       that char is a signed number, if its unsigned we risk overflow
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
equals(const char* a, const String* b)
{
    return equals(b, a);
}

FUNCTION inline bool32
equals_ignore_case(const char* a, const String* b)
{
    return equals_ignore_case(b, a);
}

FUNCTION inline bool32 
match(const char* a, String* b)
{
    return match(b, a);
}

FUNCTION inline bool32 
match_ignore_case(const char* a, const String* b)
{
    return match_ignore_case(b, a);
}

FUNCTION inline s32 
find(const String* s, char c)
{
    return find(s, 0, c);
}

FUNCTION inline s32 
find(const char* s, char c)
{
    return find(s, 0, c);
}

FUNCTION inline s32
find(const String* s, const String* thing)
{
    return find(s, 0, thing);
}

FUNCTION inline s32
find(const String* s, const char* thing)
{
    return find(s, 0, thing);
}

FUNCTION inline s32
find(const char* s, const String* thing)
{
    return find(s, 0, thing);
}

FUNCTION inline s32
find(const char* s, const char* thing)
{
    return find(s, 0, thing);
}

FUNCTION inline s32 
find_ignore_case(const String* s, char c)
{
    return find_ignore_case(s, 0, c);
}

FUNCTION inline s32 
find_ignore_case(const char* s, char c)
{
    return find_ignore_case(s, 0, c);
}

FUNCTION inline s32
find_ignore_case(const String* s, const String* thing)
{
    return find_ignore_case(s, 0, thing);
}

FUNCTION inline s32
find_ignore_case(const String* s, const char* thing)
{
    return find_ignore_case(s, 0, thing);
}

FUNCTION inline s32
find_ignore_case(const char* s, const String* thing)
{
    return find_ignore_case(s, 0, thing);
}

FUNCTION inline s32
find_ignore_case(const char* s, const char* thing)
{
    return find_ignore_case(s, 0, thing);
}

FUNCTION inline bool32
contains(const String* s, char c)
{
    return -1 != find(s, 0, c);
}

FUNCTION inline bool32
contains(const char* s, char c)
{
    return -1 != find(s, 0, c);
}

FUNCTION inline bool32 
contains(const String* s, const char* thing)
{
    return -1 != find(s, 0, thing);
}

FUNCTION inline bool32
contains(const char* s, String* thing)
{
    return -1 != find(s, 0, thing);
}

FUNCTION inline bool32
contains(const String* s, String* thing)
{
    return -1 != find(s, 0, thing);
}

FUNCTION inline bool32
contains(const char* s, const char* thing)
{
    return -1 != find(s, 0, thing);
}

FUNCTION inline bool32
contains_ignore_case(const String* s, const char* thing)
{
    return -1 != find_ignore_case(s, 0, thing);
}

FUNCTION inline bool32
contains_ignore_case(const char* s, String* thing)
{
    return -1 != find_ignore_case(s, 0, thing);
}

FUNCTION inline bool32
contains_ignore_case(const String* s, String* thing)
{
    return -1 != find_ignore_case(s, 0, thing);
}

FUNCTION inline bool32 
contains_ignore_case(const char* s, const char* thing)
{
    return -1 != find_ignore_case(s, 0, thing);
}


//
// static implementations of my string library
//

#ifdef DRAC_STRING_IMPLEMENTATIONS

#pragma region String Implementations

FUNCTION String
string_from_buffer(void* buffer, u64 bufferLength)
{
    char* alignedBuffer = (char*)align_8((uptr)buffer);
    assert((uptr)alignedBuffer > 0);
    
    s64 diff = abs((char*)buffer - alignedBuffer);
    
    String s;
    s.length = 0;
    s.capacity = bufferLength - diff;
    s.str = alignedBuffer;
    s.str[0] = NULL;
    return s;
}

FUNCTION inline bool32
check_string_n_more_bytes(String* s, s64 extra)
{
    return s->capacity > s->length + extra;
}

FUNCTION void
string_clear(String* s)
{
    memzero(s->str, s->length);
    s->length = 0;
}

FUNCTION void 
append_nchars(String* s, const char* str, u32 nchars)
{
    bool32 result = check_string_n_more_bytes(s, nchars);
    assert_msg(result, "tried to overwrite string buffer");
    if (!result) return;
    
    for(u32 i = 0;
        (i < nchars) && (i < (s->capacity-1));
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
        if (!char_is_whitespace(s->str[i])) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION bool32
string_is_null_or_empty(const char* s)
{
    return !(s && s[0]);
}

FUNCTION bool32
string_is_null_or_whitespace(const char* s)
{
    if (string_is_null_or_empty(s)) return BOOL_TRUE;
    for_str(s)
    {
        if (!char_is_whitespace(*it)) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

// @TODO unroll loops??
FUNCTION bool32
equals(const String* a, const String* b)
{
    if (a->length != b->length) return BOOL_FALSE;
    for(s32 i = 0; i < a->length; ++i)
    {
        if (a->str[i] != b->str[i]) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION bool32 
equals(const String* a, const char* b)
{
    s32 i = 0;
    for(; i < a->length; ++i)
    {
        if (a->str[i] != b[i]) return BOOL_FALSE;
    }
    if (!b[i]) return BOOL_FALSE;
    return BOOL_TRUE;
}

FUNCTION bool32 
equals(const char* a, const char* b)
{
    for(s32 i = 0; ; ++i)
    {
        if (a[i] != b[i]) return BOOL_FALSE;
        if (!a[i]) return BOOL_TRUE;
    }
}

FUNCTION bool32
equals_ignore_case(const String* a, const String* b)
{
    if (a->length != b->length) return BOOL_FALSE;
    for(s32 i = 0; i < a->length; ++i)
    {
        if (!char_equals_ignore_case(a->str[i], b->str[i])) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION bool32
equals_ignore_case(const String* a, const char* b)
{
    s32 i = 0;
    for(; i < a->length; ++i)
    {
        if (!char_equals_ignore_case(a->str[i], b[i])) return BOOL_FALSE;
    }
    if (!b[i]) return BOOL_FALSE;
    return BOOL_TRUE;
}

FUNCTION bool32
equals_ignore_case(const char* a, const char* b)
{
    for(s32 i = 0; ; ++i)
    {
        if (!char_equals_ignore_case(a[i], b[i])) return BOOL_FALSE;
        if (!a[i]) return BOOL_TRUE;
    }
}

FUNCTION bool32 
match(const String* a, const String* b)
{
    for(s32 i = 0; (i < a->length) && (i < b->length); ++i)
    {
        if (a->str[i] != b->str[i]) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION bool32 
match(const String* a, const char* b)
{
    for(s32 i = 0; i < a->length; ++i)
    {
        if (!b[i]) return BOOL_TRUE;
        if (a->str[i] != b[i]) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION bool32 
match(const char* a, const char* b)
{
    for(s32 i = 0; /* go forever */; ++i)
    {
        if (!(a[i] && b[i])) return BOOL_TRUE;
        if (a[i] != b[i]) return BOOL_FALSE;
    }
}

FUNCTION bool32 
match_ignore_case(const String* a, const String* b)
{
    for(s32 i = 0; (i < a->length) && (i < b->length); ++i)
    {
        if (!char_equals_ignore_case(a->str[i], b->str[i])) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION bool32 
match_ignore_case(const String* a, const char* b)
{
    for(s32 i = 0; i < a->length; ++i)
    {
        if (!b[i]) return BOOL_TRUE;
        if (!char_equals_ignore_case(a->str[i], b[i])) return BOOL_FALSE;
    }
    return BOOL_TRUE;
}

FUNCTION bool32 
match_ignore_case(const char* a, const char* b)
{
    for(s32 i = 0; /* go forever */; ++i)
    {
        if (!(a[i] && b[i])) return BOOL_TRUE;
        if (!char_equals_ignore_case(a[i], b[i])) return BOOL_FALSE;
    }
}

FUNCTION s32
find(const String* s, s32 start, char c)
{
    if (start >= s->length) return -1;
    for_str(s->str + start)
    {
        if (*it == c) return (s32)(it - s->str);
    }
    return -1;
}

FUNCTION s32
find(const char* s, s32 start, char c)
{
    for_str(s + start)
    {
        if (*it == c) return (s32)(it - s);
    }
    return -1;
}

FUNCTION s32 
find(const String* s, s32 start, const String* thing)
{
    if (thing->length > s->length) return -1;
    s32 seekDist = s->length - thing->length;
    for(s32 i = start; i < seekDist; ++i)
    {
        if (s->str[i] == thing->str[0])
        {
            s32 j = 1;
            while(j < thing->length && 
                  s->str[i+j] == thing->str[j]) 
            {
                ++j;
            }
            if (j == thing->length) return i;
        }
    }
    return -1;
}

FUNCTION s32 
find(const String* s, s32 start, const char* thing)
{
    for(s32 i = start; i < s->length; ++i)
    {
        if (s->str[i] == thing[0])
        {
            s32 j = 1;
            while(thing[j] && 
                  s->str[i+j] == thing[j]) 
            {
                ++j;
            }
            if (!thing[j]) return i;
        }
    }
    return -1;
}

FUNCTION s32 
find(const char* s, s32 start, const String* thing)
{
    for(s32 i = start; s[i]; ++i)
    {
        if (s[i] == thing->str[0])
        {
            s32 j = 1;
            while(j < thing->length && 
                  s[i+j] == thing->str[j]) 
            {
                ++j;
            }
            if (j == thing->length) return i;
        }
    }
    return -1;
}

FUNCTION s32
find(const char* s, s32 start, const char* thing)
{
    for(s32 i = start; s[i]; ++i)
    {
        if (s[i] == thing[0])
        {
            s32 j = 1;
            while(thing[j] && 
                  s[i+j] == thing[j]) 
            {
                ++j;
            }
            if (!thing[j]) return i;
        }
    }
    return -1;
}

FUNCTION s32
find_ignore_case(const String* s, s32 start, char c)
{
    if (start >= s->length) return -1;
    for_str(s->str + start)
    {
        if (char_equals_ignore_case(*it, c)) return (s32)(it - s->str);
    }
    return -1;
}

FUNCTION s32
find_ignore_case(const char* s, s32 start, char c)
{
    for_str(s + start)
    {
        if (char_equals_ignore_case(*it, c)) return (s32)(it - s);
    }
    return -1;
}

FUNCTION s32 
find_ignore_case(const String* s, s32 start, const String* thing)
{
    if (thing->length > s->length) return -1;
    s32 seekDist = s->length - thing->length;
    for(s32 i = start; i < seekDist; ++i)
    {
        if (char_equals_ignore_case(s->str[i], thing->str[0]))
        {
            s32 j = 1;
            while(j < thing->length && 
                  char_equals_ignore_case(s->str[i+j], thing->str[j]))
            {
                ++j;
            }
            if (j == thing->length) return i;
        }
    }
    return -1;
}

FUNCTION s32 
find_ignore_case(const String* s, s32 start, const char* thing)
{
    for(s32 i = start; i < s->length; ++i)
    {
        if (char_equals_ignore_case(s->str[i], thing[0]))
        {
            s32 j = 1;
            while(thing[j] && 
                  char_equals_ignore_case(s->str[i+j], thing[j])) 
            {
                ++j;
            }
            if (!thing[j]) return i;
        }
    }
    return -1;
}

FUNCTION s32
find_ignore_case(const char* s, s32 start, const String* thing)
{
    for(s32 i = start; s[i]; ++i)
    {
        if (char_equals_ignore_case(s[i], thing->str[0]))
        {
            s32 j = 1;
            while(j < thing->length && 
                  char_equals_ignore_case(s[i+j], thing->str[j]))
            {
                ++j;
            }
            if (j == thing->length) return i;
        }
    }
    return -1;
}

FUNCTION s32
find_ignore_case(const char* s, s32 start, const char* thing)
{
    for(s32 i = start; s[i]; ++i)
    {
        if (char_equals_ignore_case(s[i], thing[0]))
        {
            s32 j = 1;
            while(thing[j] && 
                  char_equals_ignore_case(s[i+j], thing[j]))
            {
                ++j;
            }
            if (!thing[j]) return i;
        }
    }
    return -1;
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
seek_next_char(StringReader* sr, char c)
{
    if (sr->cursor >= sr->length) return sr->length;
    s32 findIndex = find(sr->string, sr->cursor, c);
    if (-1 == findIndex)
    {
        sr->cursor = sr->length;
        return sr->length;
    }
    sr->cursor = min(findIndex + 1, sr->length); 
    return findIndex;
}

FUNCTION s32
seek_next_string(StringReader* sr, const char* s)
{
    if (sr->cursor >= sr->length) return sr->length;
    s32 seekLength = (s32)strlen(s);
    s32 findIndex = find(sr->string, sr->cursor, s);
    if (-1 == findIndex)
    {
        sr->cursor = sr->length;
        return sr->length;
    }
    sr->cursor = min(findIndex + seekLength, sr->length); 
    return findIndex;
}

FUNCTION s32
seek_next_string(StringReader* sr, const String* s)
{
    if (sr->cursor >= sr->length) return sr->length;
    s32 seekLength = s->length;
    s32 findIndex = find(sr->string, sr->cursor, s);
    if (-1 == findIndex)
    {
        sr->cursor = sr->length;
        return sr->length;
    }
    sr->cursor = min(findIndex + seekLength, sr->length); 
    return findIndex;
}

FUNCTION s32
seek_past_whitespace(StringReader* sr)
{
    while (sr->cursor < sr->length &&
           char_is_whitespace(sr->string[sr->cursor]))
    {
        sr->cursor++;
    }
    return sr->cursor;
}

FUNCTION s32
seek_next_whitespace(StringReader* sr)
{
    s32 nextCursor = max(0, sr->cursor);
    while (nextCursor < sr->length && 
           !char_is_whitespace(sr->string[nextCursor]))
    {
        ++nextCursor;
    }
    sr->cursor = min(nextCursor + 1, sr->length); 
    return nextCursor;
}

FUNCTION s32
seek_next_alpha(StringReader* sr)
{
    s32 nextCursor = max(0, sr->cursor);
    while (nextCursor < sr->length &&
           !char_is_alpha(sr->string[nextCursor]))
    {
        ++nextCursor;
    }
    sr->cursor = min(nextCursor + 1, sr->length);
    return nextCursor;
}

FUNCTION s32
seek_next_number(StringReader* sr)
{
    s32 nextCursor = max(0, sr->cursor);
    while (nextCursor < sr->length &&
           !char_is_number(sr->string[nextCursor]))
    {
        ++nextCursor;
    }
    sr->cursor = min(nextCursor + 1, sr->length);
    return nextCursor;
}

FUNCTION s32 
seek_next_alphanumeric(StringReader* sr)
{
    s32 nextCursor = max(0, sr->cursor);
    while (nextCursor < sr->length &&
           !char_is_alphanumeric(sr->string[nextCursor]))
    {
        nextCursor++;
    }
    sr->cursor = min(nextCursor + 1, sr->length);
    return nextCursor;
}

FUNCTION s32
seek_next_non_alphanumeric(StringReader* sr)
{
    s32 nextCursor = max(0, sr->cursor);
    while (nextCursor < sr->length &&
           char_is_alphanumeric(sr->string[nextCursor]))
    {
        nextCursor++;
    }
    sr->cursor = min(nextCursor + 1, sr->length);
    return nextCursor;
}

FUNCTION s32
seek_next_line(StringReader* sr, NewLineType newLineType)
{
    switch (newLineType)
    {
        case CARRIAGE_RETURN:
        {
            seek_next_char(sr, '\r');
        } break;
        
        case LINE_FEED:
        {
            seek_next_char(sr, '\n');
        } break;
        
        case END_OF_LINE:
        {
            seek_next_string(sr, "\r\n");
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

#endif /* DRAC_STRING_IMPLEMENTATIONS */

#endif /* DRAC_STRING_H */