#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <string>
#include <image.h>
#include <iostream>

bool read_dims(const std::string& filepath, ImgDims* out) {
    int w, h, c;
    if (!stbi_info(filepath.c_str(), &w, &h, &c)) {
        std::cerr << "Failed to read image info: " << filepath << std::endl;
        return false;
    }
    out->width = w;
    out->height = h;
    out->channels = c;
    return true;
}

bool make_thumbnail_256(const std::string& input_path, const std::string& output_path) {
    int w, h, c;
    unsigned char* data = stbi_load(input_path.c_str(), &w, &h, &c, 0);
    if (!data) {
        std::cerr << "Failed to load image: " << input_path << std::endl;
        return false;
    }

    const int target_size = 256;
    unsigned char* resized = new unsigned char[target_size * target_size * c];

    int alpha_channel = (c == 4) ? 3 : STBIR_ALPHA_CHANNEL_NONE;

    int success = stbir_resize_uint8_srgb(
        data, w, h, 0,                    // input
        resized, target_size, target_size, 0, // output
        c,                                // number of channels
        alpha_channel,                    // alpha channel index or -1
        0                                 // flags (0 = default)
    );

    if (!success) {
        std::cerr << "Resize failed.\n";
        stbi_image_free(data);
        delete[] resized;
        return false;
    }

    // Save as PNG
    if (!stbi_write_png(output_path.c_str(), target_size, target_size, c, resized, target_size * c)) {
        std::cerr << "Failed to write thumbnail.\n";
        stbi_image_free(data);
        delete[] resized;
        return false;
    }

    stbi_image_free(data);
    delete[] resized;
    return true;
}