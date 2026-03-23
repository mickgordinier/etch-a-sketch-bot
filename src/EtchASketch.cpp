#include <stdexcept>
#include <random>
#include <vector>
#include <filesystem>

#include "../include/EtchASketch.h"
#include "../include/imageHandling.hpp"
#include "../include/debugHelp.hpp"
#include "../include/generatePath.hpp"

namespace fs = std::filesystem;

EtchASketch::EtchASketch (
    std::string const &input_image_filepath_,
    int                cluster_cutoff_,
    std::string const &output_foldername_,
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
    output_foldername = output_foldername_;

    performFullProcess();
}

EtchASketch::EtchASketch (
    int                height_, 
    int                width_, 
    int                cluster_cutoff_,
    std::string const &output_foldername_,
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
    output_foldername = output_foldername_;

    performFullProcess();
}

EtchASketch::EtchASketch(
    std::vector<uint8_t> const &input_image_,
    int                         height_, 
    int                         width_,
    int                         cluster_cutoff_,
    std::string const          &output_foldername_,
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
    output_foldername = output_foldername_;

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

    // 1. Write original image to output folder
    const std::string output_dir = "output/" + output_foldername + "/";
    const std::string original_image_filename = output_dir + "original_image.bmp";

    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir); // creates parents if needed
    }

    if (write_bmp_image(original_image_filename, binary_image, height, width)) {
        throw std::runtime_error("EtchASketch performFullProcess: Write original bmp image failure");
    }

    // 2. Place starting pixel or border around image
    if (place_border_around_image) {
        placeBorderAroundImage();
    } else {
        binary_image[0] = 1;
    }

    #ifdef BASIC_PRINT
        print2DVector("ORIGINAL IMAGE", binary_image, height, width);
    #endif
    
    // 3. Connect all isolated pixel components
    //    Ensuring every pixel has an adjacent neighbor (up, down, left, right)
    std::vector<uint8_t> final_binary_image = connectAllComponents(binary_image, height, width);

    #ifdef BASIC_PRINT
        print2DVector("FINAL IMAGE", final_binary_image, height, width);
    #endif

    // 4. Write final binary image to output folder
    //    Once the components are connected, no other modification of image is required
    const std::string final_image_filename = output_dir + "fully_connected_image.bmp";

    if (write_bmp_image(final_image_filename, final_binary_image, height, width)) {
        throw std::runtime_error("EtchASketch performFullProcess: Write final bmp image failure");
    }

    // 5. Finding reasonable shortest steps to produce image
    //    Solving for Chinese Postman Problem
    //    Path generated will connect every single existing edge on binary image "graph"
    std::vector<uint32_t> steps = generatePath(final_binary_image, cluster_cutoff, height, width);

    std::cout << "Number of steps total: " << steps.size() << "\n\n";

    const std::string final_steps_filename = output_dir + "human_steps.txt";
    const std::string final_steps_binary_filename = output_dir + "binary_steps.bin";

    // 6. Write final step instructions as both human readable and condensed binary
    //    Binary file is what will be sent to the embedded system for processing
    write_instructions(steps, final_steps_binary_filename, final_steps_filename);
}