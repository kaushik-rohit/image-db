// test_sha256.cpp
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// ---- Your SHA-256 API (adjust the header include / declarations as needed)
struct Sha256Ctx {
    uint8_t  data[64];
    uint32_t h[8];
    uint32_t datalen;
    uint64_t bitlen;
};

void sha256_init(Sha256Ctx& ctx);
void sha256_update(Sha256Ctx& ctx, const uint8_t* data, size_t len);
void sha256_final(Sha256Ctx& ctx, uint8_t out[32]);
std::string sha256_hex(const uint8_t digest[32]);
std::string sha256_file(const std::string& filepath);

// Convenience: hash an in-memory buffer and return hex string
static std::string sha256_mem_hex(const void* data, size_t len) {
    Sha256Ctx ctx;
    sha256_init(ctx);
    sha256_update(ctx, static_cast<const uint8_t*>(data), len);
    uint8_t dig[32];
    sha256_final(ctx, dig);
    return sha256_hex(dig);
}

// Convenience: hash a std::string and return hex string
static std::string sha256_str_hex(const std::string& s) {
    return sha256_mem_hex(s.data(), s.size());
}

// Write bytes to a temp file and return its path
static std::string write_temp_file(const std::string& name_hint, const std::vector<uint8_t>& bytes) {
    // Keep it simple: write into current directory
    std::string path = name_hint;
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open temp file for writing: " + path);
    }
    ofs.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    ofs.close();
    return path;
}

// Remove file (best-effort)
static void remove_file(const std::string& path) {
    std::remove(path.c_str());
}

// Assert helper with message
static void expect_eq(const std::string& got, const std::string& want, const char* label) {
    if (got != want) {
        std::cerr << "[FAIL] " << label << "\n"
                  << "  got : " << got  << "\n"
                  << "  want: " << want << "\n";
        std::exit(1);
    } else {
        std::cout << "[PASS] " << label << "\n";
    }
}

int main() {
    try {
        // --- NIST / well-known test vectors ---

        // 1) Empty string
        // Source: FIPS 180-4
        const std::string tv_empty =
            "e3b0c44298fc1c149afbf4c8996fb924"
            "27ae41e4649b934ca495991b7852b855";
        expect_eq(sha256_str_hex(""), tv_empty, "empty string");

        // 2) "abc"
        const std::string tv_abc =
            "ba7816bf8f01cfea414140de5dae2223"
            "b00361a396177a9cb410ff61f20015ad";
        expect_eq(sha256_str_hex("abc"), tv_abc, "\"abc\"");

        // 3) Long known vector
        // "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
        const std::string long_msg =
            "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        const std::string tv_long =
            "248d6a61d20638b8e5c026930c3e6039"
            "a33ce45964ff2167f6ecedd419db06c1";
        expect_eq(sha256_str_hex(long_msg), tv_long, "NIST long message");

        // 4) One million 'a' characters
        // Known digest: cdc76e5c9914fb9281a1c7e284d73e67
        //               f1809a48a497200e046d39ccc7112cd0
        {
            const size_t N = 1000000;
            std::vector<uint8_t> million_a(N, static_cast<uint8_t>('a'));
            const std::string tv_million_a =
                "cdc76e5c9914fb9281a1c7e284d73e67"
                "f1809a48a497200e046d39ccc7112cd0";
            expect_eq(sha256_mem_hex(million_a.data(), million_a.size()), tv_million_a, "1,000,000 x 'a'");
        }

        // --- File-based tests ---

        // 5) Empty file
        {
            std::vector<uint8_t> bytes; // empty
            std::string path = write_temp_file("tmp_empty.bin", bytes);
            const std::string want = tv_empty;
            const std::string got  = sha256_file(path);
            expect_eq(got, want, "file: empty");
            remove_file(path);
        }

        // 6) Small file "abc"
        {
            std::vector<uint8_t> bytes = {'a','b','c'};
            std::string path = write_temp_file("tmp_abc.bin", bytes);
            const std::string want = tv_abc;
            const std::string got  = sha256_file(path);
            expect_eq(got, want, "file: \"abc\"");
            remove_file(path);
        }

        // 7) File with non-aligned size (e.g., 100 KiB + 13 bytes)
        {
            std::vector<uint8_t> bytes((100 << 10) + 13);
            for (size_t i = 0; i < bytes.size(); ++i) {
                bytes[i] = static_cast<uint8_t>(i & 0xFF);
            }
            std::string path = write_temp_file("tmp_misc.bin", bytes);

            // Cross-check: hash memory and file should match
            const std::string want = sha256_mem_hex(bytes.data(), bytes.size());
            const std::string got  = sha256_file(path);
            expect_eq(got, want, "file vs memory equivalence (unaligned)");
            remove_file(path);
        }

        std::cout << "\nAll tests passed ✅\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Exception: " << ex.what() << "\n";
        return 2;
    }
}
