#pragma once
#include "SensorNode.h"
#include "SinkNode.h"
#include "Config.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

enum class Protocol { MULTI_HOP, LEACH };

struct RoundStats {
    int round;
    int aliveNodes;
    int deadNodes;
    float totalEnergyRemaining;
    int messagesDelivered;
    float avgHops;
};

class Network {
private:
    std::vector<std::shared_ptr<SensorNode>> nodes;
    std::shared_ptr<SinkNode> sink;
    std::vector<RoundStats> stats;
    int firstNodeDeathRound = -1;
    int lastNodeDeathRound  = -1;

public:
    Network(std::shared_ptr<SinkNode> sink);
    ~Network() = default;

    void addNode(std::shared_ptr<SensorNode> node);

    // Topology
    void discoverNeighbors();

    // Multi-hop routing (BFS from sink)
    void computeRoutes();

    // Simulation
    void simulate(int rounds, Protocol protocol = Protocol::MULTI_HOP, float leachP = 0.2f);

    // Metrics
    RoundStats collectRoundStats(int round, int messagesThisRound, float totalHops);
    void printSummary(const std::string& protocolName) const;
    void exportCSV(const std::string& filename) const;

    // Reset for protocol comparison
    void resetNetwork();

    // Access stats for reporting
    const std::vector<RoundStats>& getStats() const;
    int getNodeCount() const;

private:
    // Protocol-specific simulation loops
    void runMultiHop(int rounds);
    void runLEACH(int rounds, float p);
};
