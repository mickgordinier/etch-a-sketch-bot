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
    
    // {executable}.out {height} {width} {output_foldername}
    if (argc == 4) {

        int const          height                       = std::stoi(argv[1]);
        int const          width                        = std::stoi(argv[2]);
        std::string const &output_foldername            = argv[3];

        EtchASketch etchASketch(
            height, 
            width, 
            CLUSTER_CUTOFF, 
            output_foldername
        );

    } 
    // {executable}.out {input_filepath} {output_foldername}
    else if (argc == 3) {

        std::string const &input_image_filepath         = argv[1];
        std::string const &output_foldername            = argv[2];

        EtchASketch etchASketch(
            input_image_filepath,
            CLUSTER_CUTOFF, 
            output_foldername
        );

    } 
    // {executable}.out {output_foldername}
    else if (argc == 2) {

        std::string const &output_foldername            = argv[1];

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
            output_foldername
        );
        
    } else {
        std::cout << "Wrong arguments provided\n";
        return 1;
    }

    return 0;
}