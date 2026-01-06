#include <iostream>
#include <vector>
#include <random>

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

void
generateNewComponent(
    const std::vector<std::vector<bool>> &binary_iamge, 
    std::vector<std::vector<int>> &component_tracker,
    int row, int col, int component_idx 
) {
    if ((row < 0) || (col < 0) || (row >= binary_iamge.size()) || (col >= binary_iamge[0].size())) return;
    if (!binary_iamge[row][col]) return;
    if (component_tracker[row][col]) return;

    component_tracker[row][col] = component_idx;

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (!i && !j) continue;
            generateNewComponent(binary_iamge, component_tracker, row+i, col+j, component_idx);
        }
    }
}

void 
getComponents(
    const std::vector<std::vector<bool>> &binary_iamge, 
    std::vector<std::vector<int>> &component_tracker
) {
    int image_rows = binary_iamge.size();
    int image_cols = binary_iamge[0].size();

    int component_idx = 1;

    for (int row = 0; row < image_rows; ++row) {
        for (int col = 0; col < image_cols; ++col) {
            if (binary_iamge[row][col] && !component_tracker[row][col]) {
                generateNewComponent(binary_iamge, component_tracker, row, col, component_idx);
                ++component_idx;
            }
        }
    }
}

void
generateRandomImage(
    std::vector<std::vector<bool>> &binary_iamge,
    int image_rows, int image_cols
) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.2); // 50% chance of 1

    for (int i = 0; i < image_rows; ++i) {
        for (int j = 0; j < image_cols; ++j) {
            binary_iamge[i][j] = dist(gen);
        }
    }
}


int 
main() 
{
    // const std::vector<std::vector<bool>> binary_iamge = {
    //     {1, 0, 0, 0, 0},
    //     {0, 1, 0, 1, 0},
    //     {0, 1, 0, 0, 0},
    //     {0, 1, 1, 1, 1}
    // };

    int image_rows = 10;
    int image_cols = 10;

    std::vector<std::vector<bool>> binary_image(
        image_rows, std::vector<bool>(image_cols, 0));

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