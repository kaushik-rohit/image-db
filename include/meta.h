#include<string>

struct ImageMeta {
    std::string image_id, sha256, mime;
    uint32_t width, height;
    uint64_t bytes, created_unix;
};

std::string meta_to_json(const ImageMeta& m);