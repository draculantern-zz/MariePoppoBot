#ifndef WIN32_IRC_CLIENT_H
#define WIN32_IRC_CLIENT_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//
// @TODO fix this to have a real platform layer to the code, 
//       this is just a quick and dirty layer to get stuff done w/o thinking 
//       eventually these won't be macros, but its own object w/ a callback table
//
#define PLATFORM_MALLOC win32_heap_alloc
#define PLATFORM_FREE win32_heap_free
#define PLATFORM_REALLOC win32_heap_realloc

#define PLATFORM_LOG win32_print
#define PLATFORM_LOG_ERROR win32_print

#define FORCE_INLINE __forceinline
#define align_struct(_byte) __declspec(align(_byte))
#define PLATFORM_SLEEP(ms) Sleep(ms)

//#define DRAC_RANDOM_MERSENNE_TWISTER 1
#define DRAC_RANDOM_PCG 1

//
// SIMD archtecture stuff
//
#ifdef _MSC_VER

// archtecture AMD64 (x64) supports SSE
#if  defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1) || \
defined(__SSE__) || defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <mmintrin.h>
#   include <xmmintrin.h>
#   define DRAC_MMX 1
#   define DRAC_SSE 1
#endif /* AMD64 or x86 */

#if  defined(_M_AMD64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || \
defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <emmintrin.h>
#   define DRAC_SSE2 1
#endif /* AMD64 or x64 */

#else /* if we're using a compiler that doesn't do the weird MSVC macros for SSE */

#if defined(__SSE__) || defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <mmintrin.h>
#   include <xmmintrin.h>
#   define DRAC_MMX 1
#   define DRAC_SSE 1
#endif 

#if defined(__SSE2__) || defined(__AVX__) || defined(__AVX2__)
#   include <emmintrin.h>
#   define DRAC_SSE2 1
#endif 

#endif /* _MSC_VER */


#if defined(__AVX__) || defined(__AVX2__)
#   include <immintrin.h>
#   define DRAC_AVX 1
#endif /* AVX or AVX2 */

#if defined(__AVX2__)
#   define DRAC_AVX2 1
#endif /* AVX2 */

#if defined(__SSE3__)
#   include <pmmintrin.h>
#   define DRAC_SSE3 1
#endif

#if defined(__SSSE3__)
#   include <tmmintrin.h>
#   define DRAC_SSSE3 1
#endif

#if defined(__SSE4_1__)
#   include <smmintrin.h>
#   define DRAC_SSE4_1 1
#endif

#if defined(__SSE4_2__)
#   include <nmmintrin.h>
#   define DRAC_SSE4_2 1
#endif


#include "platform.h"

FUNCTION void* win32_heap_alloc(u64 numbytes);
FUNCTION void* win32_heap_realloc(void* data, u64 numbytes);
FUNCTION void  win32_heap_free(void* data);
FUNCTION void  win32_print(const char* buf);

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "twitch.h"

struct Win32TwitchClient
{
    const char* url; 
    const char* port;
    char* nick;
    char* pass;
    char* channel;
    int sendFlags;
    
    SOCKET ircSocket;
    TwitchClient twitchClient;
};



#endif /* WIN32_IRC_CLIENT_H */

