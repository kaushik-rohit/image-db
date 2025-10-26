#pragma once
#include <string>

// Computes the SHA-256 hash of the file at `filepath`.
// Returns a 64-character lowercase hex string.
// Throws std::runtime_error on failure.
std::string sha256_file(const std::string& filepath);