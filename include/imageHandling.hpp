#include <vector>
#include <cstdint>

int
read_bmp_image(
    const char * input_filepath, 
    std::vector<uint8_t> &binary_image,
    int &height, int &width
);

int
write_bmp_image (
    const char * output_image_filepath, 
    std::vector<uint8_t> binary_image,
    int width, int height
);