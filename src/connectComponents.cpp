#include <vector>
#include <cstdint>
#include <queue>
#include <cmath>

#include "../include/debugHelp.hpp"
#include "../include/generatePath.hpp"

struct QueueNode {
    std::pair<int, int> currentNode;
    std::pair<int, int> originalNode;
};

// Will only return true if
// i. Location is valid
// ii. The pixel exist at location
// iii. The pixel is not yet added to the zero distance queue
static bool inline
doesPixelExist(
    const std::vector<uint8_t> &binary_image, 
    const std::vector<int>     &nodes_tracker,
    int                         nodeRow, 
    int                         nodeCol,
    int                         height, 
    int                         width
) {
    if ((nodeRow < 0) || (nodeCol < 0) || (nodeRow >= height) || (nodeCol >= width)) return false;;
    if (!binary_image[(nodeRow*width) + nodeCol]) return false;
    return (nodes_tracker[(nodeRow*width) + nodeCol] == -1);
}

// Adding all pixels in individual component to the nodes_tracker and zero_distance_queue
static void
addComponentToQueue(
    const std::vector<uint8_t> &binary_image,
    std::vector<int>           &nodes_tracker,
    std::queue<QueueNode>      &zero_distance_queue,
    int                         start_row, 
    int                         start_col,
    int                         height, 
    int                         width
) {

    // All identified components will have pixels that don't point back to anything
    // Additionally node_tracker will indicate the distance is 0 for each pixel
    zero_distance_queue.push({{start_row, start_col}, {-1, -1}});
    nodes_tracker[(start_row*width) + start_col] = 0;

    // Performing DFS to look for all pixels in component
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

            if (!doesPixelExist(binary_image, nodes_tracker, adjacentRow, adjacentCol, height, width)) 
                continue;

            // All identified components will have pixels that don't point back to anything
            // Additionally node_tracker will indicate the distance is 0 for each pixel
            zero_distance_queue.push({{adjacentRow, adjacentCol}, {-1, -1}});
            nodes_tracker[(adjacentRow*width) + adjacentCol] = 0;
            
            nodesToCheck.push({adjacentRow, adjacentCol});
        }
    }
}


static void
addPathToQueue(
    std::vector<uint8_t> &binary_image,
    std::vector<int> &nodes_tracker,
    std::queue<QueueNode> &zero_distance_queue,
    int islandRow, int islandCol,
    int originalRow, int originalCol,
    int width
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
            zero_distance_queue.push({{rowToAdd, startCol}, {-1, -1}});
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
            zero_distance_queue.push({{rowToAdd, col}, {-1, -1}});
        }

        currentRow = trueRow;
    }
}


// Performing an exhaustive BFS to search through all pixels to ensure they have at least 1 adjacent neighbor
// Doing a BFS helps ensure additional pixels required stays at a minimum
// NOTE: Requires that original_binary_image[0] == 1
std::vector<uint8_t>
connectAllComponents(
    const std::vector<uint8_t> &original_binary_image,
    int height, int width
) {
    ScopedTimer timer("Connect all components");

    if (original_binary_image[0] != 1) {
        throw std::runtime_error("connectAllComponents: original_binary_image[0][0] != 1");
    }

    // Will be adding pixels/paths to connect components
    // Do not want to modify original_binary_image
    std::vector<uint8_t> final_binary_image = original_binary_image;

    // Tracker for each BFS iteration
    // Each connected component pixel considered to be at distance 0
    // As doing BFS, will be placing outside expanding pixels one beyond
    // Indicating that there exist pixels with at least a distance of 0
    std::vector<std::queue<QueueNode>> node_queues(1);

    std::vector<int> nodes_tracker(height * width, -1);

    // Adding the entire component found at location (0, 0) to the nodes_tracker and node_queues[0]
    addComponentToQueue(original_binary_image, nodes_tracker, node_queues[0], 0, 0, height, width);

    #ifdef EXTRA_PRINT 
        print2DVector("Fill Tracker Adding Border Component", nodes_tracker, height, width);
    #endif

    // Perform an exhaustive BFS to find the next nearest island component to connect to the final component
    while(true) {

        // Looking for queue of smallest unvisited distance
        int distance = 0;
        for (distance = 0; distance < (int)node_queues.size(); ++distance) {
            if (!node_queues[distance].empty()) break;
        }

        // We have gone through all pixels and have connected all components
        if (distance == (int)node_queues.size()) return final_binary_image;

        // Current node is either already connected or is not written to
        QueueNode nodeToCheck = node_queues[distance].front();
        node_queues[distance].pop();

        int currentRow = nodeToCheck.currentNode.first;
        int currentCol = nodeToCheck.currentNode.second;

        // Node has already been proceed and can be ignored
        if (distance > nodes_tracker[(currentRow*width) + currentCol]) continue;

        int originalRow = nodeToCheck.originalNode.first;
        int originalCol = nodeToCheck.originalNode.second;
        
        int adjacentDistance = distance + 1;

        // Check the neighbors
        for (int neighbor = 0; neighbor < 4; ++neighbor) {

            const int adjacentRow = currentRow + adjacentNodes[neighbor][0];
            const int adjacentCol = currentCol + adjacentNodes[neighbor][1];

            if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;

            const int idx = (adjacentRow*width) + adjacentCol;

            // If it is already being visited by someone closer, ignore
            const int node_track_val = nodes_tracker[idx];
            if ((node_track_val <= adjacentDistance) && (node_track_val != -1)) continue;
        
            // New island component found
            if (original_binary_image[idx]) {

                #ifdef EXTRA_PRINT
                    print2DVector("Round Tracked Locations Before Finding New Component", nodes_tracker, height, width);
                    std::cout << "New island component found at: (" << adjacentRow << ", " << adjacentCol << ")\n\n";
                #endif

                // Add the entire component found at given location to the globalSavedQueue
                addComponentToQueue(original_binary_image, nodes_tracker, node_queues[0], adjacentRow, adjacentCol, height, width);
                
                #ifdef EXTRA_PRINT
                    print2DVector("Fill Tracker Adding New Component", nodes_tracker, height, width);
                #endif
                
                // Add a path the connects the island component
                if (adjacentDistance == 1) {
                    // This can happen is we add new path pixels that are adjacent to islands
                    #ifdef EXTRA_PRINT
                        std::cout << "Starting location at: (" << currentRow << ", " << currentCol << ")\n\n";
                    #endif
                    addPathToQueue(final_binary_image, nodes_tracker, node_queues[0],
                                    adjacentRow, adjacentCol, currentRow, currentCol,
                                    width);
                } else {
                    #ifdef EXTRA_PRINT
                        std::cout << "Starting location at: (" << originalRow << ", " << originalCol << ")\n\n";
                    #endif
                    addPathToQueue(final_binary_image, nodes_tracker, node_queues[0],
                                    adjacentRow, adjacentCol, originalRow, originalCol,
                                    width);
                }

                #ifdef EXTRA_PRINT
                    print2DVector("Fill Tracker Adding New Component+Path", nodes_tracker, height, width);
                #endif

            } else {
                // Otherwise, just add blank space to expand
                
                if ((int)node_queues.size() == adjacentDistance) {
                    std::queue<QueueNode> emptyQueue;
                    node_queues.push_back(emptyQueue);
                }

                if (adjacentDistance == 1) {
                    node_queues[adjacentDistance].push({{adjacentRow, adjacentCol}, {currentRow, currentCol}});
                } else {
                    node_queues[adjacentDistance].push({{adjacentRow, adjacentCol}, {originalRow, originalCol}});
                }
                
                nodes_tracker[(adjacentRow*width) + adjacentCol] = adjacentDistance;
            }
        }
    }
}