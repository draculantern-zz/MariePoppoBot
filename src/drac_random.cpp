#ifndef DRAC_RANDOM_CPP
#define DRAC_RANDOM_CPP

#include "drac_random.h"


FUNCTION float
normalize_32_bits_to_f32(uint32_t x)
{
    union {
        uint32_t i;
        float    f;
    } result;
    const uint32_t exponent = 127;
    uint32_t mantissa = x >> 9;
    result.i = (exponent << 23) | mantissa;
    return result.f - 1.0f;
}

FUNCTION double
normalize_64_bits_to_f64(uint64_t x)
{
    union {
        uint64_t i;
        double d;
    } result;
    const uint64_t exponent = 1023;
    uint64_t mantissa = x >> 12uLL;
    result.i = (exponent << 52uLL) | mantissa;
    return result.d - 1.0;
}

// gb_* functions copied from public domain library
// https://github.com/gingerBill/gb/blob/master/gb_math.h
FUNCTION uint32_t
gb_rand_murmur3_hash_32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

FUNCTION uint64_t
gb_rand_murmur3_hash_64(uint64_t h)
{
    h ^= h >> 33uLL;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33uLL;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33uLL;
    return h;
}

FUNCTION double
rand_f64(double lowerBound, double upperBound)
{
    assert(upperBound - lowerBound > 0);
    
    union {
        struct { uint32_t lo, hi; };
        uint64_t packed;
    };
    lo = rand_32_bits();
    hi = rand_32_bits();
    double normalized = normalize_64_bits_to_f64(packed);
    normalized *= (upperBound - lowerBound);
    return normalized + lowerBound;
}

FUNCTION float
rand_f32(float lowerBound, float upperBound)
{
    assert(upperBound - lowerBound > 0);
    
    auto r = rand_32_bits();
    float normalized = normalize_32_bits_to_f32(r);
    normalized *= (upperBound - lowerBound);
    return normalized + lowerBound;
}

FUNCTION int32_t
rand_i32(int32_t lowerBound, int32_t upperBound)
{
    assert(upperBound - lowerBound > 0);
    
    auto range = upperBound - lowerBound + 1;
    auto ret = rand_f32();
    return (int32_t)((ret * range) + lowerBound);
}

#if defined(DRAC_RANDOM_PCG)

#define PCG_RAND_MIN 0
#define PCG_RAND_MAX 0xFFFFFFFFuLL
#define PCG_RAND_RANGE (PCG_RAND_MAX - PCG_RAND_MIN + 0x1i64)

struct pcg_plant
{
    uint64_t state;
    uint64_t sequence;
};
GLOBAL pcg_plant PcgPlant;
GLOBAL bool PcgInitialized = false;

FUNCTION uint32_t
pcg_next_32_bits()
{
    uint64_t oldstate = PcgPlant.state;
    PcgPlant.state = oldstate * 0x5851f42d4c957f2duLL + PcgPlant.sequence;
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18uLL) ^ oldstate) >> 27uLL );
    uint32_t rot = (uint32_t)(oldstate >> 59uLL);
    return (xorshifted >> rot) | (xorshifted << ((-(int32_t)rot) & 31) );
}

FUNCTION void
pcg_seed_rand(uint64_t seed)
{
    uint64_t value = (seed << 1uLL) | 1uLL;
    value = gb_rand_murmur3_hash_64(value);
    PcgPlant.state = 0;
    PcgPlant.sequence = (value << 1uLL) | 1uLL;
    pcg_next_32_bits();
    PcgPlant.state += gb_rand_murmur3_hash_64(value);
    pcg_next_32_bits();
}

FUNCTION uint32_t
rand_32_bits()
{
    if (!PcgInitialized) seed_rand(1150609);
    return pcg_next_32_bits();
}

FUNCTION void    
seed_rand(uint64_t seed)
{
    PcgInitialized = true;
    pcg_seed_rand(seed);
}


#undef PCG_RAND_MIN 
#undef PCG_RAND_MAX 
#undef PCG_F64_RAND_MAX 
#undef PCG_RAND_RANGE
#undef PCG_SEED_DEFAULT

/* DRAC_RANDOM_PCG */

#elif 0
#endif /* random alg selector */


#endif /* DRAC_RANDOM_CPP */
