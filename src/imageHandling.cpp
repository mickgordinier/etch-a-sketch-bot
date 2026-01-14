#include <vector>
#include <cstdint>
#include <iostream>

#include "../include/imageHandling.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb/stb_image_write.h"


int
read_bmp_image(
    const char * input_filepath, 
    std::vector<uint8_t> &binary_image,
    int &height, int &width
) {
    int channels;

    unsigned char* data = stbi_load(input_filepath, &width, &height, &channels, 1);
    if (!data) {
        std::cerr << "Failed to load image\n";
        return 1;
    }

    binary_image.resize(height * width);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            binary_image[y * width + x] = data[y * width + x] > 128 ? 0 : 1;
        }
    }

    stbi_image_free(data);
    return 0;
}


int
write_bmp_image (
    const char * output_image_filepath, 
    const std::vector<uint8_t> &binary_image,
    int height, int width
) {
    // Writing to output filepath
    if (!stbi_write_bmp(output_image_filepath, width, height, 1, binary_image.data())) {
        std::cerr << "Failed to write BMP\n";
        return 1;
    }   
    return 0;
}