#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <filesystem>

#include "../include/imageHandling.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb/stb_image_write.h"

namespace fs = std::filesystem;

#define MOTOR_LEFT   0  // Left  motor counterclockwise
#define MOTOR_RIGHT  1  // Left  motor clockwise
#define MOTOR_DOWN   2  // Right motor counterclockwise
#define MOTOR_UP     3  // Right motor clockwise

const char* const DIRECTIONS[] = {
    "LEFT",
    "RIGHT",
    "DOWN",
    "UP"
};


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
    const std::string &output_image_filepath, 
    std::vector<uint8_t> binary_image,
    int height, int width
) {
    // Outputting final image to bmp file
    for (int i = 0; i < height * width; ++i) {
        binary_image[i] = (binary_image[i]) ? 0 : 255;
    }

    // Writing to output filepath
    if (!stbi_write_bmp(output_image_filepath.c_str(), width, height, 1, binary_image.data())) {
        std::cerr << "Failed to write BMP\n";
        return 1;
    }   
    return 0;
}


void inline
write_step (
    std::ofstream &humanFile,
    std::ofstream &binaryFile,
    uint8_t direction, uint8_t stepCount
) {
    humanFile << DIRECTIONS[direction] << " " << std::to_string(stepCount) << "\n";
    binaryFile.put(static_cast<char>((direction << 6) | stepCount));
}


void
write_instructions(
    const std::vector<uint32_t> &steps,
    const std::string &output_binary_steps_filepath,
    const std::string &output_human_steps_filepath
) {
    // Creating both human readable file

    std::ofstream humanFile(output_human_steps_filepath);
    std::ofstream binaryFile(output_binary_steps_filepath);

    if (!humanFile.is_open()) {
        std::cerr << "Unable to open human text file\n";
        return;
    }

    if (!binaryFile.is_open()) {
        std::cerr << "Unable to open binary file";
        return;
    }

    uint8_t prevDirection = 0;
    uint8_t currDirection = 0; 
    uint8_t stepCount = 0;

    for (size_t i = 1; i < steps.size(); ++i) {
        
        // Determine current step direction
        if (abs((int)steps[i] - (int)steps[i-1]) > 1) {
            currDirection = (steps[i] > steps[i-1]) ? MOTOR_DOWN : MOTOR_UP;
        } else {
            currDirection = (steps[i] > steps[i-1]) ? MOTOR_RIGHT : MOTOR_LEFT;
        }

        if (currDirection == prevDirection) {
            ++stepCount;
            if (stepCount == 64) {
                write_step(humanFile, binaryFile, prevDirection, 63);
                stepCount = 1;
            }
        } else {
            if (stepCount > 0) {
                write_step(humanFile, binaryFile, prevDirection, stepCount);
            }
            prevDirection = currDirection;
            stepCount = 1;
        }
    }

    write_step(humanFile, binaryFile, prevDirection, stepCount);
    
    humanFile << "END OF INSTRUCTIONS\n";
    binaryFile.put(static_cast<char>(0));

    humanFile.close();
    binaryFile.close();
}