#include "sha256.h"
#include<iostream>
#include<array>

/***
 * SHA-256 digest are eight 32-bit words.
 */
struct Sha256Ctx {
    std::array<uint32_t, 8> h;
    std::array<uint8_t, 64>  data; 
    uint64_t bitlen;
    uint32_t datalen; 
};

// --- helpers ---
static inline uint32_t ROTR(uint32_t x, unsigned n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t SHR (uint32_t x, unsigned n) { return x >> n; }

static inline uint32_t Ch (uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }

static inline uint32_t BSIG0(uint32_t x) { return ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22); }
static inline uint32_t BSIG1(uint32_t x) { return ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25); }
static inline uint32_t SSIG0(uint32_t x) { return ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3); }
static inline uint32_t SSIG1(uint32_t x) { return ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10); }

// 64 SHA-256 constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes)
static constexpr uint32_t K[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

void sha256_init(Sha256Ctx& ctx) {
    ctx.h[0] = 0x6a09e667;
    ctx.h[1] = 0xbb67ae85;
    ctx.h[2] = 0x3c6ef372;
    ctx.h[3] = 0xa54ff53a;
    ctx.h[4] = 0x510e527f;
    ctx.h[5] = 0x9b05688c;
    ctx.h[6] = 0x1f83d9ab;
    ctx.h[7] = 0x5be0cd19;

    ctx.datalen = 0;    // how many bytes currently buffered (0–63)
    ctx.bitlen = 0;     // total number of bits processed so far
}

void sha256_transform(Sha256Ctx& ctx, const std::array<uint8_t, 64> block) {
    uint32_t m[64];

    // 1) Prepare message schedule 'm'
    //    Load first 16 words as big-endian
    for (int i = 0; i < 16; ++i) {
        m[i] =
            (static_cast<uint32_t>(block[i * 4 + 0]) << 24) |
            (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
            (static_cast<uint32_t>(block[i * 4 + 2]) <<  8) |
            (static_cast<uint32_t>(block[i * 4 + 3]) <<  0);
    }
    //    Extend to 64 words
    for (int i = 16; i < 64; ++i) {
        m[i] = SSIG1(m[i - 2]) + m[i - 7] + SSIG0(m[i - 15]) + m[i - 16];
    }

    // 2) Initialize working variables with current hash state
    uint32_t a = ctx.h[0];
    uint32_t b = ctx.h[1];
    uint32_t c = ctx.h[2];
    uint32_t d = ctx.h[3];
    uint32_t e = ctx.h[4];
    uint32_t f = ctx.h[5];
    uint32_t g = ctx.h[6];
    uint32_t h = ctx.h[7];

    // 3) Main compression loop
    for (int i = 0; i < 64; ++i) {
        uint32_t T1 = h + BSIG1(e) + Ch(e, f, g) + K[i] + m[i];
        uint32_t T2 = BSIG0(a) + Maj(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    // 4) Add the compressed chunk to the current hash value
    ctx.h[0] += a;
    ctx.h[1] += b;
    ctx.h[2] += c;
    ctx.h[3] += d;
    ctx.h[4] += e;
    ctx.h[5] += f;
    ctx.h[6] += g;
    ctx.h[7] += h;
}

void sha256_update(Sha256Ctx& ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx.data[ctx.datalen++] = data[i];

        // Process a full 512-bit block
        if (ctx.datalen == 64) {
            sha256_transform(ctx, ctx.data);
            ctx.bitlen += 512;   // count processed bits
            ctx.datalen = 0;
        }
    }
}

void sha256_final(Sha256Ctx& ctx, uint8_t out[32]){
    // Total message length in bits BEFORE padding
    uint64_t total_bits = ctx.bitlen + static_cast<uint64_t>(ctx.datalen) * 8ULL;

    // Append the '1' bit (0x80 byte)
    ctx.data[ctx.datalen++] = 0x80;

    // Pad with zeros until we have 56 bytes (so we can append 8-byte length)
    if (ctx.datalen > 56) {
        std::memset(ctx.data.data() + ctx.datalen, 0, 64 - ctx.datalen);
        sha256_transform(ctx, ctx.data);
        ctx.bitlen += 512;
        ctx.datalen = 0;
    }

    std::memset(ctx.data.data() + ctx.datalen, 0, 56 - ctx.datalen);
    ctx.datalen = 56;

    // Append 64-bit big-endian length
    for (int i = 7; i >= 0; --i) {
        ctx.data[ctx.datalen++] = static_cast<uint8_t>(total_bits >> (i * 8));
    }

    // Final block
    sha256_transform(ctx, ctx.data);
    // ctx.bitlen += 512; // not needed anymore, hashing is complete

    // Output digest as big-endian bytes from h[0..7]
    for (int i = 0; i < 8; ++i) {
        uint32_t w = ctx.h[i];
        out[i * 4 + 0] = static_cast<uint8_t>((w >> 24) & 0xFF);
        out[i * 4 + 1] = static_cast<uint8_t>((w >> 16) & 0xFF);
        out[i * 4 + 2] = static_cast<uint8_t>((w >>  8) & 0xFF);
        out[i * 4 + 3] = static_cast<uint8_t>((w >>  0) & 0xFF);
    }
}

std::string sha256_hex(const uint8_t digest[32]) {
    static const char* kHex = "0123456789abcdef";
    std::string s;
    s.resize(64);
    for (int i = 0; i < 32; ++i) {
        uint8_t b = digest[i];
        s[2*i]     = kHex[(b >> 4) & 0xF];
        s[2*i + 1] = kHex[b & 0xF];
    }
    return s;
}

std::string sha256_file(const std::string& filepath) {
    Sha256Ctx ctx;
    sha256_init(ctx);
    std::array<uint8_t, 1<<20> buf;

    FILE* f = fopen(filepath.c_str(), "rb");

    if(!f) throw std::runtime_error("Cannot open file: " + filepath);

    for(;;){
        size_t n = fread(buf.data(), 1, buf.size(), f);
        if(n) sha256_update(ctx, buf.data(), n);
        if(n < buf.size()) {
            if(ferror(f)) throw std::runtime_error("error reading file");
            break;
        }
    }
    fclose(f);
    uint8_t dig[32];
    sha256_final(ctx, dig);
    return sha256_hex(dig);
}