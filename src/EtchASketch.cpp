#include <stdexcept>
#include <random>
#include <vector>

#include "../include/EtchASketch.h"
#include "../include/imageHandling.hpp"
#include "../include/debugHelp.hpp"
#include "../include/generatePath.hpp"

EtchASketch::EtchASketch (
    std::string const &input_image_filepath_,
    int                cluster_cutoff_,
    std::string const &output_image_filepath_,
    std::string const &output_steps_filepath_,
    std::string const &output_steps_binary_filepath_,
    bool               place_border_around_image_
) {
    if (read_bmp_image(input_image_filepath_.c_str(), binary_image, height, width)) {
        throw std::runtime_error("EtchASketch constructor: read_bmp_image() failure");
    }

    if ((height == 0) || (width == 0)) {
        throw std::runtime_error("EtchASketch constructor: Invalid image dimensions");
    }

    place_border_around_image = place_border_around_image_;
    cluster_cutoff = cluster_cutoff_;

    initOutputFilepaths(output_image_filepath_, output_steps_filepath_, output_steps_binary_filepath_);
    performFullProcess();
}

EtchASketch::EtchASketch (
    int                height_, 
    int                width_, 
    int                cluster_cutoff_,
    std::string const &output_image_filepath_,
    std::string const &output_steps_filepath_,
    std::string const &output_steps_binary_filepath_,
    bool               place_border_around_image_
) :
    height(height_),
    width(width_)
{
    if ((height == 0) || (width == 0)) {
        throw std::runtime_error("EtchASketch constructor: Invalid image dimensions");
    }

    binary_image.resize(height * width);
    generateRandomImage();

    place_border_around_image = place_border_around_image_;
    cluster_cutoff = cluster_cutoff_;

    initOutputFilepaths(output_image_filepath_, output_steps_filepath_, output_steps_binary_filepath_);
    performFullProcess();
}

EtchASketch::EtchASketch(
    std::vector<uint8_t> const &input_image_,
    int                         height_, 
    int                         width_,
    int                         cluster_cutoff_,
    std::string const          &output_image_filepath_,
    std::string const          &output_steps_filepath_,
    std::string const          &output_steps_binary_filepath_,
    bool                        place_border_around_image_
) :
    height(height_),
    width(width_),
    binary_image(input_image_)
{
    if ((height == 0) || (width == 0)) {
        throw std::runtime_error("EtchASketch constructor: Invalid image dimensions");
    }

    place_border_around_image = place_border_around_image_;
    cluster_cutoff = cluster_cutoff_;

    initOutputFilepaths(output_image_filepath_, output_steps_filepath_, output_steps_binary_filepath_);
    performFullProcess();
}

void
EtchASketch::generateRandomImage() {
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
EtchASketch::initOutputFilepaths(
    std::string const          &output_image_filepath_,
    std::string const          &output_steps_filepath_,
    std::string const          &output_steps_binary_filepath_
) {
    output_image_filepath = output_image_filepath_;
    output_steps_filepath = output_steps_filepath_;
    output_steps_binary_filepath = output_steps_binary_filepath_;
}

void
EtchASketch::placeBorderAroundImage() {
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

void
EtchASketch::performFullProcess() {

    if (place_border_around_image) {
        placeBorderAroundImage();
    } else {
        binary_image[0] = 1;
    }

    #ifdef BASIC_PRINT
        print2DVector("ORIGINAL IMAGE", binary_image, height, width);
    #endif
    
    std::vector<uint8_t> final_binary_image = connectAllComponents(binary_image, height, width);

    #ifdef BASIC_PRINT
        print2DVector("FINAL IMAGE", final_binary_image, height, width);
    #endif

    if (write_bmp_image(("output/" + output_image_filepath).c_str(), final_binary_image, height, width)) {
        throw std::runtime_error("EtchASketch performFullProcess: Write bmp image failure");
    }

    // Finding reasonable shortest path to produce image
    // Solving for Chinese Postman Problem
    std::vector<uint32_t> steps = generatePath(final_binary_image, cluster_cutoff, height, width);

    std::cout << "Number of steps total: " << steps.size() << "\n\n";

    write_instructions(steps, output_steps_binary_filepath, output_steps_filepath);
}