#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <string>
#include <cstdint>

#include "../include/debugHelp.hpp"
#include "../include/generatePath.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb/stb_image_write.h"

uint8_t EXTRA_PRINT = 1;
uint8_t BASIC_PRINT = 1;


void
generateRandomImage(
    std::vector<uint8_t> &binary_image,
    int height, int width
) {
    ScopedTimer timer("Generate Random Image");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.2);  // density of 1's

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            binary_image[i*width + j] = dist(gen);
        }
    }
}


void
placeBorderAroundImage(
    std::vector<uint8_t> &binary_image,
    int height, int width
) {
    ScopedTimer timer("Place Border Around Image");

    for (int col = 0; col < width; ++col) {
        binary_image[col] = 1;
        binary_image[((height-1)*width) + col] = 1;
    }

    for (int row = 1; row < height-1; ++row) {
        binary_image[row*width] = 1;
        binary_image[(row*width) + (width-1)] = 1;
    }
}


int 
main(int argc, char **argv) 
{
    ScopedTimer program_timer("TOTAL PROGRAM");

    int height, width;
    std::string output_image, output_steps;

    std::vector<uint8_t> binary_image;

    // {executable}.out {height} {width} {output_image} {output_steps} {BASIC_PRINT} {EXTRA_PRINT}
    if (argc == 7) {

        height = std::stoi(argv[1]);
        width = std::stoi(argv[2]);
        output_image = argv[3];
        output_steps = argv[4];

        BASIC_PRINT = std::stoi(argv[5]);
        EXTRA_PRINT = std::stoi(argv[6]);
        
        binary_image.resize(height * width);

        // binary_image = {
        //     1,   0,   0,   0,   0,   1,   0, 
        //     0,   1,   1,   1,   0,   0,   0, 
        //     0,   1,   0,   0,   0,   0,   0, 
        //     0,   0,   0,   0,   0,   0,   0, 
        //     0,   0,   0,   0,   0,   0,   0, 
        //     1,   1,   0,   1,   1,   0,   0, 
        //     0,   1,   0,   0,   0,   0,   0
        // };
        
        generateRandomImage(binary_image, height, width);

    } 
    // {executable}.out {input_filepath} {output_filepath} {output_steps} {BASIC_PRINT} {EXTRA_PRINT}
    else if (argc == 6) {

        int channels;
        const char * input_filepath = argv[1];
        output_image = argv[2];
        output_steps = argv[3];
        BASIC_PRINT = std::stoi(argv[4]);
        EXTRA_PRINT = std::stoi(argv[5]);

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

    } else {
        std::cout << "Wrong arguments provided\n";
        return 1;
    }

    placeBorderAroundImage(binary_image, height, width);
    // binary_image[0] = 1;
    
    if (BASIC_PRINT) print2DVector("ORIGINAL IMAGE", binary_image, height, width);
    
    std::vector<uint8_t> final_binary_image = connectAllComponents(binary_image, height, width);

    if (BASIC_PRINT) print2DVector("FINAL IMAGE", final_binary_image, height, width);

    // Finding reasonable shortest path to produce image
    // Solving for Chinese Postman Problem
    performCPP(final_binary_image, output_steps, 10000, height, width);

    // Outputting final image to bmp file
    for (int i = 0; i < height * width; ++i) {
        final_binary_image[i] = (final_binary_image[i]) ? 0 : 255;
    }

    // Writing to output filepath
    if (!stbi_write_bmp(output_image.c_str(), width, height, 1, final_binary_image.data())) {
        std::cerr << "Failed to write BMP\n";
        return 1;
    }

    return 0;
}