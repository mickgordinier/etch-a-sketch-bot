#pragma once

#include <string>
#include <vector>

class EtchASketch {
public:

    // Cosntructor to pass in monochorme bitmap image filepath
    EtchASketch (
        std::string const &input_image_filepath_,
        int                cluster_cutoff_,
        std::string const &output_foldername_,
        bool               place_border_around_image_ = true
    );

    // Constructor to pass in height/width for random monochrome image generation
    EtchASketch (
        int                height, 
        int                width, 
        int                cluster_cutoff_,
        std::string const &output_foldername_,
        bool               place_border_around_image_ = true
    );

    // Constructor to pass in 2D vector monochrome image
    EtchASketch(
        std::vector<uint8_t> const &input_image_,
        int                         height_, 
        int                         width_,
        int                         cluster_cutoff_,
        std::string const          &output_foldername_,
        bool                        place_border_around_image_ = true
    );


private:
    void generateRandomImage();
    void performFullProcess();
    void placeBorderAroundImage();

    int                  height, width;
    std::vector<uint8_t> binary_image;
    bool                 place_border_around_image;
    int                  cluster_cutoff;

    std::string          output_foldername;
};