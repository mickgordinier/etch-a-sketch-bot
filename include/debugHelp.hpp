#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>

extern uint8_t EXTRA_PRINT;
extern uint8_t BASIC_PRINT;

struct ScopedTimer {
    std::string name;
    std::chrono::high_resolution_clock::time_point start;

    ScopedTimer(const std::string &task_name)
    : name(task_name),
    start(std::chrono::high_resolution_clock::now()) {
        std::cout << "\n[START] " << name << "\n";
    }
    
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "[END]   " << name
                  << " | Time: " << duration.count() << " ms\n\n";
    }
};


template <typename T>
void
print2DVector(
    const std::string &image_name,
    const std::vector<T> &vec,
    int height, int width
) {
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";
    std::cout << image_name << "\n";
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            std::cout << std::setw(3) << static_cast<int>(vec[row*width + col]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "-----------------------------------------------------------------------------------------------------------\n\n";
}


template <typename T>
void
print2DVector(
    const std::string &image_name,
    const std::vector<std::vector<T>> &vec
) {
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";
    std::cout << image_name << "\n";
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";

    for (size_t row = 0; row < vec.size(); ++row) {
        for (size_t col = 0; col < vec[0].size(); ++col) {
            std::cout << std::setw(3) << static_cast<int>(vec[row][col]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "-----------------------------------------------------------------------------------------------------------\n\n";
}