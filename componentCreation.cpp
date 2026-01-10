#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <string>
#include <iomanip>
#include <chrono>

const int adjacentNodes[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

uint8_t EXTRA_PRINT = 1;


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


struct QueueNode {
    std::pair<int, int> currentNode;
    std::pair<int, int> originalNode;
};


template <typename T>
void
print2DVector(
    const std::string &image_name,
    const std::vector<T> &vec,
    int image_rows, int image_cols
) {
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";
    std::cout << image_name << "\n";
    std::cout << "-----------------------------------------------------------------------------------------------------------\n";

    for (int row = 0; row < image_rows; ++row) {
        for (int col = 0; col < image_cols; ++col) {
            std::cout << std::setw(3) << static_cast<int>(vec[row*image_cols + col]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "-----------------------------------------------------------------------------------------------------------\n\n";

}


bool inline
checkNode(
    const std::vector<uint8_t> &binary_image, 
    const std::vector<uint8_t> &component_tracker,
    int nodeRow, int nodeCol,
    int image_rows, int image_cols
) {
    if ((nodeRow < 0) || (nodeCol < 0) || (nodeRow >= image_rows) || (nodeCol >= image_cols)) return false;;
    if (!binary_image[(nodeRow*image_cols) + nodeCol]) return false;
    if (component_tracker[(nodeRow*image_cols) + nodeCol]) return false;
    return true;
}


// void
// generateNewComponent(
//     const std::vector<uint8_t> &binary_image, 
//     std::vector<uint8_t> &component_tracker,
//     int row, int col, int component_idx,
//     int image_rows, int image_cols
// ) {
//     if (!checkNode(binary_image, component_tracker, row, col, image_rows, image_cols)) return;

//     component_tracker[(row*image_cols) + col] = component_idx;

//     std::queue<std::pair<int, int>> nodesToCheck;
//     nodesToCheck.push({row, col});

//     while(!nodesToCheck.empty()) {
//         std::pair<int, int> currNode = nodesToCheck.front();
//         nodesToCheck.pop();

//         int nodeRow = currNode.first;
//         int nodeCol= currNode.second;

//         // As stepper motors only moves one at a time, it is not feasible to go diagonal.
//         // Thus, a valid neighbor must be directly adjacent, not diagonal. 
//         for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
//             int adjacentRow = nodeRow + adjacentNodes[neighbor][0];
//             int adjacentCol = nodeCol + adjacentNodes[neighbor][1];

//             if (!checkNode(binary_image, component_tracker, adjacentRow, adjacentCol, image_rows, image_cols)) 
//                 continue;

//             component_tracker[(adjacentRow*image_cols) + adjacentCol] = component_idx;
//             nodesToCheck.push({adjacentRow, adjacentCol});
//         }
//     }
// }

// void 
// getComponents(
//     const std::vector<uint8_t> &binary_image, 
//     std::vector<uint8_t> &component_tracker,
//     int image_rows, int image_cols
// ) {
//     ScopedTimer timer("Generate Component Labelling");

//     int component_idx = 1;

//     for (int row = 0; row < image_rows; ++row) {
//         for (int col = 0; col < image_cols; ++col) {
//             if (binary_image[(row*image_cols) + col] && !component_tracker[(row*image_cols) + col]) {
//                 generateNewComponent(binary_image, component_tracker, row, col, component_idx, image_rows, image_cols);
//                 ++component_idx;
//             }
//         }
//     }
// }


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

void
addComponentToQueue(
    const std::vector<uint8_t> &binary_image,
    std::vector<uint8_t> &fill_tracker,
    std::queue<QueueNode> &globalSavedQueue,
    int start_row, int start_col,
    int image_rows, int image_cols
) {
    globalSavedQueue.push({{start_row, start_col}, {0, 0}});
    fill_tracker[(start_row*image_cols) + start_col] = 1;

    std::queue<std::pair<int, int>> nodesToCheck;
    nodesToCheck.push({start_row, start_col});

    while(!nodesToCheck.empty()) {
        std::pair<int, int> currNode = nodesToCheck.front();
        nodesToCheck.pop();

        int nodeRow = currNode.first;
        int nodeCol = currNode.second;

        // As stepper motors only moves one at a time, it is not feasible to go diagonal.
        // Thus, a valid neighbor must be directly adjacent, not diagonal. 
        for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
            int adjacentRow = nodeRow + adjacentNodes[neighbor][0];
            int adjacentCol = nodeCol + adjacentNodes[neighbor][1];

            if (!checkNode(binary_image, fill_tracker, adjacentRow, adjacentCol, image_rows, image_cols)) 
                continue;

            fill_tracker[(adjacentRow*image_cols) + adjacentCol] = 1;
            nodesToCheck.push({adjacentRow, adjacentCol});
            globalSavedQueue.push({{adjacentRow, adjacentCol}, {0, 0}});
        }
    }
}

void
addPathToQueue(
    std::vector<uint8_t> &binary_image,
    std::vector<uint8_t> &fill_tracker,
    std::queue<QueueNode> &globalSavedQueue,
    int islandRow, int islandCol,
    int originalRow, int originalCol,
    int image_rows, int image_cols
) {
    // Making a simple shortest path from original node to island node
    // Using Bresenham’s Line Generation Algorithm

    int startRow, endRow, startCol, endCol;

    if (islandCol < originalCol) {
        startRow = islandRow; startCol = islandCol;
        endRow = originalRow; endCol = originalCol;
    } else {
        startRow = originalRow; startCol = originalCol;
        endRow = islandRow; endCol = islandCol;
    }

    int dx = endCol - startCol;
    int dy = endRow - startRow;

    if (!dx) {
        for (int rowToAdd = startRow; rowToAdd < endRow; ++rowToAdd) {
            
            // Need to make sure not adding any duplicate nodes
            if (fill_tracker[(rowToAdd*image_cols) + startCol]) continue;

            binary_image[(rowToAdd*image_cols) + startCol] = 1;
            fill_tracker[(rowToAdd*image_cols) + startCol] = 1;
            globalSavedQueue.push({{rowToAdd, startCol}, {0, 0}});
        }
        return;
    }

    double m = (double)dy / (double)dx;

    int currentRow = startRow;

    for (int col = startCol; col <= endCol; ++col) {
        int trueRow = round(startRow + (m * (col - startCol)));

        for (int rowToAdd = std::min(currentRow, trueRow); rowToAdd <= std::max(currentRow, trueRow); ++rowToAdd) {
            
            // Need to make sure not adding any duplicate nodes
            if (fill_tracker[(rowToAdd*image_cols) + col]) continue;

            binary_image[(rowToAdd*image_cols) + col] = 1;
            fill_tracker[(rowToAdd*image_cols) + col] = 1;
            globalSavedQueue.push({{rowToAdd, col}, {0, 0}});
        }

        currentRow = trueRow;
    }
}


std::vector<uint8_t>
connectAllComponents(
    const std::vector<uint8_t> &original_binary_image,
    std::vector<uint8_t> &fill_tracker,
    int image_rows, int image_cols
) {
    std::queue<QueueNode> globalSavedQueue;

    std::vector<uint8_t> final_binary_image = original_binary_image;

    // Adding the entire component found at location (0, 0) to the globalSavedQueue
    addComponentToQueue(original_binary_image, fill_tracker, globalSavedQueue, 0, 0, image_rows, image_cols);

    if (EXTRA_PRINT) print2DVector("Fill Tracker Adding Border Component", fill_tracker, image_rows, image_cols);
    
    // Iterative process
    while(true) {

        // 1. Indicate all of the currently connected nodes (Distance = 0)
        // All nodes in the round queue are all connected
        std::queue<QueueNode> roundQueue = globalSavedQueue;

        std::vector<uint8_t> tempTracker = fill_tracker;

        uint8_t new_component_found = 0;

        // 2. Perform an exhaustive BFS to find the next nearest island component to connect to the final component
        while(!roundQueue.empty()) {

            // Current node is either already connected or is not written to
            QueueNode nodeToCheck = roundQueue.front();
            roundQueue.pop();

            int currentRow = nodeToCheck.currentNode.first;
            int currentCol = nodeToCheck.currentNode.second;

            int originalRow = nodeToCheck.originalNode.first;
            int originalCol = nodeToCheck.originalNode.second;

            // Check the neighbors
            for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
                int adjacentRow = currentRow + adjacentNodes[neighbor][0];
                int adjacentCol = currentCol + adjacentNodes[neighbor][1];

                if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= image_rows) || (adjacentCol >= image_cols)) continue;
                if (tempTracker[(adjacentRow*image_cols) + adjacentCol]) continue;
            
                // New island component found
                if (original_binary_image[(adjacentRow*image_cols) + adjacentCol]) {

                    if (EXTRA_PRINT) {
                        print2DVector("Round Tracked Locations Before Finding New Component", tempTracker, image_rows, image_cols);
                        std::cout << "New island component found at: (" << adjacentRow << ", " << adjacentCol << ")\n\n";
                    }

                    // Add the entire component found at given location to the globalSavedQueue
                    addComponentToQueue(original_binary_image, fill_tracker, globalSavedQueue, adjacentRow, adjacentCol, image_rows, image_cols);
                    
                    if (EXTRA_PRINT) {
                        print2DVector("Fill Tracker Adding New Component", fill_tracker, image_rows, image_cols);
                    }
                    
                    // Add a path the connects the island component
                    if (!originalRow && !originalCol) {
                        // This can happen is we add new path pixels that are adjacent to islands
                        if (EXTRA_PRINT) std::cout << "Starting location at: (" << currentRow << ", " << currentCol << ")\n\n";
                        addPathToQueue(final_binary_image, fill_tracker, globalSavedQueue,
                                        adjacentRow, adjacentCol, currentRow, currentCol,
                                        image_rows, image_cols);
                    } else {
                        if (EXTRA_PRINT) std::cout << "Starting location at: (" << originalRow << ", " << originalCol << ")\n\n";
                        addPathToQueue(final_binary_image, fill_tracker, globalSavedQueue,
                                        adjacentRow, adjacentCol, originalRow, originalCol,
                                        image_rows, image_cols);
                    }

                    if (EXTRA_PRINT) print2DVector("Fill Tracker Adding New Component+Path", fill_tracker, image_rows, image_cols);

                    new_component_found = 1;

                } else {
                    // Otherwise, just add blank space to expand
                    if (!new_component_found) {
                        if (!originalRow && !originalCol) {
                            roundQueue.push({{adjacentRow, adjacentCol}, {currentRow, currentCol}});
                        } else {
                            roundQueue.push({{adjacentRow, adjacentCol}, {originalRow, originalCol}});
                        }
                    }
                        
                    tempTracker[(adjacentRow*image_cols) + adjacentCol] = 1;
                }
            }
        }

        if (!new_component_found) return final_binary_image;
        new_component_found = 0;
    }
}


int 
main(int argc, char **argv) 
{
    ScopedTimer program_timer("TOTAL PROGRAM");

    if (argc != 4) {
        std::cout << "Required ./a.out {image_rows} {image_cols} {extra_print}\n";
        return 1;
    }

    int image_rows = std::stoi(argv[1]);
    int image_cols = std::stoi(argv[2]);

    EXTRA_PRINT = std::stoi(argv[3]);

    // std::vector<uint8_t> binary_image = {
    //     1,   0,   0,   1,   0,   0,   0, 
    //     0,   0,   0,   0,   0,   0,   1, 
    //     0,   0,   0,   0,   0,   0,   0, 
    //     0,   0,   1,   0,   0,   1,   0, 
    //     1,   0,   0,   0,   1,   0,   0, 
    //     0,   0,   1,   0,   0,   0,   0, 
    //     0,   0,   0,   0,   0,   0,   1
    // };
    std::vector<uint8_t> binary_image(image_rows * image_cols);
    generateRandomImage(binary_image, image_rows, image_cols);

    placeBorderAroundImage(binary_image, image_rows, image_cols);
    // binary_image[0] = 1;
    
    // std::vector<uint32_t> component_tracker(image_rows * image_cols);
    // getComponents(binary_image, component_tracker, image_rows, image_cols);
    
    print2DVector("ORIGINAL IMAGE", binary_image, image_rows, image_cols);
    
    std::vector<uint8_t> fill_tracker(image_rows * image_cols);
    std::vector<uint8_t> final_binary_image = connectAllComponents(binary_image, fill_tracker, image_rows, image_cols);

    print2DVector("FINAL IMAGE", final_binary_image, image_rows, image_cols);


    // findComponentNearestNeighbor()

    // print2DVector("COMPONENT TRACKER", component_tracker, image_rows, image_cols);

    return 0;
}