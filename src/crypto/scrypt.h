#ifndef SCRYPT_H
#define SCRYPT_H
#include <stdlib.h>
#include <stdint.h>

static const int SCRYPT_SCRATCHPAD_SIZE = 131072 + 63;

  
//void crypto_scrypt(const unsigned char *ucInput, unsigned long nInputSize, const unsigned char *ucSalt,
//        unsigned long nSaltSize, unsigned long N, unsigned int p, unsigned int r, unsigned char *ucOutput, unsigned long nOutputSize);
bool scrypt_sp_generic(const char *ucInput, unsigned long nInputSize, const unsigned char *ucSalt,
        unsigned long nSaltSize, unsigned long N, unsigned int p, unsigned int r, unsigned char *ucOutput, unsigned long nOutputSize);

//#if defined(USE_SSE2)
//#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64) || (defined(MAC_OSX) && defined(__i386__))
//#define USE_SSE2_ALWAYS 1
//#define scrypt_1024_1_1_256_sp(input, output, scratchpad) scrypt_1024_1_1_256_sp_sse2((input), (output), (scratchpad))
//#else
//#define scrypt_1024_1_1_256_sp(input, output, scratchpad) scrypt_1024_1_1_256_sp_detected((input), (output), (scratchpad))
//#endif
//
//void scrypt_detect_sse2();
//void scrypt_1024_1_1_256_sp_sse2(const char *input, char *output, char *scratchpad);
//extern void (*scrypt_1024_1_1_256_sp_detected)(const char *input, char *output, char *scratchpad);
//#else
#define scrypt_1024_1_1_256_sp(input, output, scratchpad) scrypt_1024_1_1_256_sp_generic((input), (output), (scratchpad))
//#endif

void
PBKDF2_SHA256(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt,
    size_t saltlen, uint64_t c, uint8_t *buf, size_t dkLen);

static inline uint32_t le32dec(const void *pp)
{
        const uint8_t *p = (uint8_t const *)pp;
        return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
            ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

static inline void le32enc(void *pp, uint32_t x)
{
        uint8_t *p = (uint8_t *)pp;
        p[0] = x & 0xff;
        p[1] = (x >> 8) & 0xff;
        p[2] = (x >> 16) & 0xff;
        p[3] = (x >> 24) & 0xff;
}
//static void smix(uint8_t * B, size_t r, uint64_t N, uint32_t * V, uint32_t * XY);
//static void blkcpy(uint32_t * dest, uint32_t * src, size_t len);
//static void blkxor(uint32_t * dest, uint32_t * src, size_t len);
//static void salsa20_8(uint32_t B[16]);
//static void blockmix_salsa8(uint32_t * Bin, uint32_t * Bout, uint32_t * X, size_t r);
//static uint64_t integerify(uint32_t * B, size_t r);
#endif
