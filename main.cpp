#include <iostream>
#include <memory>
#include "Network.h"
#include "SensorNode.h"
#include "SinkNode.h"
#include "Config.h"
#include "ReportGenerator.h"

// Helper to build a consistent network for fair comparison
void buildNodes(Network& network) {
    // 10 sensor nodes spread across a 50x50 grid with communication range 20
    network.addNode(std::make_shared<SensorNode>(1,  10.0f,  5.0f,  5.0f,  "Temperature", 20.0f));
    network.addNode(std::make_shared<SensorNode>(2,  10.0f, 10.0f, 20.0f,  "Humidity",    20.0f));
    network.addNode(std::make_shared<SensorNode>(3,  10.0f, 15.0f, 10.0f,  "Pressure",    20.0f));
    network.addNode(std::make_shared<SensorNode>(4,  10.0f, 30.0f, 15.0f,  "Temperature", 20.0f));
    network.addNode(std::make_shared<SensorNode>(5,  10.0f, 20.0f, 30.0f,  "Humidity",    20.0f));
    network.addNode(std::make_shared<SensorNode>(6,  10.0f, 35.0f, 35.0f,  "Pressure",    20.0f));
    network.addNode(std::make_shared<SensorNode>(7,  10.0f, 40.0f, 20.0f,  "Temperature", 20.0f));
    network.addNode(std::make_shared<SensorNode>(8,  10.0f, 45.0f, 40.0f,  "Humidity",    20.0f));
    network.addNode(std::make_shared<SensorNode>(9,  10.0f, 25.0f, 45.0f,  "Pressure",    20.0f));
    network.addNode(std::make_shared<SensorNode>(10, 10.0f, 12.0f, 38.0f,  "Temperature", 20.0f));
}

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║        WSN Simulator v3.0                ║\n";
    std::cout << "║  Multi-Hop vs LEACH Protocol Comparison  ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";

    const int ROUNDS = 15;
    std::vector<RoundStats> mhStats, leStats;
    int totalNodes = 0;

    // ─── Run 1: Multi-Hop Protocol ──────────────────────────────────────────
    {
        auto sink = std::make_shared<SinkNode>(0, 25.0f, 25.0f);
        Network network(sink);
        buildNodes(network);
        network.simulate(ROUNDS, Protocol::MULTI_HOP);
        network.exportCSV("wsn_multihop.csv");
        mhStats = network.getStats();
        totalNodes = network.getNodeCount();
    }

    std::cout << "\n\n";

    // ─── Run 2: LEACH Protocol ──────────────────────────────────────────────
    {
        auto sink = std::make_shared<SinkNode>(0, 25.0f, 25.0f);
        Network network(sink);
        buildNodes(network);
        network.simulate(ROUNDS, Protocol::LEACH, 0.2f);
        network.exportCSV("wsn_leach.csv");
        leStats = network.getStats();
    }

    // ─── Generate Visual Report ─────────────────────────────────────────────
    ReportGenerator::generate("wsn_report.html", mhStats, leStats, totalNodes);

    std::cout << "\n✅ All outputs generated:\n";
    std::cout << "   📄 wsn_multihop.csv\n";
    std::cout << "   📄 wsn_leach.csv\n";
    std::cout << "   📊 wsn_report.html  ← Open in browser for interactive charts\n";

    return 0;
}
