#include <vector>
#include <cstdint>
#include <iostream>
#include <queue>
#include <algorithm>
#include <stack>
#include <array>
#include <cassert>

#include "../include/debugHelp.hpp"
#include "../include/generatePath.hpp"

struct DistanceNode {
    uint16_t distance;
    uint32_t idx;
};

// Vector of all pixels in image
// Each pixel has (at most) 4 neighbors.
// Can know exactly which index to use through adjacentNodes
using Graph = std::vector<std::array<uint32_t, 4>>;


static void
performBfsClustering(
    const std::vector<uint8_t> &final_binary_image,
    std::vector<uint32_t> &allNodesClustering,
    int startRow, int startCol, uint32_t clusterIdx,
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


static std::vector<uint32_t>
getClusterOddVerticies(
    const std::vector<uint32_t> &allNodesClustering,
    std::vector<uint32_t> &verticesCheck, uint32_t visitIdx,
    int startRow, int startCol, uint32_t clusterIdx,
    int height, int width
) {
    // ScopedTimer timer("getClusterOddVerticies on cluster index: " + std::to_string(clusterIdx));

    std::vector<uint32_t> oddVerticesList;

    // Determining odd vertices in cluster
    std::queue<uint32_t> q;

    q.push(startRow*width + startCol);
    verticesCheck[startRow*width + startCol] = visitIdx;
    
    while(!q.empty()) {
        uint32_t currentNode = q.front();
        q.pop();

        int currentRow = currentNode / width;
        int currentCol = currentNode % width;
        
        int numNeighbors = 0;
        
        // Check the neighbors
        for (int neighbor = 0; neighbor < 4; ++neighbor) {
            
            int adjacentRow = currentRow + adjacentNodes[neighbor][0];
            int adjacentCol = currentCol + adjacentNodes[neighbor][1];
            
            if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
            if (allNodesClustering[adjacentRow*width + adjacentCol] != clusterIdx) continue;

            ++numNeighbors;

            if (verticesCheck[adjacentRow*width + adjacentCol] == visitIdx) continue;

            verticesCheck[adjacentRow*width + adjacentCol] = visitIdx;
            q.push(adjacentRow*width + adjacentCol);
        }

        if (numNeighbors % 2) oddVerticesList.push_back(currentRow*width + currentCol);
    }

    return oddVerticesList;
}


static std::vector<std::vector<int>>
getDistanceMatrix (
    const std::vector<uint32_t> &allNodesClustering,
    const std::vector<uint32_t> &oddVerticesList,
    std::vector<uint32_t> &verticesCheck, uint32_t &visitIdx,
    uint32_t clusterIdx, int height, int width
) {
    // ScopedTimer timer("getDistanceMatrix on cluster index: " + std::to_string(clusterIdx));

    std::vector<std::vector<int>> distanceMatrix(oddVerticesList.size(), std::vector<int>(oddVerticesList.size()));

    if (oddVerticesList.empty()) return distanceMatrix;

    // Converting list --> matrix for easy finding of pairs
    std::vector<uint32_t> oddVerticesMatrix(height * width);
    for (size_t i = 0; i < oddVerticesList.size(); ++i) {
        const uint32_t &p = oddVerticesList[i];
        oddVerticesMatrix[p] = i+1;
    }

    for (size_t i = 0; i < oddVerticesList.size()-1; ++i) {

        uint32_t pairDistancesLeftToFind = oddVerticesList.size()-i-1;
        
        std::queue<DistanceNode> distanceQueue;
        verticesCheck[oddVerticesList[i]] = visitIdx;
        
        // Perform BFS to find distance
        distanceQueue.push({0, oddVerticesList[i]});
        
        while(pairDistancesLeftToFind > 0) {
            DistanceNode currentNode = distanceQueue.front();
            distanceQueue.pop();

            int row = currentNode.idx / width;
            int col = currentNode.idx % width;
            
            // Check the neighbors
            for (int neighbor = 0; neighbor < 4; ++neighbor) {
                
                int adjacentRow = row + adjacentNodes[neighbor][0];
                int adjacentCol = col + adjacentNodes[neighbor][1];
                
                if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;

                uint32_t adjacentNode = adjacentRow*width + adjacentCol;

                if (allNodesClustering[adjacentNode] != clusterIdx) continue;
                if (verticesCheck[adjacentNode] == visitIdx) continue;
                
                uint16_t adjacentDistance = currentNode.distance +1;
                
                if (oddVerticesMatrix[adjacentNode] > i) {
                    int otherOddVertex = oddVerticesMatrix[adjacentNode]-1;
                    distanceMatrix[i][otherOddVertex] = adjacentDistance;
                    distanceMatrix[otherOddVertex][i] = adjacentDistance;
                    --pairDistancesLeftToFind;
                }

                verticesCheck[adjacentNode] = visitIdx;
                distanceQueue.push({adjacentDistance, adjacentNode});
            }
        }

        ++visitIdx;
    }

    return distanceMatrix;
}


static std::vector<std::pair<int, int>>
findOddPairings (
    const std::vector<uint32_t> &oddVerticesList,
    const std::vector<std::vector<int>> &distanceMatrix,
    int width
) {
    // ScopedTimer timer("findOddPairings");

    // Perform greedy first pass on finding shortest distances
    std::vector<std::tuple<int, int, int>> distanceTrackerTuple;

    for (int i = 0; i < (int)distanceMatrix.size(); ++i) {
        for (int j = i+1; j < (int)distanceMatrix.size(); ++j) {
            distanceTrackerTuple.push_back({distanceMatrix[i][j], i, j});
        }
    }
    std::sort(distanceTrackerTuple.begin(), distanceTrackerTuple.end());

    std::vector<std::pair<int, int>> oddPairings(distanceMatrix.size()/2);
    std::vector<uint8_t> oddVertexUsed(distanceMatrix.size());

    size_t pairIdx = 0;

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

        for (size_t i = 0; i < oddPairings.size(); ++i) {
            std::cout << "(" << oddVerticesList[oddPairings[i].first]/width << ", " << oddVerticesList[oddPairings[i].first]%width << ") --> "
                    << "(" << oddVerticesList[oddPairings[i].second]/width << ", " << oddVerticesList[oddPairings[i].second]%width << ")"
                    << "   Distance: " << distanceMatrix[oddPairings[i].first][oddPairings[i].second] << "\n";

            totalDistance += distanceMatrix[oddPairings[i].first][oddPairings[i].second];
        }
        std::cout << "Total Added Distance: " << totalDistance << "\n\n";
    }

    uint8_t hasImproved = 1;

    // Performing 2-opt (2-edge optimization) improvement
    while (hasImproved) {

        hasImproved = 0;

        for (size_t i = 0; i < oddPairings.size(); ++i) {
            for (size_t j = i+1; j < oddPairings.size(); ++j) {
                
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


static void
populateGraph(
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    int height, int width
) {
    ScopedTimer timer("Populating Initial Graph");

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {

            if (!final_binary_image[row*width + col]) continue;

            int currentNode = row*width + col;

            for (int neighbor = 0; neighbor < 4; ++neighbor) {
                    
                int adjacentRow = row + adjacentNodes[neighbor][0];
                int adjacentCol = col + adjacentNodes[neighbor][1];

                if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;
                if (!final_binary_image[adjacentRow*width + adjacentCol]) continue;

                allEdges[currentNode][neighbor] = 1;
            }
        }
    }
}


static void
updateGraphWithShortestPath(
    const std::vector<uint32_t> &allNodesClustering,
    std::vector<uint8_t> &prevNodes, std::vector<uint32_t> &visitTracker, uint32_t visitId,
    Graph &allEdges,
    uint32_t clusterIdx,
    uint32_t node1, uint32_t node2,
    int height, int width
) {
    std::queue<uint32_t> q;
    q.push(node1);
    
    visitTracker[node1] = visitId;

    if (EXTRA_PRINT) std::cout << "(" << node2/width << ", " << node2%width << ") --> ";

    while(true) {
        uint32_t currentNode = q.front();
        q.pop();

        int currentRow = currentNode / width;
        int currentCol = currentNode % width;

        for (uint8_t neighbor = 0; neighbor < 4; ++neighbor) {
            
            int adjacentRow = currentRow + adjacentNodes[neighbor][0];
            int adjacentCol = currentCol + adjacentNodes[neighbor][1];
            
            if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;

            uint32_t adjacentNode = adjacentRow*width + adjacentCol;

            if (allNodesClustering[adjacentNode] != clusterIdx) continue;
            if (visitTracker[adjacentNode] == visitId) continue;

            prevNodes[adjacentNode] = neighbor;   // Indicating that adjacentNode = currentNode + neighbor
            visitTracker[adjacentNode] = visitId;

            // If we found the other node we need to reach
            if (adjacentNode == node2) {
                
                // Walk backwards through looking at the neighbors
                currentNode = node2;
                
                while(currentNode != node1) {

                    // currentNode = prevNode + prevNeighbor
                    // --> prevNode = currentNode - prevNeighbor
                    uint8_t prevNeighbor = prevNodes[currentNode];
                    uint32_t prevNode = currentNode - (adjacentNodes[prevNeighbor][0]*width + adjacentNodes[prevNeighbor][1]);

                    if (EXTRA_PRINT) std::cout << "(" << prevNode/width << ", " << prevNode%width << ") --> ";

                    // 0 <-> 1  and 2 <-> 3 (Must flip 0 bit through XOR)
                    ++allEdges[prevNode][prevNeighbor];
                    ++allEdges[currentNode][prevNeighbor ^ 0x1];

                    currentNode = prevNode;
                }

                return;
            }
            
            q.push(adjacentRow*width + adjacentCol);
        }
    }
}


static void
updateAllShortestPaths(
    Graph &allEdges,
    const std::vector<std::pair<int, int>> &oddPairings,
    const std::vector<uint32_t> &oddVerticesList,
    const std::vector<uint32_t> &allNodesClustering,
    std::vector<uint32_t> &visitTracker, uint32_t &visitId,
    uint32_t clusterIdx,
    int height, int width
) {
    // ScopedTimer timer("updateAllShortestPaths on cluster index: " + std::to_string(clusterIdx));;

    if (EXTRA_PRINT) std::cout << "Odd Vertex Pairings (Distance):\n";
    // int totalDistance = 0;

    std::vector<uint8_t> prevNodes(height * width);
    
    for (const std::pair<int, int> &pair : oddPairings) {

        updateGraphWithShortestPath(
            allNodesClustering,
            prevNodes, visitTracker, visitId,
            allEdges,
            clusterIdx,
            oddVerticesList[pair.first], oddVerticesList[pair.second],
            height, width
        );
        ++visitId;

        // if (EXTRA_PRINT) std::cout << "   Distance: " << distanceMatrix[pair.first][pair.second] << "\n";
        
        // totalDistance += distanceMatrix[pair.first][pair.second];
    }

    // if (BASIC_PRINT) std::cout << "Total Added Distance: " << totalDistance << "\n\n";
}

static void
performBfsClusteringAndEulerize(
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    std::vector<uint32_t> &allNodesClustering,
    int startRow, int startCol, uint32_t clusterIdx, int cutoffClustering,
    int height, int width
) {
    // ScopedTimer timer("performBfsClusteringAndEulerize on cluster index: " + std::to_string(clusterIdx));

    if (EXTRA_PRINT) std::cout << "Performing Clustering for cluster index: " << clusterIdx << "\n\n";

    performBfsClustering(final_binary_image, allNodesClustering, startRow, startCol, clusterIdx, height, width, cutoffClustering);

    if (EXTRA_PRINT) print2DVector("All Nodes Clustering After New Cluster", allNodesClustering, height, width);

    std::vector<uint32_t> verticesCheck(height * width); 
    uint32_t visitIdx = 1;

    std::vector<uint32_t> oddVerticesList = getClusterOddVerticies(allNodesClustering, verticesCheck, visitIdx, startRow, startCol, clusterIdx, height, width);
    ++visitIdx;

    if (EXTRA_PRINT) {
        std::cout << "Odd Vertex Locations:\n";
        
        for (size_t i = 0; i < oddVerticesList.size(); ++i) {
            std::cout << "(" << oddVerticesList[i]/width << ", " << oddVerticesList[i]%width << "), ";
        }
        std::cout << "\n\n";
    }

    // Produce distance matrix for all odd vertices
    std::vector<std::vector<int>> distanceMatrix = getDistanceMatrix(
        allNodesClustering, oddVerticesList, 
        verticesCheck, visitIdx,
        clusterIdx, height, width);

    if (EXTRA_PRINT) print2DVector("Distance Matrix", distanceMatrix);

    // Perform Greedy + 2-opt to find near-best odd pairing
    std::vector<std::pair<int, int>> oddPairings = findOddPairings(oddVerticesList, distanceMatrix, width);

    // Update Graph to have double edges where needed
    updateAllShortestPaths(
        allEdges, oddPairings, oddVerticesList, allNodesClustering, 
        verticesCheck, visitIdx,
        clusterIdx, height, width);

    // std::cout << "VisitIdx: " << visitIdx << "\n";

}


static void
generateAndEulerizeClusters (
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    std::vector<uint32_t> &allNodesClustering,
    int cutoffClustering,
    int height, int width
) {
    ScopedTimer timer("generateAndEulerizeClusters");

    uint32_t clusterIdx = 1;
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


static void
connectClustersOnGraph (
    const std::vector<uint8_t> &final_binary_image,
    Graph &allEdges,
    const std::vector<uint32_t> &allNodesClustering,
    int height, int width
) {
    ScopedTimer timer("connectClustersOnGraph");
    
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {

            int currentNode = row*width + col;

            if (!final_binary_image[currentNode]) continue;

            for (int neighbor = 0; neighbor < 4; ++neighbor) {
                    
                int adjacentRow = row + adjacentNodes[neighbor][0];
                int adjacentCol = col + adjacentNodes[neighbor][1];
                
                if ((adjacentRow < 0) || (adjacentCol < 0) || (adjacentRow >= height) || (adjacentCol >= width)) continue;

                int adjacentNode = adjacentRow*width + adjacentCol;

                if (!final_binary_image[adjacentNode]) continue;
                if (allNodesClustering[currentNode] == allNodesClustering[adjacentNode]) continue;

                ++allEdges[currentNode][neighbor];
            }
        }
    }
}


static void
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

struct StackEntry {
    uint32_t node;
    int prevDir; // -1 if none
};

static std::vector<uint32_t> 
findEulerCircuit(
    Graph &allEdges, 
    uint32_t startNode,
    int width
) {
    ScopedTimer timer("Find Euler Circuit");

    // Prioritize straigh lines, and de-prioritize reverse polarity
    std::array<std::array<int, 4>, 4> dirPriority;
    dirPriority[UP] = {UP, LEFT, RIGHT, DOWN};
    dirPriority[DOWN] = {DOWN, LEFT, RIGHT, UP};
    dirPriority[LEFT] = {LEFT, UP, DOWN, RIGHT};
    dirPriority[RIGHT] = {RIGHT, UP, DOWN, LEFT};

    std::vector<uint32_t> circuit;
    std::stack<StackEntry> st;

    st.push({startNode, -1});

    while (!st.empty()) {
        auto [u, prevDir] = st.top();
        bool moved = false;

        // If we have a previous direction, use prioritized order
        if (prevDir != -1) {
            for (int k = 0; k < 4; ++k) {
                int d = dirPriority[prevDir][k];
                uint32_t &edgeCount = allEdges[u][d];

                if (edgeCount > 0) {
                    uint32_t v = u + (adjacentNodes[d][0] * width +
                                      adjacentNodes[d][1]);

                    --edgeCount;
                    --allEdges[v][d ^ 0x1];

                    st.push({v, d});
                    moved = true;
                    break;
                }
            }
        }
        // No previous direction (start node) -> normal order
        else {
            for (int d = 0; d < 4; ++d) {
                uint32_t &edgeCount = allEdges[u][d];
                if (edgeCount > 0) {
                    uint32_t v = u + (adjacentNodes[d][0] * width +
                                      adjacentNodes[d][1]);

                    --edgeCount;
                    --allEdges[v][d ^ 0x1];

                    st.push({v, d});
                    moved = true;
                    break;
                }
            }
        }

        // Dead end -> backtrack
        if (!moved) {
            circuit.push_back(u);
            st.pop();
        }
    }

    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}


std::vector<uint32_t>
generatePath (
    const std::vector<uint8_t> &final_binary_image,
    int cutoffClustering,
    int height, int width
) {
    ScopedTimer timer("Perform Chinese Postman Problem");

    Graph allEdges(height * width, std::array<uint32_t, 4>({0, 0, 0, 0}));

    populateGraph(final_binary_image, allEdges, height, width);

    eulerizeGraph(final_binary_image, allEdges, cutoffClustering, height, width);

    // Perform Hierholzer's algorithm to find Eulerian Circuit
    std::vector<uint32_t> fullCircuit = findEulerCircuit(allEdges, 0, width);
    return fullCircuit;
}