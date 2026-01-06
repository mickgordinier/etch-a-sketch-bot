#include <iostream>
#include <vector>
#include <random>
#include <queue>

template <typename T>
void
print2DVector(
    const std::vector<std::vector<T>> &vec
) {
    int image_rows = vec.size();
    int image_cols = vec[0].size();

    for (int row = 0; row < image_rows; ++row) {
        for (int col = 0; col < image_cols; ++col) {
            std::cout << vec[row][col] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

bool inline
checkNode(
    const std::vector<std::vector<uint8_t>> &binary_image, 
    const std::vector<std::vector<int>> &component_tracker,
    int nodeRow, int nodeCol
) {
    if ((nodeRow < 0) || (nodeCol < 0) || (nodeRow >= binary_image.size()) || (nodeCol >= binary_image[0].size())) return false;;
    if (!binary_image[nodeRow][nodeCol]) return false;
    if (component_tracker[nodeRow][nodeCol]) return false;
    return true;
}

void
generateNewComponent(
    const std::vector<std::vector<uint8_t>> &binary_image, 
    std::vector<std::vector<int>> &component_tracker,
    int row, int col, int component_idx 
) {
    if (!checkNode(binary_image, component_tracker, row, col)) return;

    component_tracker[row][col] = component_idx;

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
                if (!checkNode(binary_image, component_tracker, nodeRow+i, nodeCol+j)) continue;
                component_tracker[nodeRow+i][nodeCol+j] = component_idx;
                nodesToCheck.push({nodeRow+i, nodeCol+j});
            }
        }
    }
}

void 
getComponents(
    const std::vector<std::vector<uint8_t>> &binary_image, 
    std::vector<std::vector<int>> &component_tracker
) {
    int image_rows = binary_image.size();
    int image_cols = binary_image[0].size();

    int component_idx = 1;

    for (int row = 0; row < image_rows; ++row) {
        for (int col = 0; col < image_cols; ++col) {
            if (binary_image[row][col] && !component_tracker[row][col]) {
                generateNewComponent(binary_image, component_tracker, row, col, component_idx);
                ++component_idx;
            }
        }
    }
}

void
generateRandomImage(
    std::vector<std::vector<uint8_t>> &binary_image,
    int image_rows, int image_cols
) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.2); // 50% chance of 1

    for (int i = 0; i < image_rows; ++i) {
        for (int j = 0; j < image_cols; ++j) {
            binary_image[i][j] = dist(gen);
        }
    }
}


int 
main() 
{
    // const std::vector<std::vector<uint8_t>> binary_image = {
    //     {1, 0, 0, 0, 0},
    //     {0, 1, 0, 1, 0},
    //     {0, 1, 0, 0, 0},
    //     {0, 1, 1, 1, 1}
    // };

    int image_rows = 10000;
    int image_cols = 10000;

    std::vector<std::vector<uint8_t>> binary_image(
        image_rows, std::vector<uint8_t>(image_cols, 0));

    generateRandomImage(binary_image, image_rows, image_cols);

    std::cout << "ORIGINAL IMAGE\n\n";
    print2DVector(binary_image);

    std::vector<std::vector<int>> component_tracker(
        image_rows, std::vector<int>(image_cols, 0));

    getComponents(binary_image, component_tracker);

    std::cout << "COMPONENT TRACKER\n\n";
    print2DVector(component_tracker);

    return 0;
}