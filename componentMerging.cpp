#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <stack>
#include <string>
#include <iomanip>
#include <chrono>
#include <tuple>
#include <algorithm>

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
    std::pair<int, int> currentNode;
    std::pair<int, int> originalNode;
};

struct Edge {
    int adjacentRow;
    int adjacentCol;
    int edgeCount;
};
using Graph = std::vector<std::vector<Edge>>;


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

    for (int row = 0; row < vec.size(); ++row) {
        for (int col = 0; col < vec[0].size(); ++col) {
            std::cout << std::setw(3) << static_cast<int>(vec[row][col]) << " ";
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


void
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


void
performBfsClustering(
    const std::vector<uint8_t> &final_binary_image,
    std::vector<uint32_t> &allNodesClustering,
    int startRow, int startCol, int clusterIdx,
    int height, int width, int cuttoffClustering
) {
    // ScopedTimer timer("performBfsClustering on cluster index: " + std::to_string(clusterIdx));

    std::queue<std::pair<int, int>> q;
    q.push({startRow, startCol});
    allNodesClustering[startRow*width + startCol] = clusterIdx;

    int nodesInCluster = 1;

    // Clustering nodes
    while(!q.empty()) {
        std::pair<int, int> currentNode = q.front();
        q.pop();

        // Check the neighbors
        for (int neighbor = 0; neighbor < 4; ++neighbor) {
        
            int adjacentRow = currentNode.first + adjacentNodes[neighbor][0];
            int adjacentCol = currentNode.second + adjacentNodes[neighbor][1];

            if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
            if (!final_binary_image[adjacentRow*width + adjacentCol]) continue;
            if (allNodesClustering[adjacentRow*width + adjacentCol]) continue;

            allNodesClustering[adjacentRow*width + adjacentCol] = clusterIdx;
            
            ++nodesInCluster;

            if (nodesInCluster >= cuttoffClustering) continue;
            q.push({adjacentRow, adjacentCol});
        }
    }
}


std::vector<std::pair<int, int>>
getClusterOddVerticies(
    const std::vector<uint32_t> &allNodesClustering,
    int startRow, int startCol, int clusterIdx,
    int height, int width
) {
    // ScopedTimer timer("getClusterOddVerticies on cluster index: " + std::to_string(clusterIdx));

    std::vector<std::pair<int, int>> oddVerticesList;

    // Determining odd vertices in cluster
    std::vector<uint32_t> verticesCheck(width * height);
    std::queue<std::pair<int, int>> q;

    q.push({startRow, startCol});
    verticesCheck[startRow*width + startCol] = 1;
    
    while(!q.empty()) {
        std::pair<int, int> currentNode = q.front();
        q.pop();
        
        int numNeighbors = 0;
        
        // Check the neighbors
        for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
            int adjacentRow = currentNode.first + adjacentNodes[neighbor][0];
            int adjacentCol = currentNode.second + adjacentNodes[neighbor][1];
            
            if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
            if (allNodesClustering[adjacentRow*width + adjacentCol] != clusterIdx) continue;

            ++numNeighbors;

            if (verticesCheck[adjacentRow*width + adjacentCol]) continue;

            verticesCheck[adjacentRow*width + adjacentCol] = 1;
            q.push({adjacentRow, adjacentCol});
        }

        if (numNeighbors % 2) oddVerticesList.push_back({currentNode.first, currentNode.second});
    }

    return oddVerticesList;
}


std::vector<std::vector<int>>
getDistanceMatrix (
    const std::vector<uint8_t> &final_binary_image,
    const std::vector<uint32_t> &allNodesClustering,
    const std::vector<std::pair<int, int>> &oddVerticesList,
    int startRow, int startCol, int clusterIdx,
    int height, int width
) {
    // ScopedTimer timer("getDistanceMatrix on cluster index: " + std::to_string(clusterIdx));

    std::vector<std::vector<int>> distanceMatrix(oddVerticesList.size(), std::vector<int>(oddVerticesList.size()));

    if (oddVerticesList.empty()) return distanceMatrix;

    // Converting list --> matrix for easy finding of pairs
    std::vector<uint32_t> oddVerticesMatrix(height * width);
    for (int i = 0; i < oddVerticesList.size(); ++i) {
        const std::pair<int, int> &p = oddVerticesList[i];
        oddVerticesMatrix[p.first*width + p.second] = i+1;
    }

    std::vector<uint32_t> verticesChecked(width * height, 0);

    for (int i = 0; i < oddVerticesList.size()-1; ++i) {
        
        uint32_t pairDistancesLeftToFind = oddVerticesList.size()-i-1;

        std::queue<std::tuple<int, int, int>> distanceQueue;
        ++verticesChecked[oddVerticesList[i].first*width + oddVerticesList[i].second];
            
        // Perform BFS to find distance
        distanceQueue.push({0, oddVerticesList[i].first, oddVerticesList[i].second});

        while(pairDistancesLeftToFind > 0) {
            std::tuple<int, int, int> currentNode = distanceQueue.front();
            distanceQueue.pop();

            // Check the neighbors
            for (int neighbor = 0; neighbor < 4; ++neighbor) {
                
                int adjacentRow = std::get<1>(currentNode) + adjacentNodes[neighbor][0];
                int adjacentCol = std::get<2>(currentNode) + adjacentNodes[neighbor][1];
                
                if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
                if (allNodesClustering[adjacentRow*width + adjacentCol] != clusterIdx) continue;
                if (verticesChecked[adjacentRow*width + adjacentCol] == i+1) continue;

                int adjacentDistance = std::get<0>(currentNode)+1;

                if (oddVerticesMatrix[adjacentRow*width + adjacentCol] > i) {
                    int otherOddVertex = oddVerticesMatrix[adjacentRow*width + adjacentCol]-1;
                    distanceMatrix[i][otherOddVertex] = adjacentDistance;
                    distanceMatrix[otherOddVertex][i] = adjacentDistance;
                    --pairDistancesLeftToFind;
                }

                ++verticesChecked[adjacentRow*width + adjacentCol];
                distanceQueue.push({adjacentDistance, adjacentRow, adjacentCol});
            }
        }
    }

    return distanceMatrix;
}


std::vector<std::pair<int, int>>
findOddPairings (
    const std::vector<std::pair<int, int>> &oddVerticesList,
    const std::vector<std::vector<int>> &distanceMatrix
) {
    // ScopedTimer timer("findOddPairings");

    // Perform greedy first pass on finding shortest distances
    std::vector<std::tuple<int, int, int>> distanceTrackerTuple;

    for (int i = 0; i < distanceMatrix.size(); ++i) {
        for (int j = i+1; j < distanceMatrix.size(); ++j) {
            distanceTrackerTuple.push_back({distanceMatrix[i][j], i, j});
        }
    }
    std::sort(distanceTrackerTuple.begin(), distanceTrackerTuple.end());

    std::vector<std::pair<int, int>> oddPairings(distanceMatrix.size()/2);
    std::vector<uint8_t> oddVertexUsed(distanceMatrix.size());

    int pairIdx = 0;

    for (const std::tuple<int, int, int> &closestDistance : distanceTrackerTuple) {
        if (!oddVertexUsed[std::get<1>(closestDistance)] && !oddVertexUsed[std::get<2>(closestDistance)]) {
            oddVertexUsed[std::get<1>(closestDistance)] = 1;
            oddVertexUsed[std::get<2>(closestDistance)] = 1;
            oddPairings[pairIdx++] = {std::get<1>(closestDistance), std::get<2>(closestDistance)};

            if (pairIdx == oddPairings.size()) break;
        }
    }

    if (EXTRA_PRINT) {
        std::cout << "Odd Vertex Pairings AFTER GREEDY (Distance):\n";

        int totalDistance = 0;

        for (int i = 0; i < oddPairings.size(); ++i) {
            std::cout << "(" << oddVerticesList[oddPairings[i].first].first << ", " << oddVerticesList[oddPairings[i].first].second << ") --> "
                    << "(" << oddVerticesList[oddPairings[i].second].first << ", " << oddVerticesList[oddPairings[i].second].second << ")"
                    << "   Distance: " << distanceMatrix[oddPairings[i].first][oddPairings[i].second] << "\n";

            totalDistance += distanceMatrix[oddPairings[i].first][oddPairings[i].second];
        }
        std::cout << "Total Added Distance: " << totalDistance << "\n\n";
    }

    uint8_t hasImproved = 1;

    // Performing 2-opt (2-edge optimization) improvement
    while (hasImproved) {

        hasImproved = 0;

        for (int i = 0; i < oddPairings.size(); ++i) {
            for (int j = i+1; j < oddPairings.size(); ++j) {
                
                int a = oddPairings[i].first; 
                int b = oddPairings[i].second;
                
                int c = oddPairings[j].first; 
                int d = oddPairings[j].second;
                
                
                int currentPairDistance = distanceMatrix[a][b] + distanceMatrix[c][d];
                
                int newPairDistance = distanceMatrix[a][c] + distanceMatrix[b][d];
                
                if (newPairDistance < currentPairDistance) {
                    oddPairings[i] = {a, c};
                    oddPairings[j] = {b, d};
                    currentPairDistance = newPairDistance;
                    hasImproved = 1;
                    break;
                }
                
                newPairDistance = distanceMatrix[a][d] + distanceMatrix[b][c];
                
                if (newPairDistance < currentPairDistance) {
                    oddPairings[i] = {a, d};
                    oddPairings[j] = {b, c};
                    hasImproved = 1;
                    break;
                }
            }
        }
    }

    return oddPairings;
}


void
updateGraphWithShortestPath(
    const std::vector<uint32_t> &allNodesClustering,
    Graph &allEdges,
    int clusterIdx,
    int row1, int col1, int row2, int col2,
    int height, int width
) {
    std::queue<int> q;
    q.push(row1*width + col1);
    
    std::vector<int> prevNodes(height * width, -1);
    prevNodes[row1*width + col1] = row1*width + col1;

    if (EXTRA_PRINT) std::cout << "(" << row2 << ", " << col2 << ") --> ";

    while(true) {
        int currentNode = q.front();
        q.pop();

        int currentRow = currentNode / width;
        int currentCol = currentNode % width;

        for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
            int adjacentRow = currentRow + adjacentNodes[neighbor][0];
            int adjacentCol = currentCol + adjacentNodes[neighbor][1];

            if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
            if (allNodesClustering[adjacentRow*width + adjacentCol] != clusterIdx) continue;
            if (prevNodes[adjacentRow*width + adjacentCol] != -1) continue;

            prevNodes[adjacentRow*width + adjacentCol] = currentRow*width + currentCol;

            if ((adjacentRow == row2) && (adjacentCol == col2)) {
                currentRow = row2;
                currentCol = col2;
                
                while((currentRow != row1) || (currentCol != col1)) {
                    int prevNode = prevNodes[currentRow*width + currentCol];

                    int prevRow = prevNode / width;
                    int prevCol = prevNode % width;

                    if (EXTRA_PRINT) std::cout << "(" << prevRow << ", " << prevCol << ") --> ";

                    for (Edge &edge : allEdges[currentRow*width + currentCol]) {
                        if ((edge.adjacentRow != prevRow) || (edge.adjacentCol != prevCol)) continue;
                        ++edge.edgeCount;
                        break;
                    }

                    for (Edge &edge : allEdges[prevRow*width + prevCol]) {
                        if ((edge.adjacentRow != currentRow) || (edge.adjacentCol != currentCol)) continue;
                        ++edge.edgeCount;
                        break;
                    }

                    currentRow = prevRow;
                    currentCol = prevCol;
                }

                return;
            }
            
            q.push(adjacentRow*width + adjacentCol);
        }
    }
}

void
populateGraph(
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    int height, int width
) {
    ScopedTimer timer("Populating Initial Graph");

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {

            if (!final_binary_image[row*width + col]) continue;

            for (int neighbor = 0; neighbor < 4; ++neighbor) {
                    
                int adjacentRow = row + adjacentNodes[neighbor][0];
                int adjacentCol = col + adjacentNodes[neighbor][1];

                if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
                if (!final_binary_image[adjacentRow*width + adjacentCol]) continue;

                allEdges[row*width + col].push_back({adjacentRow, adjacentCol, 1});
            }
        }
    }
}


void
updateAllShortestPaths(
    Graph &allEdges,
    const std::vector<std::pair<int, int>> &oddPairings,
    const std::vector<std::pair<int, int>> &oddVerticesList,
    const std::vector<uint32_t> &allNodesClustering,
    int clusterIdx,
    int height, int width
) {
    // ScopedTimer timer("updateAllShortestPaths on cluster index: " + std::to_string(clusterIdx));;

    if (EXTRA_PRINT) std::cout << "Odd Vertex Pairings (Distance):\n";
    // int totalDistance = 0;
    
    for (const std::pair<int, int> &pair : oddPairings) {

        updateGraphWithShortestPath(
            allNodesClustering,
            allEdges,
            clusterIdx,
            oddVerticesList[pair.first].first, oddVerticesList[pair.first].second,
            oddVerticesList[pair.second].first, oddVerticesList[pair.second].second,
            height, width
        );

        // if (EXTRA_PRINT) std::cout << "   Distance: " << distanceMatrix[pair.first][pair.second] << "\n";
        
        // totalDistance += distanceMatrix[pair.first][pair.second];
    }

    // if (BASIC_PRINT) std::cout << "Total Added Distance: " << totalDistance << "\n\n";
}


void
performBfsClusteringAndEulerize(
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    std::vector<uint32_t> &allNodesClustering,
    int startRow, int startCol, int clusterIdx, int cutoffClustering,
    int height, int width
) {
    // ScopedTimer timer("performBfsClusteringAndEulerize on cluster index: " + std::to_string(clusterIdx));

    if (EXTRA_PRINT) std::cout << "Performing Clustering for cluster index: " << clusterIdx << "\n\n";

    performBfsClustering(final_binary_image, allNodesClustering, startRow, startCol, clusterIdx, height, width, cutoffClustering);

    if (EXTRA_PRINT) print2DVector("All Nodes Clustering After New Cluster", allNodesClustering, height, width);

    std::vector<std::pair<int, int>> oddVerticesList = getClusterOddVerticies(allNodesClustering, startRow, startCol, clusterIdx, height, width);

    if (EXTRA_PRINT) {
        std::cout << "Odd Vertex Locations:\n";
        
        for (int i = 0; i < oddVerticesList.size(); ++i) {
            std::cout << "(" << oddVerticesList[i].first << ", " << oddVerticesList[i].second << "), ";
        }
        std::cout << "\n\n";
    }

    // Produce distance matrix for all odd vertices
    std::vector<std::vector<int>> distanceMatrix = getDistanceMatrix(
        final_binary_image, allNodesClustering, oddVerticesList, startRow, startCol, clusterIdx, height, width);

    if (EXTRA_PRINT) print2DVector("Distance Matrix", distanceMatrix);

    // Perform Greedy + 2-opt to find near-best odd pairing
    std::vector<std::pair<int, int>> oddPairings = findOddPairings(oddVerticesList, distanceMatrix);

    // Update Graph to have double edges where needed
    updateAllShortestPaths(allEdges, oddPairings, oddVerticesList, allNodesClustering, clusterIdx, height, width);

}


void
generateAndEulerizeClusters (
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    std::vector<uint32_t> &allNodesClustering,
    int cutoffClustering,
    int height, int width
) {
    ScopedTimer timer("generateAndEulerizeClusters");

    int clusterIdx = 1;
    int totalPixels = 0;

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {

            totalPixels += final_binary_image[row*width + col];

            if (!final_binary_image[row*width + col]) continue;
            if (allNodesClustering[row*width + col] == 0) {
                performBfsClusteringAndEulerize(final_binary_image, allEdges, allNodesClustering, row, col, clusterIdx, cutoffClustering, height, width);
                ++clusterIdx;
            }
        }
    }

    if (BASIC_PRINT) std::cout << "TOTAL PIXEL COUNT: " << totalPixels << "\n\n";
}


void
connectClustersOnGraph (
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    const std::vector<uint32_t> &allNodesClustering,
    int height, int width
) {
    ScopedTimer timer("connectClustersOnGraph");
    
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {

            if (!final_binary_image[row*width + col]) continue;

            for (int neighbor = 0; neighbor < 4; ++neighbor) {
                    
                int adjacentRow = row + adjacentNodes[neighbor][0];
                int adjacentCol = col + adjacentNodes[neighbor][1];

                if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
                if (!final_binary_image[adjacentRow*width + adjacentCol]) continue;
                if (allNodesClustering[row*width + col] == allNodesClustering[adjacentRow*width + adjacentCol]) continue;

                for (Edge &edge : allEdges[row*width + col]) {
                    if ((edge.adjacentRow != adjacentRow) || (edge.adjacentCol != adjacentCol)) continue;
                    ++edge.edgeCount;
                    break;
                }
            }
        }
    }
}


void
eulerizeGraph (
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    int cutoffClustering,
    int height, int width
) {
    ScopedTimer timer("eulerizeGraph");

    std::vector<uint32_t> allNodesClustering(height * width);
    generateAndEulerizeClusters(final_binary_image, allEdges, allNodesClustering, cutoffClustering, height, width);

    // Double edge count the neighboring edges to other components
    connectClustersOnGraph(final_binary_image, allEdges, allNodesClustering, height, width);
}


std::vector<std::pair<int, int>> 
findEulerCircuit(
    Graph &g, 
    int startRow, int startCol,
    int height, int width
) {

    ScopedTimer timer("Find Euler Circuit");

    std::vector<std::pair<int, int>> circuit;
    std::stack<std::pair<int, int>> st;

    st.push({startRow, startCol});

    while (!st.empty()) {
        std::pair<int, int> u = st.top();

        bool moved = false;

        for (Edge &e : g[u.first*width + u.second]) {
            
            if (e.edgeCount > 0) {
                std::pair<int, int> v = {e.adjacentRow, e.adjacentCol};

                // consume edge u -> v
                e.edgeCount--;

                // consume edge v -> u
                for (Edge &rev : g[v.first*width + v.second]) {
                    if ((rev.adjacentRow == u.first) && (rev.adjacentCol == u.second)) {
                        rev.edgeCount--;
                        break;
                    }
                }

                st.push(v);
                moved = true;
                break;
            }
        }

        if (!moved) {
            circuit.push_back(u);
            st.pop();
        }
    }

    std::reverse(circuit.begin(), circuit.end());

    for (auto &adj : g)
        for (auto &e : adj)
            assert(e.edgeCount == 0);

    return circuit;
}


void
performCPP (
    const std::vector<uint8_t> &final_binary_image,
    const std::string &output_steps,
    int cutoffClustering,
    int height, int width
) {
    ScopedTimer timer("Perform Chinese Postman Problem");

    Graph allEdges(height * width);
    populateGraph(final_binary_image, allEdges, height, width);

    eulerizeGraph(final_binary_image, allEdges, cutoffClustering, height, width);

    // Perform Hierholzer's algorithm to find Eulerian Circuit
    std::vector<std::pair<int, int>> fullCircuit = findEulerCircuit(allEdges, 0, 0, height, width);

    std::cout << "Number of steps total: " << fullCircuit.size() << "\n\n";
}


int 
main(int argc, char **argv) 
{
    ScopedTimer program_timer("TOTAL PROGRAM");

    int height, width;
    std::string output_image, output_steps;

    std::vector<uint8_t> binary_image;

    // {executable}.out {height} {width} {output_image} {output_steps} {BASIC_PRINT} {EXTRA_PRINT}
    if (argc == 7) {

        height = std::stoi(argv[1]);
        width = std::stoi(argv[2]);
        output_image = argv[3];
        output_steps = argv[4];

        BASIC_PRINT = std::stoi(argv[5]);
        EXTRA_PRINT = std::stoi(argv[6]);
        
        binary_image.resize(height * width);

        // binary_image = {
        //     1,   0,   0,   0,   0,   1,   0, 
        //     0,   1,   1,   1,   0,   0,   0, 
        //     0,   1,   0,   0,   0,   0,   0, 
        //     0,   0,   0,   0,   0,   0,   0, 
        //     0,   0,   0,   0,   0,   0,   0, 
        //     1,   1,   0,   1,   1,   0,   0, 
        //     0,   1,   0,   0,   0,   0,   0
        // };
        
        generateRandomImage(binary_image, height, width);

    } 
    // {executable}.out {input_filepath} {output_filepath} {output_steps} {BASIC_PRINT} {EXTRA_PRINT}
    else if (argc == 6) {

        int channels;
        const char * input_filepath = argv[1];
        output_image = argv[2];
        output_steps = argv[3];
        BASIC_PRINT = std::stoi(argv[4]);
        EXTRA_PRINT = std::stoi(argv[5]);

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

    // Finding reasonable shortest path to produce image
    // Solving for Chinese Postman Problem
    performCPP(final_binary_image, output_steps, 100, height, width);

    // Outputting final image to bmp file
    for (int i = 0; i < height * width; ++i) {
        final_binary_image[i] = (final_binary_image[i]) ? 0 : 255;
    }

    // Writing to output filepath
    if (!stbi_write_bmp(output_image.c_str(), width, height, 1, final_binary_image.data())) {
        std::cerr << "Failed to write BMP\n";
        return 1;
    }

    return 0;
}