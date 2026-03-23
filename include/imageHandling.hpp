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
    const std::string &output_image_filepath, 
    std::vector<uint8_t> binary_image,
    int width, int height
);

void
write_instructions(
    const std::vector<uint32_t> &steps,
    const std::string &output_binary_steps_filepath,
    const std::string &output_human_steps_filepath
);