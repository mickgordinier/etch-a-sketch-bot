#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <string>
#include <iomanip>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb/stb_image_write.h"

const int adjacentNodes[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

uint8_t EXTRA_PRINT = 1;
uint8_t BASIC_PRINT = 1;


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
    int distance;
    std::pair<int, int> currentNode;
    std::pair<int, int> originalNode;
};

struct CompareByDistance {
    bool operator()(const QueueNode& a, const QueueNode& b) const {
        return a.distance > b.distance; // min-heap
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


bool inline
checkNode(
    const std::vector<uint8_t> &binary_image, 
    const std::vector<int> &nodes_tracker,
    int nodeRow, int nodeCol,
    int height, int width
) {
    if ((nodeRow < 0) || (nodeCol < 0) || (nodeRow >= height) || (nodeCol >= width)) return false;;
    if (!binary_image[(nodeRow*width) + nodeCol]) return false;
    return (nodes_tracker[(nodeRow*width) + nodeCol] == -1);
}


// bool inline
// checkNode(
//     const std::vector<uint8_t> &binary_image, 
//     const std::vector<uint8_t> &component_tracker,
//     int nodeRow, int nodeCol,
//     int height, int width
// ) {
//     if ((nodeRow < 0) || (nodeCol < 0) || (nodeRow >= height) || (nodeCol >= width)) return false;;
//     if (!binary_image[(nodeRow*width) + nodeCol]) return false;
//     if (component_tracker[(nodeRow*width) + nodeCol]) return false;
//     return true;
// }


void
generateRandomImage(
    std::vector<uint8_t> &binary_image,
    int height, int width
) {
    ScopedTimer timer("Generate Random Image");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.2);  // density of 1's

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            binary_image[i*width + j] = dist(gen);
        }
    }
}


void
placeBorderAroundImage(
    std::vector<uint8_t> &binary_image,
    int height, int width
) {
    ScopedTimer timer("Place Border Around Image");

    for (int col = 0; col < width; ++col) {
        binary_image[col] = 1;
        binary_image[((height-1)*width) + col] = 1;
    }

    for (int row = 1; row < height-1; ++row) {
        binary_image[row*width] = 1;
        binary_image[(row*width) + (width-1)] = 1;
    }
}

void
addComponentToQueue(
    const std::vector<uint8_t> &binary_image,
    std::vector<int> &nodes_tracker,
    std::priority_queue<QueueNode, std::vector<QueueNode>, CompareByDistance> &nodePq,
    int start_row, int start_col,
    int height, int width
) {

    nodePq.push({0, {start_row, start_col}, {-1, -1}});
    nodes_tracker[(start_row*width) + start_col] = 0;

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

            if (!checkNode(binary_image, nodes_tracker, adjacentRow, adjacentCol, height, width)) 
                continue;

            nodePq.push({0, {adjacentRow, adjacentCol}, {-1, -1}});
            nodes_tracker[(adjacentRow*width) + adjacentCol] = 0;
            
            nodesToCheck.push({adjacentRow, adjacentCol});
        }
    }
}


// void
// addComponentToQueue(
//     const std::vector<uint8_t> &binary_image,
//     std::vector<uint8_t> &fill_tracker,
//     std::vector<uint8_t> &tempTracker,
//     std::queue<QueueNode> &globalSavedQueue,
//     int start_row, int start_col,
//     int height, int width
// ) {
//     globalSavedQueue.push({{start_row, start_col}, {0, 0}});
//     fill_tracker[(start_row*width) + start_col] = 1;
//     tempTracker[(start_row*width) + start_col] = 1;

//     std::queue<std::pair<int, int>> nodesToCheck;
//     nodesToCheck.push({start_row, start_col});

//     while(!nodesToCheck.empty()) {
//         std::pair<int, int> currNode = nodesToCheck.front();
//         nodesToCheck.pop();

//         int nodeRow = currNode.first;
//         int nodeCol = currNode.second;

//         // As stepper motors only moves one at a time, it is not feasible to go diagonal.
//         // Thus, a valid neighbor must be directly adjacent, not diagonal. 
//         for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
//             int adjacentRow = nodeRow + adjacentNodes[neighbor][0];
//             int adjacentCol = nodeCol + adjacentNodes[neighbor][1];

//             if (!checkNode(binary_image, fill_tracker, adjacentRow, adjacentCol, height, width)) 
//                 continue;

//             fill_tracker[(adjacentRow*width) + adjacentCol] = 1;
//             tempTracker[(adjacentRow*width) + adjacentCol] = 1;
//             nodesToCheck.push({adjacentRow, adjacentCol});
//             globalSavedQueue.push({{adjacentRow, adjacentCol}, {0, 0}});
//         }
//     }
// }



void
addPathToQueue(
    std::vector<uint8_t> &binary_image,
    std::vector<int> &nodes_tracker,
    std::priority_queue<QueueNode, std::vector<QueueNode>, CompareByDistance> &nodePq,
    int islandRow, int islandCol,
    int originalRow, int originalCol,
    int height, int width
) {
    // Making a simple shortest path from original node to island node
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
        for (int rowToAdd = std::min(startRow, endRow); rowToAdd < std::max(startRow, endRow); ++rowToAdd) {
            
            // Need to make sure not adding any duplicate nodes
            if (nodes_tracker[(rowToAdd*width) + startCol] == 0) continue;

            binary_image[(rowToAdd*width) + startCol] = 1;
            nodes_tracker[(rowToAdd*width) + startCol] = 0;
            nodePq.push({0, {rowToAdd, startCol}, {-1, -1}});
        }
        return;
    }

    double m = (double)dy / (double)dx;

    int currentRow = startRow;

    for (int col = startCol; col <= endCol; ++col) {
        int trueRow = round(startRow + (m * (col - startCol)));

        for (int rowToAdd = std::min(currentRow, trueRow); rowToAdd <= std::max(currentRow, trueRow); ++rowToAdd) {
            
            // Need to make sure not adding any duplicate nodes
            if (nodes_tracker[(rowToAdd*width) + col] == 0) continue;

            binary_image[(rowToAdd*width) + col] = 1;
            nodes_tracker[(rowToAdd*width) + col] = 0;
            nodePq.push({0, {rowToAdd, col}, {-1, -1}});
        }

        currentRow = trueRow;
    }
}


// void
// addPathToQueue(
//     std::vector<uint8_t> &binary_image,
//     std::vector<uint8_t> &fill_tracker,
//     std::vector<int> &tempTracker,
//     std::queue<QueueNode> &globalSavedQueue,
//     int islandRow, int islandCol,
//     int originalRow, int originalCol,
//     int height, int width
// ) {
//     // Making a simple shortest path from original node to island node
//     // Using Bresenham’s Line Generation Algorithm

//     int startRow, endRow, startCol, endCol;

//     if (islandCol < originalCol) {
//         startRow = islandRow; startCol = islandCol;
//         endRow = originalRow; endCol = originalCol;
//     } else {
//         startRow = originalRow; startCol = originalCol;
//         endRow = islandRow; endCol = islandCol;
//     }

//     int dx = endCol - startCol;
//     int dy = endRow - startRow;

//     if (!dx) {
//         for (int rowToAdd = std::min(startRow, endRow); rowToAdd < std::max(startRow, endRow); ++rowToAdd) {
            
//             // Need to make sure not adding any duplicate nodes
//             if (fill_tracker[(rowToAdd*width) + startCol]) continue;

//             binary_image[(rowToAdd*width) + startCol] = 1;
//             fill_tracker[(rowToAdd*width) + startCol] = 1;
//             tempTracker[(rowToAdd*width) + startCol] = 1;
//             globalSavedQueue.push({{rowToAdd, startCol}, {0, 0}});
//         }
//         return;
//     }

//     double m = (double)dy / (double)dx;

//     int currentRow = startRow;

//     for (int col = startCol; col <= endCol; ++col) {
//         int trueRow = round(startRow + (m * (col - startCol)));

//         for (int rowToAdd = std::min(currentRow, trueRow); rowToAdd <= std::max(currentRow, trueRow); ++rowToAdd) {
            
//             // Need to make sure not adding any duplicate nodes
//             if (fill_tracker[(rowToAdd*width) + col]) continue;

//             binary_image[(rowToAdd*width) + col] = 1;
//             fill_tracker[(rowToAdd*width) + col] = 1;
//             tempTracker[(rowToAdd*width) + startCol] = 1;
//             globalSavedQueue.push({{rowToAdd, col}, {0, 0}});
//         }

//         currentRow = trueRow;
//     }
// }


std::vector<uint8_t>
connectAllComponents(
    const std::vector<uint8_t> &original_binary_image,
    int height, int width
) {
    ScopedTimer timer("Connect all components");

    std::vector<uint8_t> final_binary_image = original_binary_image;

    std::priority_queue<QueueNode, std::vector<QueueNode>, CompareByDistance> nodePq;

    std::vector<int> nodes_tracker(height * width, -1);

    // Adding the entire component found at location (0, 0) to the globalSavedQueue
    // INEFFICIENT: USING TEMPORARY FILL_TRACKER HERE. NEED TO REMOVE
    addComponentToQueue(original_binary_image, nodes_tracker, nodePq, 0, 0, height, width);

    if (EXTRA_PRINT) print2DVector("Fill Tracker Adding Border Component", nodes_tracker, height, width);


    // Perform an exhaustive BFS to find the next nearest island component to connect to the final component
    while(!nodePq.empty()) {

        // Current node is either already connected or is not written to
        QueueNode nodeToCheck = nodePq.top();
        nodePq.pop();

        int currentRow = nodeToCheck.currentNode.first;
        int currentCol = nodeToCheck.currentNode.second;

        // Node has already been proceed and can be ignored
        int distance = nodeToCheck.distance;
        if (distance > nodes_tracker[(currentRow*width) + currentCol]) continue;

        int originalRow = nodeToCheck.originalNode.first;
        int originalCol = nodeToCheck.originalNode.second;
        
        int adjacentDistance = distance + 1;

        // Check the neighbors
        for (int neighbor = 0; neighbor < 4; ++neighbor) {
        
            int adjacentRow = currentRow + adjacentNodes[neighbor][0];
            int adjacentCol = currentCol + adjacentNodes[neighbor][1];

            if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;

            // If it is already being visited by someone closer, ignore
            int node_track_val = nodes_tracker[(adjacentRow*width) + adjacentCol];
            if ((node_track_val <= adjacentDistance) && (node_track_val != -1)) continue;
        
            // New island component found
            if (original_binary_image[(adjacentRow*width) + adjacentCol]) {

                if (EXTRA_PRINT) {
                    print2DVector("Round Tracked Locations Before Finding New Component", nodes_tracker, height, width);
                    std::cout << "New island component found at: (" << adjacentRow << ", " << adjacentCol << ")\n\n";
                }

                // Add the entire component found at given location to the globalSavedQueue
                addComponentToQueue(original_binary_image, nodes_tracker, nodePq, adjacentRow, adjacentCol, height, width);
                
                if (EXTRA_PRINT) {
                    print2DVector("Fill Tracker Adding New Component", nodes_tracker, height, width);
                }
                
                // Add a path the connects the island component
                if (adjacentDistance == 1) {
                    // This can happen is we add new path pixels that are adjacent to islands
                    if (EXTRA_PRINT) std::cout << "Starting location at: (" << currentRow << ", " << currentCol << ")\n\n";
                    addPathToQueue(final_binary_image, nodes_tracker, nodePq,
                                    adjacentRow, adjacentCol, currentRow, currentCol,
                                    height, width);
                } else {
                    if (EXTRA_PRINT) std::cout << "Starting location at: (" << originalRow << ", " << originalCol << ")\n\n";
                    addPathToQueue(final_binary_image, nodes_tracker, nodePq,
                                    adjacentRow, adjacentCol, originalRow, originalCol,
                                    height, width);
                }

                if (EXTRA_PRINT) print2DVector("Fill Tracker Adding New Component+Path", nodes_tracker, height, width);

            } else {
                // Otherwise, just add blank space to expand
                if (adjacentDistance == 1) {
                    nodePq.push({adjacentDistance, {adjacentRow, adjacentCol}, {currentRow, currentCol}});
                } else {
                    nodePq.push({adjacentDistance, {adjacentRow, adjacentCol}, {originalRow, originalCol}});
                }
                    
                nodes_tracker[(adjacentRow*width) + adjacentCol] = adjacentDistance;
            }
        }
    }

    return final_binary_image;



    // std::vector<uint8_t> fill_tracker(height * width);

    // std::queue<QueueNode> globalSavedQueue;

    // std::vector<uint8_t> final_binary_image = original_binary_image;

    // // Adding the entire component found at location (0, 0) to the globalSavedQueue
    // // INEFFICIENT: USING TEMPORARY FILL_TRACKER HERE. NEED TO REMOVE
    // addComponentToQueue(original_binary_image, fill_tracker, fill_tracker, globalSavedQueue, 0, 0, height, width);

    // if (EXTRA_PRINT) print2DVector("Fill Tracker Adding Border Component", fill_tracker, height, width);
    
    // // Iterative process
    // while(true) {

    //     // 1. Indicate all of the currently connected nodes (Distance = 0)
    //     // All nodes in the round queue are all connected
    //     std::queue<QueueNode> roundQueue = globalSavedQueue;

    //     std::vector<uint8_t> tempTracker = fill_tracker;

    //     uint8_t new_component_found = 0;

    //     // 2. Perform an exhaustive BFS to find the next nearest island component to connect to the final component
    //     while(!roundQueue.empty()) {

    //         // Current node is either already connected or is not written to
    //         QueueNode nodeToCheck = roundQueue.front();
    //         roundQueue.pop();

    //         int currentRow = nodeToCheck.currentNode.first;
    //         int currentCol = nodeToCheck.currentNode.second;

    //         int originalRow = nodeToCheck.originalNode.first;
    //         int originalCol = nodeToCheck.originalNode.second;

    //         // Check the neighbors
    //         for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
    //             int adjacentRow = currentRow + adjacentNodes[neighbor][0];
    //             int adjacentCol = currentCol + adjacentNodes[neighbor][1];

    //             if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
    //             if (tempTracker[(adjacentRow*width) + adjacentCol]) continue;
            
    //             // New island component found
    //             if (original_binary_image[(adjacentRow*width) + adjacentCol]) {

    //                 if (EXTRA_PRINT) {
    //                     print2DVector("Round Tracked Locations Before Finding New Component", tempTracker, height, width);
    //                     std::cout << "New island component found at: (" << adjacentRow << ", " << adjacentCol << ")\n\n";
    //                 }

    //                 // Add the entire component found at given location to the globalSavedQueue
    //                 addComponentToQueue(original_binary_image, fill_tracker, tempTracker, globalSavedQueue, adjacentRow, adjacentCol, height, width);
                    
    //                 if (EXTRA_PRINT) {
    //                     print2DVector("Fill Tracker Adding New Component", fill_tracker, height, width);
    //                 }
                    
    //                 // Add a path the connects the island component
    //                 if (!originalRow && !originalCol) {
    //                     // This can happen is we add new path pixels that are adjacent to islands
    //                     if (EXTRA_PRINT) std::cout << "Starting location at: (" << currentRow << ", " << currentCol << ")\n\n";
    //                     addPathToQueue(final_binary_image, fill_tracker, tempTracker, globalSavedQueue,
    //                                     adjacentRow, adjacentCol, currentRow, currentCol,
    //                                     height, width);
    //                 } else {
    //                     if (EXTRA_PRINT) std::cout << "Starting location at: (" << originalRow << ", " << originalCol << ")\n\n";
    //                     addPathToQueue(final_binary_image, fill_tracker, tempTracker, globalSavedQueue,
    //                                     adjacentRow, adjacentCol, originalRow, originalCol,
    //                                     height, width);
    //                 }

    //                 if (EXTRA_PRINT) print2DVector("Fill Tracker Adding New Component+Path", fill_tracker, height, width);

    //                 new_component_found = 1;

    //             } else {
    //                 // Otherwise, just add blank space to expand
    //                 if (!new_component_found) {
    //                     if (!originalRow && !originalCol) {
    //                         roundQueue.push({{adjacentRow, adjacentCol}, {currentRow, currentCol}});
    //                     } else {
    //                         roundQueue.push({{adjacentRow, adjacentCol}, {originalRow, originalCol}});
    //                     }
    //                 }
                        
    //                 tempTracker[(adjacentRow*width) + adjacentCol] = 1;
    //             }
    //         }
    //     }

    //     if (!new_component_found) {
    //         return final_binary_image;
    //     }
    //     new_component_found = 0;
    // }
}


int 
main(int argc, char **argv) 
{
    ScopedTimer program_timer("TOTAL PROGRAM");

    int height, width;
    std::string output_filepath;

    std::vector<uint8_t> binary_image;

    // {executable}.out {height} {width} {output_filepath} {BASIC_PRINT} {EXTRA_PRINT}
    if (argc == 6) {

        height = std::stoi(argv[1]);
        width = std::stoi(argv[2]);
        output_filepath = argv[3];
        BASIC_PRINT = std::stoi(argv[4]);
        EXTRA_PRINT = std::stoi(argv[5]);

        // std::vector<uint8_t> binary_image = {
        // 1,   1,   1,   1,   1,   1,   1,   1,   1,   1, 
        // 1,   0,   0,   1,   0,   0,   0,   0,   0,   1, 
        // 1,   0,   0,   0,   0,   0,   0,   0,   0,   1, 
        // 1,   1,   0,   0,   0,   1,   0,   0,   0,   1, 
        // 1,   0,   0,   0,   0,   0,   0,   0,   0,   1, 
        // 1,   0,   0,   1,   0,   0,   0,   0,   0,   1, 
        // 1,   0,   1,   0,   0,   0,   0,   0,   0,   1, 
        // 1,   1,   0,   0,   1,   0,   0,   0,   0,   1, 
        // 1,   0,   0,   0,   0,   0,   0,   0,   0,   1, 
        // 1,   1,   1,   1,   1,   1,   1,   1,   1,   1
        // };
        
        binary_image.resize(height * width);
        generateRandomImage(binary_image, height, width);

    } 
    // {executable}.out {input_filepath} {output_filepath} {BASIC_PRINT} {EXTRA_PRINT}
    else if (argc == 5) {

        int channels;
        const char * input_filepath = argv[1];
        output_filepath = argv[2];
        BASIC_PRINT = std::stoi(argv[3]);
        EXTRA_PRINT = std::stoi(argv[4]);

        unsigned char* data = stbi_load(input_filepath, &width, &height, &channels, 1);
        if (!data) {
            std::cerr << "Failed to load image\n";
            return 1;
        }

        binary_image.resize(height * width);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                binary_image[y * width + x] = data[y * width + x] > 128 ? 0 : 1;
            }
        }

        stbi_image_free(data);

    } else {
        std::cout << "Wrong arguments provided\n";
        return 1;
    }

    placeBorderAroundImage(binary_image, height, width);
    // binary_image[0] = 1;
    
    if (BASIC_PRINT) print2DVector("ORIGINAL IMAGE", binary_image, height, width);
    
    std::vector<uint8_t> final_binary_image = connectAllComponents(binary_image, height, width);

    if (BASIC_PRINT) print2DVector("FINAL IMAGE", final_binary_image, height, width);

    for (int i = 0; i < height * width; ++i) {
        final_binary_image[i] = (final_binary_image[i]) ? 0 : 255;
    }

    // Writing to output filepath
    if (!stbi_write_bmp(output_filepath.c_str(), width, height, 1, final_binary_image.data())) {
        std::cerr << "Failed to write BMP\n";
        return 1;
    }

    return 0;
}