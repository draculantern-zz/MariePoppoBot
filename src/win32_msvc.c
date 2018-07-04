
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#include <xmmintrin.h>
#include <emmintrin.h>

#define alignN(val, alignment) (((val) + ((alignment)-1)) & ~((alignment)-1))

int _fltused = 0x9875;

#pragma function(fabs)
double fabs(double x)
{
    union {
        int64_t i;
        double d;
    } u;
    u.d = x;
    u.i = u.i & 0x7fffffffffffffff;
    return u.d;
}

float fabsf(float x)
{
    union {
        int32_t i;
        float f;
    } u;
    u.f = x;
    u.i = u.i & 0x7fffffff;
    return u.f;
}

//
// strlen
//

#pragma function(strlen)
size_t strlen(const char* str)
{
#if _WIN64
#define mask01 0x0101010101010101
#define mask80 0x8080808080808080
#define PTR_MASK (sizeof(uint64_t) - 1)
    
    const char *p;
    const uint64_t *lp;
    int64_t va, vb;
    
    lp = (const uint64_t*)((uintptr_t)str & ~PTR_MASK);
    va = (*lp - mask01);
    vb = ((~*lp) & mask80);
    lp++;
    if (va & vb)
    {
        for (p = str; p < (const char *)lp; p++) {
            if (!*p) return (p - str);   
        }
    }
    for (;; ++lp) 
    {
        va = (*lp - mask01);
        vb = ((~*lp) & mask80);
        if (va & vb) 
        {
            uint64_t x = *lp;
            if(!(x & 0xFF)) return (char*)lp - str;
            if(!(x & 0xFF00)) return (char*)lp - str + 1;
            if(!(x & 0xFF0000)) return (char*)lp - str + 2;
            if(!(x & 0xFF000000)) return (char*)lp - str + 3;
            if(!(x & 0xFF00000000)) return (char*)lp - str + 4;
            if(!(x & 0xFF0000000000)) return (char*)lp - str + 5;
            if(!(x & 0xFF000000000000)) return (char*)lp - str + 6;
            if(!(x & 0xFF00000000000000)) return (char*)lp - str + 7;
        }
    }
    
#else 
#define mask01 0x01010101
#define mask80 0x80808080
#define PTR_MASK (sizeof(uint32_t) - 1)
    
    const char *p;
    const uint32_t *lp;
    int32_t va, vb;
    
    lp = (const uint32_t*)((uintptr_t)str & ~PTR_MASK);
    va = (*lp - mask01);
    vb = ((~*lp) & mask80);
    lp++;
    if (va & vb)
    {
        for (p = str; p < (const char *)lp; p++) {
            if (!*p) return (p - str);   
        }
    }
    for (;; ++lp) {
        va = (*lp - mask01);
        vb = ((~*lp) & mask80);
        if (va & vb) 
        {            
            uint32_t x = *lp;
            if(!(x & 0xFF)) return (char*)lp - str;
            if(!(x & 0xFF00)) return (char*)lp - str + 1;
            if(!(x & 0xFF0000)) return (char*)lp - str + 2;
            if(!(x & 0xFF000000)) return (char*)lp - str + 3;
        }
    }
    
#endif
    
#undef mask01
#undef mask80
#undef PTR_MASK
}

//
// memset
//

__forceinline void*
memset_sse2(char* dst, int32_t val, size_t numBytes)
{
    const register int64_t regSize = sizeof(__m128i);
    
    int64_t extraFront = alignN((uintptr_t)dst, regSize) - (int64_t)dst;
    int64_t sseBytes = (numBytes - extraFront) & ~(regSize-1);
    int64_t extraBack = numBytes - sseBytes - extraFront;
    register char* ptr = dst;
    register char set = (char)val;
    
    __m128i xmm0;
    if (set) xmm0 = _mm_set1_epi8(set);
    else     xmm0 = _mm_setzero_si128();
    
    { // set bytes that are not on 16byte alignment offset from the front
        while (extraFront--)
            *ptr++ = set;
    }
    
    int64_t stride = 8 * regSize;
    while(sseBytes >= stride)
    {
        _mm_stream_si128(((__m128i*)ptr) + 0, xmm0);
        _mm_stream_si128(((__m128i*)ptr) + 1, xmm0);
        _mm_stream_si128(((__m128i*)ptr) + 2, xmm0);
        _mm_stream_si128(((__m128i*)ptr) + 3, xmm0);
        _mm_stream_si128(((__m128i*)ptr) + 4, xmm0);
        _mm_stream_si128(((__m128i*)ptr) + 5, xmm0);
        _mm_stream_si128(((__m128i*)ptr) + 6, xmm0);
        _mm_stream_si128(((__m128i*)ptr) + 7, xmm0);
        ptr += stride;
        sseBytes -= stride;
    }
    
    while(sseBytes > (regSize - 1))
    {
        _mm_stream_si128(((__m128i*)ptr), xmm0);
        ptr += regSize;
        sseBytes -= regSize;
    }
    
    { // set bytes that dangle from the back
        while (extraBack--)
            *ptr++ = set;
    }
    
    return dst;
}

#pragma function(memset)
void* memset(void* dst, int32_t val, size_t numBytes)
{
    return memset_sse2((char*)dst, val, numBytes);
} 

//
// memcpy
//

__forceinline void* 
memcpy_sse2(char* dst, char* src, size_t numBytes)
{
    const register uint32_t sseSize = sizeof(__m128i);
    
    int bytesPerCopy = 1;
    if (((uintptr_t)dst % sseSize) != ((uintptr_t)src % sseSize))
    {
        bytesPerCopy = sseSize;
    }
    else if (((uintptr_t)dst % 8) != ((uintptr_t)src % 8))
    {
        bytesPerCopy = 8;
    }
    else if (((uintptr_t)dst % 4) != ((uintptr_t)src % 4))
    {
        bytesPerCopy = 4;
    }
    
    register char* ptr = dst;
    register char* set = src;
    if (1 == bytesPerCopy)
    {
        while (numBytes--) 
        {
            *ptr = *set;
            ptr++; set++;
        }
        return dst;
    }
    
    int64_t extraFront = alignN((uintptr_t)dst, bytesPerCopy) - (int64_t)dst;
    int64_t middleBytes = (numBytes - extraFront) & ~(bytesPerCopy-1);
    int64_t extraBack = numBytes - middleBytes - extraFront;
    
    { // set bytes that are not aligned from the front
        while (extraFront--) 
        {
            *ptr = *set;
            ptr++; set++;
        }
    }
    
    if (sseSize == bytesPerCopy)
    {
        int64_t stride = 2 * bytesPerCopy;
        while (middleBytes >= stride)
        {
            _mm_stream_si128(((__m128i*)ptr) + 0, _mm_load_si128((__m128i*)set + 0));
            _mm_stream_si128(((__m128i*)ptr) + 1, _mm_load_si128((__m128i*)set + 1));
            
            ptr += stride;
            set += stride;
            middleBytes -= stride;
        }
        
        while(middleBytes > 0)
        {
            _mm_stream_si128((__m128i*)ptr, _mm_load_si128((__m128i*)set));
            ptr += bytesPerCopy;
            set += bytesPerCopy;
            middleBytes -= bytesPerCopy;
        }
    }
    else if (8 == bytesPerCopy)
    {
        int64_t stride = 8 * bytesPerCopy;
        while (middleBytes >= stride)
        {
            *((uint64_t*)ptr + 0) = *((uint64_t*)set + 0);
            *((uint64_t*)ptr + 1) = *((uint64_t*)set + 1);
            *((uint64_t*)ptr + 2) = *((uint64_t*)set + 2);
            *((uint64_t*)ptr + 3) = *((uint64_t*)set + 3);
            *((uint64_t*)ptr + 4) = *((uint64_t*)set + 4);
            *((uint64_t*)ptr + 5) = *((uint64_t*)set + 5);
            *((uint64_t*)ptr + 6) = *((uint64_t*)set + 6);
            *((uint64_t*)ptr + 7) = *((uint64_t*)set + 7);
            
            ptr += stride;
            set += stride;
            middleBytes -= stride;
        }
        
        while(middleBytes > 0)
        {
            *(uint64_t*)ptr = *(uint64_t*)set;
            ptr += bytesPerCopy;
            set += bytesPerCopy;
            middleBytes -= bytesPerCopy;
        }
    }
    else if (4 == bytesPerCopy)
    {
        int64_t stride = 8 * bytesPerCopy;
        while (middleBytes >= stride)
        {
            *((uint32_t*)ptr + 0) = *((uint32_t*)set + 0);
            *((uint32_t*)ptr + 1) = *((uint32_t*)set + 1);
            *((uint32_t*)ptr + 2) = *((uint32_t*)set + 2);
            *((uint32_t*)ptr + 3) = *((uint32_t*)set + 3);
            *((uint32_t*)ptr + 4) = *((uint32_t*)set + 4);
            *((uint32_t*)ptr + 5) = *((uint32_t*)set + 5);
            *((uint32_t*)ptr + 6) = *((uint32_t*)set + 6);
            *((uint32_t*)ptr + 7) = *((uint32_t*)set + 7);
            
            ptr += stride;
            set += stride;
            middleBytes -= stride;
        }
        
        while(middleBytes > 0)
        {
            *(uint32_t*)ptr = *(uint32_t*)set;
            ptr += bytesPerCopy;
            set += bytesPerCopy;
            middleBytes -= bytesPerCopy;
        }
    }
    
    { // set unaligned bytes that dangle from the back
        while (extraBack--) 
        {
            *ptr = *set;
            ptr++; set++;
        }
    }
    
    return dst;
}

#pragma function(memcpy)
void* memcpy(void* dst, void const* src, size_t numBytes)
{
    return memcpy_sse2((char*)dst, (char*)src, numBytes);
}


//
// memmove
//

__forceinline void*
memmove_sse2(char* dst, char* src, size_t numBytes)
{
    // if copy from front or no overlap, use memcpy
    if (((uintptr_t)dst < (uintptr_t)src) || ((uintptr_t)dst - numBytes > (uintptr_t)src))
    {
        return memcpy_sse2(dst, src, numBytes);
    }
    
    // if we get here, always copy from the back
    
    const register int64_t sseSize = sizeof(__m128i);
    int bytesPerCopy = 1;
    if (((uintptr_t)dst % sseSize) != ((uintptr_t)src % sseSize))
    {
        bytesPerCopy = sseSize;
    }
    else if (((uintptr_t)dst % 8) != ((uintptr_t)src % 8))
    {
        bytesPerCopy = 8;
    }
    else if (((uintptr_t)dst % 4) != ((uintptr_t)src % 4))
    {
        bytesPerCopy = 4;
    }
    
    register char* ptr = dst + numBytes;
    register char* set = src + numBytes;
    
    // copy from back of src
    if (1 == bytesPerCopy)
    {
        while (numBytes--) 
        {
            ptr--; set--;
            *ptr = *set;
        }
        return dst;
    }
    
    int64_t extraFront = alignN((uintptr_t)dst, bytesPerCopy) - (int64_t)dst;
    int64_t middleBytes = (numBytes - extraFront) & ~(bytesPerCopy-1);
    int64_t extraBack = numBytes - middleBytes - extraFront;
    
    { // set unaligned bytes that dangle from the back
        while (extraBack--) 
        {
            ptr--; set--;
            *ptr = *set;
        }
    }
    
    if (sseSize == bytesPerCopy)
    {
        int64_t stride = 2 * bytesPerCopy;
        while (middleBytes >= stride)
        {
            ptr -= stride;
            set -= stride;
            
            _mm_stream_si128(((__m128i*)ptr) + 1, _mm_load_si128((__m128i*)set + 1));
            _mm_stream_si128(((__m128i*)ptr) + 0, _mm_load_si128((__m128i*)set + 0));
            
            middleBytes -= stride;
        }
        
        while(middleBytes > 0)
        {
            ptr -= bytesPerCopy;
            set -= bytesPerCopy;
            _mm_stream_si128((__m128i*)ptr, _mm_load_si128((__m128i*)set));
            middleBytes -= bytesPerCopy;
        }
    }
    else if (8 == bytesPerCopy)
    {
        int64_t stride = 8 * bytesPerCopy;
        while (middleBytes >= stride)
        {
            ptr -= stride;
            set -= stride;
            
            *((uint64_t*)ptr + 7) = *((uint64_t*)set + 7);
            *((uint64_t*)ptr + 6) = *((uint64_t*)set + 6);
            *((uint64_t*)ptr + 5) = *((uint64_t*)set + 5);
            *((uint64_t*)ptr + 4) = *((uint64_t*)set + 4);
            *((uint64_t*)ptr + 3) = *((uint64_t*)set + 3);
            *((uint64_t*)ptr + 2) = *((uint64_t*)set + 2);
            *((uint64_t*)ptr + 1) = *((uint64_t*)set + 1);
            *((uint64_t*)ptr + 0) = *((uint64_t*)set + 0);
            
            middleBytes -= stride;
        }
        
        while(middleBytes > 0)
        {
            ptr -= bytesPerCopy;
            set -= bytesPerCopy;
            *(uint64_t*)ptr = *(uint64_t*)set;
            middleBytes -= bytesPerCopy;
        }
    }
    else if (4 == bytesPerCopy)
    {
        int64_t stride = 8 * bytesPerCopy;
        while (middleBytes >= stride)
        {
            ptr -= stride;
            set -= stride;
            
            *((uint32_t*)ptr + 7) = *((uint32_t*)set + 7);
            *((uint32_t*)ptr + 6) = *((uint32_t*)set + 6);
            *((uint32_t*)ptr + 5) = *((uint32_t*)set + 5);
            *((uint32_t*)ptr + 4) = *((uint32_t*)set + 4);
            *((uint32_t*)ptr + 3) = *((uint32_t*)set + 3);
            *((uint32_t*)ptr + 2) = *((uint32_t*)set + 2);
            *((uint32_t*)ptr + 1) = *((uint32_t*)set + 1);
            *((uint32_t*)ptr + 0) = *((uint32_t*)set + 0);
            
            middleBytes -= stride;
        }
        
        while(middleBytes > 0)
        {
            ptr -= bytesPerCopy;
            set -= bytesPerCopy;
            *(uint32_t*)ptr = *(uint32_t*)set;
            middleBytes -= bytesPerCopy;
        }
    }
    
    { // set bytes that are not aligned from the front
        while (extraFront--) 
        {
            ptr--; set--;
            *ptr = *set;
        }
    }
    
    return dst;
}

#pragma function(memmove)
void* memmove(void* dst, void const* src, size_t numBytes)
{
    return memmove_sse2((char*)dst, (char*)src, numBytes);
}


//
// https://en.wikipedia.org/wiki/Taylor_series
//

#pragma function(sinf)
float sinf(float radians)
{
    static const float taylor_1 = -0.166666666666666666666666666667f;
    static const float taylor_2 =  0.008333333333333333333333333333f;
    static const float taylor_3 = -1.984126984126984126984126984127e-4f; 
    static const float taylor_4 =  2.755731922398589065255731922399e-6f;
    
    float x = radians;
    float x2 = x*x;
    float x3 = x*x2;
    float x5 = x3*x2;
    float x7 = x5*x2;
    float x9 = x7*x2;
    return x + x3*taylor_1 + x5*taylor_2 + x7*taylor_3 + x9*taylor_4;
}

#pragma function(cosf)
float cosf(float radians)
{
    static const float taylor_1 = -0.5f;
    static const float taylor_2 =  0.0416666666666666666666666666667f;
    static const float taylor_3 = -0.0013888888888888888888888888889f;
    static const float taylor_4 =  2.4801587301587301587301587301587e-5f;
    
    float x = radians;
    float x2 = x*x;
    float x4 = x2*x2;
    float x6 = x4*x2;
    float x8 = x4*x4;
    return 1.0f + x2*taylor_1 + x4*taylor_2 + x6*taylor_3 + x8*taylor_4;
}

//
// At first I followed the implementation of tan and arctan from the taylor series
// wikipedia article, but its seems that they converge very poorly for values close to +/-pi
// meaning we would need to do a lot of iterations (and cycles) to fully calculate
// So I looked online for a public domain implementation and found this
// implementations courtesy of gingerBill 
// https://github.com/gingerBill/gb/blob/master/gb_math.h
//
#pragma function(tanf)
float tanf(float radians)
{
    float rr = radians*radians;
    float a = 9.5168091e-03f;
    a *= rr;
    a += 2.900525e-03f;
    a *= rr;
    a += 2.45650893e-02f;
    a *= rr;
    a += 5.33740603e-02f;
    a *= rr;
    a += 1.333923995e-01f;
    a *= rr;
    a += 3.333314036e-01f;
    a *= rr;
    a += 1.0f;
    a *= radians;
    return a;
}

#pragma function(atanf)
float atanf(float a)
{
    static const float taylor_1 = 0.33288950512027f;
    static const float taylor_2 = 0.08467922817644f;
    static const float taylor_3 = 0.03252232640125f;
    static const float taylor_4 = 0.00749305860992f;
    float u  = a*a;
    float u2 = u*u;
    float u3 = u2*u;
    float u4 = u3*u;
    float f  = 1.0f + taylor_1*u - taylor_2*u2 + taylor_3*u3 - taylor_4*u4;
    return a / f;
}

#pragma function(atan2f)
float atan2f(float y, float x)
{
    const static float pi = 3.14159265358979323846264338327950288f;
    const static float pi_over_2 = 1.570796326794896619231321691639751442f;
    if (fabsf(x) > fabsf(y))
    {
        float a = atanf(y / x);
        if (x > 0.0f)
            return a;
        else
            return (y > 0.0f) ? (a + pi) : (a - pi);
    } 
    else
    {
        float a = atanf(x / y);
        if (x > 0.0f)
            return (y > 0.0f) ? (pi_over_2 - a) : (-pi_over_2 - a);
        else
            return (y > 0.0f) ? (pi_over_2 + a) : (-pi_over_2 + a);
    }
}

#pragma function(sqrtf)
float sqrtf(float f)
{
    __m128 xmm0 = _mm_set_ss(f);
    __m128 xmm1 = _mm_sqrt_ss(xmm0);
    return _mm_cvtss_f32(xmm1);
}

#pragma function(asinf)
float asinf(float a) 
{ 
    return atan2f(a, sqrtf((1.0f + a) * (1.0f - a))); 
}

#pragma function(acosf)
float acosf(float a)
{
    return atan2f(sqrtf((1.0f + a) * (1.0 - a)), a); 
}

#undef alignN