#include<string>

struct ImgDims {
    int width;
    int height;
    int channels;
};

bool read_dims(const std::string& filepath, ImgDims* out);

bool make_thumbnail_256(const std::string& src_path, const std::string& dst_path);
