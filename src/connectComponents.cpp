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


static bool inline
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


static void
addComponentToQueue(
    const std::vector<uint8_t> &binary_image,
    std::vector<int> &nodes_tracker,
    std::queue<QueueNode> &zeroDistanceQueue,
    int start_row, int start_col,
    int height, int width
) {

    zeroDistanceQueue.push({{start_row, start_col}, {-1, -1}});
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

            zeroDistanceQueue.push({{adjacentRow, adjacentCol}, {-1, -1}});
            nodes_tracker[(adjacentRow*width) + adjacentCol] = 0;
            
            nodesToCheck.push({adjacentRow, adjacentCol});
        }
    }
}


static void
addPathToQueue(
    std::vector<uint8_t> &binary_image,
    std::vector<int> &nodes_tracker,
    std::queue<QueueNode> &zeroDistanceQueue,
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
            zeroDistanceQueue.push({{rowToAdd, startCol}, {-1, -1}});
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
            zeroDistanceQueue.push({{rowToAdd, col}, {-1, -1}});
        }

        currentRow = trueRow;
    }
}


std::vector<uint8_t>
connectAllComponents(
    const std::vector<uint8_t> &original_binary_image,
    int height, int width
) {
    ScopedTimer timer("Connect all components");

    std::vector<uint8_t> final_binary_image = original_binary_image;

    std::vector<std::queue<QueueNode>> nodeQueues(1);

    std::vector<int> nodes_tracker(height * width, -1);

    // Adding the entire component found at location (0, 0) to the globalSavedQueue
    addComponentToQueue(original_binary_image, nodes_tracker, nodeQueues[0], 0, 0, height, width);

    if (EXTRA_PRINT) print2DVector("Fill Tracker Adding Border Component", nodes_tracker, height, width);

    // Perform an exhaustive BFS to find the next nearest island component to connect to the final component
    while(true) {

        int distance = 0;
        for (distance = 0; distance < nodeQueues.size(); ++distance) {
            if (nodeQueues[distance].size() > 0) break;
        }

        if (distance == nodeQueues.size()) return final_binary_image;

        // Current node is either already connected or is not written to
        QueueNode nodeToCheck = nodeQueues[distance].front();
        nodeQueues[distance].pop();

        int currentRow = nodeToCheck.currentNode.first;
        int currentCol = nodeToCheck.currentNode.second;

        // Node has already been proceed and can be ignored
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
                addComponentToQueue(original_binary_image, nodes_tracker, nodeQueues[0], adjacentRow, adjacentCol, height, width);
                
                if (EXTRA_PRINT) {
                    print2DVector("Fill Tracker Adding New Component", nodes_tracker, height, width);
                }
                
                // Add a path the connects the island component
                if (adjacentDistance == 1) {
                    // This can happen is we add new path pixels that are adjacent to islands
                    if (EXTRA_PRINT) std::cout << "Starting location at: (" << currentRow << ", " << currentCol << ")\n\n";
                    addPathToQueue(final_binary_image, nodes_tracker, nodeQueues[0],
                                    adjacentRow, adjacentCol, currentRow, currentCol,
                                    height, width);
                } else {
                    if (EXTRA_PRINT) std::cout << "Starting location at: (" << originalRow << ", " << originalCol << ")\n\n";
                    addPathToQueue(final_binary_image, nodes_tracker, nodeQueues[0],
                                    adjacentRow, adjacentCol, originalRow, originalCol,
                                    height, width);
                }

                if (EXTRA_PRINT) print2DVector("Fill Tracker Adding New Component+Path", nodes_tracker, height, width);

            } else {
                // Otherwise, just add blank space to expand
                
                if (nodeQueues.size() == adjacentDistance) {
                    std::queue<QueueNode> emptyQueue;
                    nodeQueues.push_back(emptyQueue);
                }

                if (adjacentDistance == 1) {
                    nodeQueues[adjacentDistance].push({{adjacentRow, adjacentCol}, {currentRow, currentCol}});
                } else {
                    nodeQueues[adjacentDistance].push({{adjacentRow, adjacentCol}, {originalRow, originalCol}});
                }
                
                nodes_tracker[(adjacentRow*width) + adjacentCol] = adjacentDistance;
            }
        }
    }
}