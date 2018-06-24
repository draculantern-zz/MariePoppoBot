#ifndef PLATFORM_H
#define PLATFORM_H

#define GLOBAL static

#define LOCAL_STATIC static

#ifndef FUNCTION
#define FUNCTION static
#endif

#ifndef FORCE_INLINE 
#define FORCE_INLINE inline
#endif

#include <stdint.h>

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       f32;
typedef double      f64;

typedef int32_t     bool32;

typedef u8          byte;

typedef uintptr_t   uptr;
typedef intptr_t    iptr;


#define null 0

#define BOOL_TRUE  1
#define BOOL_FALSE 0

#define s8_MIN  0x80i8
#define s8_MAX  0x7Fi8
#define s16_MIN 0x8000i16
#define s16_MAX 0x7FFFi16
#define s32_MIN 0x80000000i32
#define s32_MAX 0x7FFFFFFFi32
#define s64_MIN 0x8000000000000000i64
#define s64_MAX 0x7FFFFFFFFFFFFFFFi64

#define u8_MIN  0
#define u8_MAX  0xFFu
#define u16_MIN 0
#define u16_MAX 0xFFFFu
#define u32_MIN 0
#define u32_MAX 0xFFFFFFFFu
#define u64_MIN 0
#define u64_MAX 0xFFFFFFFFFFFFFFFFuLL

// smallest such that 1.0+f32_EPSILON != 1.0
#define f32_EPSILON   1.192092896e-07f
// max value
#define f32_MAX       3.402823466e+38f
// min positive value
#define f32_MIN       1.175494351e-38f

// smallest such that 1.0+f64_EPSILON != 1.0
#define f64_EPSILON   2.2204460492503131e-016
// max value
#define f64_MAX       1.7976931348623158e+308
// min positive value
#define f64_MIN       2.2250738585072014e-308

#define kilobytes(n) ((n) * (1uLL<<10))
#define megabytes(n) ((n) * (1uLL<<20))
#define gigabytes(n) ((n) * (1uLL<<30))

#define assert_power_of_2(_integer) assert(((_integer) & ((_integer) - 1)) == 0)
#define ARRAY_LENGTH(array_literal) sizeof(array_literal) / sizeof(array_literal[0])
#define offset_of(_struct,_member) ((u64)&(((_struct*)0)->_member))
#define alignment_of_struct(_struct) alignof(_struct)

#define swap_bitwise(a, b) \
do { \
    a = (a) ^ (b);  \
    b = (a) ^ (b);  \
    a = (a) ^ (b);  \
} while(0)

//
// defer, execute when something goes out of scope
#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)

#define defer const auto& CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

template <typename T>
struct ExitScope
{
    public:
    T deferred;
    ExitScope(T _lambda): deferred(_lambda) { }
    ~ExitScope() { deferred(); }
    ExitScope(const ExitScope&) = delete;
    private:
    ExitScope& operator=(const ExitScope&) = delete;
};
class ExitScopeHelp
{
    public:
    template<typename T>
        ExitScope<T> operator+(T t) { return t; }
};

#define for_str(str_) \
for (auto it = str_;  \
*it != 0;        \
++it)


#define CRASH *(volatile int*)0 = 0
#define NOT_IMPLEMENTED assert(!"function not implemented!")
#define CONST_LITERAL_TO_STRING(x) #x
#define TO_STRING(x) CONST_LITERAL_TO_STRING(x)
#define HAS_BITMASK(flags, bits) (((flags) | (bits)) == (flags))
#define PFN(fn) PFN_##fn

#define IF_DEBUG if(DEBUG)

#if DEBUG

#   define assert(_expr) \
if (!(_expr)) { \
    PLATFORM_LOG_ERROR("\nAssertion Failed: " #_expr    \
    "\nFile: " TO_STRING(__FILE__)   \
    "\nLine: " TO_STRING(__LINE__)); \
    CRASH;\
}

#   define assert_msg(_expr, msg) \
if (!(_expr)) { \
    PLATFORM_LOG_ERROR("\nAssertion Failed: " msg       \
    "\nFile: " TO_STRING(__FILE__)   \
    "\nLine: " TO_STRING(__LINE__)); \
    CRASH;\
}

#else
//#   define assert(_expr) ((void)0)
#   define assert(_expr) 
#   define assert_msg(_expr, msg) 
#endif


#define align_2(val) (((val) + 1) & ~1)
#define align_4(val) (((val) + 3) & ~3)
#define align_8(val) (((val) + 7) & ~7)
#define align_16(val) (((val) + 15) & ~15)
#define align_32(val) (((val) + 31) & ~31)
#define align_64(val) (((val) + 63) & ~63)
#define align_128(val) (((val) + 127) & ~127)
#define align_256(val) (((val) + 255) & ~255)
#define align_512(val) (((val) + 511) & ~511)
#define align_1024(val) (((val) + 1023) & ~1023)
#define align_2048(val) (((val) + 2047) & ~2047)
#define align_4096(val) (((val) + 4095) & ~4095)
#define align_n(val, alignment) (((val) + ((alignment)-1)) & ~((alignment)-1))
#define align_down_n(val, alignment) ((val) & ~((alignment)-1))

#define abs(x) ( ((x) >= 0) ? (x) : (-(x)) )
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define clamp(x, low, high) (min(max((x), (low)), (high)))
#define lerp(a, b, T) ((a) + ((T) * ((b) - (a))))

#define memzero(data, numbytes) memset(data, 0, numbytes)


#if !defined(PLATFORM_MALLOC) && !defined(PLATFORM_FREE) && !defined(PLATFORM_REALLOC)

#   define PLATFORM_MALLOC(nbytes) malloc(nbytes)
#   define PLATFORM_FREE(ptr) free(ptr)
#   define PLATFORM_REALLOC(ptr,nbytes) realloc(ptr,nbytes)

#elif (defined(PLATFORM_MALLOC) && \
(!defined(PLATFORM_FREE) || !defined(PLATFORM_REALLOC))) \
|| (defined(PLATFORM_FREE) && \
(!defined(PLATFORM_MALLOC) || !defined(PLATFORM_REALLOC))) \
|| (defined(PLATFORM_REALLOC) && \
(!defined(PLATFORM_MALLOC) || !defined(PLATFORM_FREE)))

#   error Must define all of PLATFORM_MALLOC, PLATFORM_FREE, PLATFORM_REALLOC or none of them

#endif /* Platform allocation defs */

#if !defined(PLATFORM_LOG)
#   define PLATFORM_LOG(str) printf(str)
#endif

#if !defined(PLATFORM_LOG_ERROR)
#   define PLATFORM_LOG_ERROR(str) printf(str)
#endif

#if !defined(PLATFORM_SLEEP)
#   define PLATFORM_SLEEP(x) sleep(x)
#endif



#endif /* PLATFORM_H */