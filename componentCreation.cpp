#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <string>
#include <iomanip>
#include <chrono>


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
    const std::vector<std::vector<T>> &vec
) {
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";
    std::cout << image_name << "\n";
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";
    
    int image_rows = vec.size();
    int image_cols = vec[0].size();

    for (int row = 0; row < image_rows; ++row) {
        for (int col = 0; col < image_cols; ++col) {
            std::cout << std::setw(3) << static_cast<int>(vec[row][col]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "-----------------------------------------------------------------------------------------------------------\n\n";

}


bool inline
checkNode(
    const std::vector<uint8_t> &binary_image, 
    const std::vector<uint32_t> &component_tracker,
    int nodeRow, int nodeCol,
    int image_rows, int image_cols
) {
    if ((nodeRow < 0) || (nodeCol < 0) || (nodeRow >= image_rows) || (nodeCol >= image_cols)) return false;;
    if (!binary_image[(nodeRow*image_cols) + nodeCol]) return false;
    if (component_tracker[(nodeRow*image_cols) + nodeCol]) return false;
    return true;
}


void
generateNewComponent(
    const std::vector<uint8_t> &binary_image, 
    std::vector<uint32_t> &component_tracker,
    int row, int col, int component_idx,
    int image_rows, int image_cols
) {
    if (!checkNode(binary_image, component_tracker, row, col, image_rows, image_cols)) return;

    component_tracker[(row*image_cols) + col] = component_idx;

    std::queue<std::pair<int, int>> nodesToCheck;
    nodesToCheck.push({row, col});

    while(!nodesToCheck.empty()) {
        std::pair<int, int> currNode = nodesToCheck.front();
        nodesToCheck.pop();

        int nodeRow = currNode.first;
        int nodeCol= currNode.second;

        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (!i && !j) continue;
                if (!checkNode(binary_image, component_tracker, nodeRow+i, nodeCol+j, image_rows, image_cols)) continue;
                component_tracker[(nodeRow+i)*image_cols + (nodeCol+j)] = component_idx;
                nodesToCheck.push({nodeRow+i, nodeCol+j});
            }
        }
    }
}

void 
getComponents(
    const std::vector<uint8_t> &binary_image, 
    std::vector<uint32_t> &component_tracker,
    int image_rows, int image_cols
) {
    ScopedTimer timer("Generate Component Labelling");

    int component_idx = 1;

    for (int row = 0; row < image_rows; ++row) {
        for (int col = 0; col < image_cols; ++col) {
            if (binary_image[(row*image_cols) + col] && !component_tracker[(row*image_cols) + col]) {
                generateNewComponent(binary_image, component_tracker, row, col, component_idx, image_rows, image_cols);
                ++component_idx;
            }
        }
    }
}


void
generateRandomImage(
    std::vector<uint8_t> &binary_image,
    int image_rows, int image_cols
) {
    ScopedTimer timer("Generate Random Image");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.2);  // density of 1's

    for (int i = 0; i < image_rows; ++i) {
        for (int j = 0; j < image_cols; ++j) {
            binary_image[i*image_cols + j] = dist(gen);
        }
    }
}


void
placeBorderAroundImage(
    std::vector<uint8_t> &binary_image,
    int image_rows, int image_cols
) {
    ScopedTimer timer("Place Border Around Image");

    for (int col = 0; col < image_cols; ++col) {
        binary_image[col] = 1;
        binary_image[((image_rows-1)*image_cols) + col] = 1;
    }

    for (int row = 1; row < image_rows-1; ++row) {
        binary_image[row*image_cols] = 1;
        binary_image[(row*image_cols) + (image_cols-1)] = 1;
    }
}


int 
main(int argc, char **argv) 
{
    ScopedTimer program_timer("TOTAL PROGRAM");

    if (argc != 3) {
        std::cout << "Required ./a.out {image_rows} {image_cols}\n";
        return 1;
    }

    int image_rows = std::stoi(argv[1]);
    int image_cols = std::stoi(argv[2]);

    std::vector<uint8_t> binary_image(image_rows * image_cols);
    std::vector<uint32_t> component_tracker(image_rows * image_cols);

    generateRandomImage(binary_image, image_rows, image_cols);

    placeBorderAroundImage(binary_image, image_rows, image_cols);

    getComponents(binary_image, component_tracker, image_rows, image_cols);

    

    // print2DVector("ORIGINAL IMAGE", binary_image);
    // print2DVector("COMPONENT TRACKER", component_tracker);

    return 0;
}