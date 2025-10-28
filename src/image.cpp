#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION 
#define STBIW_SPRINTF snprintf
#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"
#include <string>
#include <image.h>
#include <iostream>
#include <algorithm> 

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

    stbir_pixel_layout layout = (c == 4) ? STBIR_RGBA : STBIR_RGB;

    double scale = 256.0 / std::max(w, h);
    int target_w = std::max(1, int(w * scale));
    int target_h = std::max(1, int(h * scale));

    unsigned char* ok = stbir_resize_uint8_srgb(
        data, w, h, 0,                 // input
        resized, target_w, target_h, 0,// output (compute target_w/h to preserve aspect)
        layout
    );

    if (!ok) {
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