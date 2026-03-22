#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

#include "../include/EtchASketch.h"
#include "../include/debugHelp.hpp"

const int CLUSTER_CUTOFF = 10000;

int 
main(int argc, char **argv) 
{
    ScopedTimer program_timer("TOTAL PROGRAM");
    
    // {executable}.out {height} {width} {output_image_filepath} {output_steps_filepath} {output_steps_binary_filepath}
    if (argc == 6) {

        int const          height                       = std::stoi(argv[1]);
        int const          width                        = std::stoi(argv[2]);
        std::string const &output_image_filepath        = argv[3];
        std::string const &output_steps_filepath        = argv[4];
        std::string const &output_steps_binary_filepath = argv[5];

        EtchASketch etchASketch(
            height, 
            width, 
            CLUSTER_CUTOFF, 
            output_image_filepath, 
            output_steps_filepath, 
            output_steps_binary_filepath
        );

    } 
    // {executable}.out {input_filepath} {output_filepath} {output_steps_filepath} {output_steps_binary_filepath}
    else if (argc == 5) {

        std::string const &input_image_filepath         = argv[1];
        std::string const &output_image_filepath        = argv[2];
        std::string const &output_steps_filepath        = argv[3];
        std::string const &output_steps_binary_filepath = argv[4];

        EtchASketch etchASketch(
            input_image_filepath,
            CLUSTER_CUTOFF, 
            output_image_filepath, 
            output_steps_filepath, 
            output_steps_binary_filepath
        );

    } 
    // {executable}.out {output_filepath} {output_steps_filepath} {output_steps_binary_filepath}
    else if (argc == 4) {

        std::string const &output_image_filepath        = argv[1];
        std::string const &output_steps_filepath        = argv[2];
        std::string const &output_steps_binary_filepath = argv[3];

        const int height = 7;
        const int width  = 7;

        const std::vector<uint8_t> input_image = {
            1,   0,   0,   0,   0,   1,   0, 
            0,   1,   1,   1,   0,   0,   0, 
            0,   1,   0,   0,   0,   0,   0, 
            0,   0,   0,   0,   0,   0,   0, 
            0,   0,   0,   0,   0,   0,   0, 
            1,   1,   0,   1,   1,   0,   0, 
            0,   1,   0,   0,   0,   0,   0
        };

        EtchASketch etchASketch(
            input_image,
            height, 
            width,
            CLUSTER_CUTOFF, 
            output_image_filepath, 
            output_steps_filepath, 
            output_steps_binary_filepath
        );
        
    } else {
        std::cout << "Wrong arguments provided\n";
        return 1;
    }

    return 0;
}