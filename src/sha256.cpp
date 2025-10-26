#include "sha256.h"

std::string sha256_file(const std::string& filepath) {
    FILE* f = fopen(filepath.c_str(), "rb");

    if(!f) throw std::runtime_error("Cannot open file: " + filepath);

}