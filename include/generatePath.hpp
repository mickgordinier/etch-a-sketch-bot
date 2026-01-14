#pragma once

#include <vector>
#include <string>
#include <cstdint>

const int adjacentNodes[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

std::vector<uint8_t>
connectAllComponents(
    const std::vector<uint8_t> &original_binary_image,
    int height, int width
);

std::vector<uint32_t>
generatePath (
    const std::vector<uint8_t> &final_binary_image,
    int cutoffClustering,
    int height, int width
);