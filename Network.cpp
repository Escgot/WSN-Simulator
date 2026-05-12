#include "Network.h"

Network::Network(std::shared_ptr<SinkNode> sink) : sink(sink) {}

void Network::addNode(std::shared_ptr<SensorNode> node) {
    nodes.push_back(node);
    std::cout << "[Network] Node " << node->getId()
              << " added at (" << node->getX() << ", " << node->getY() << ")\n";
}

void Network::resetNetwork() {
    stats.clear();
    firstNodeDeathRound = -1;
    lastNodeDeathRound = -1;
    for (auto& node : nodes) {
        node->resetEnergy();
    }
}

// ─── Topology Discovery ────────────────────────────────────────────────────────

void Network::discoverNeighbors() {
    for (auto& node : nodes) {
        node->clearNeighbors();
    }

    for (auto& node : nodes) {
        if (!node->isAlive()) continue;

        for (auto& other : nodes) {
            if (node == other || !other->isAlive()) continue;
            if (node->distanceTo(*other) <= node->getRange()) {
                node->addNeighbor(other);
            }
        }

        if (node->distanceTo(*sink) <= node->getRange()) {
            node->addNeighbor(sink);
        }
    }
}

// ─── BFS Routing ────────────────────────────────────────────────────────────────

void Network::computeRoutes() {
    std::unordered_map<int, int> hopDistance;
    std::unordered_map<int, std::shared_ptr<Node>> bestNextHop;
    std::queue<std::shared_ptr<Node>> bfsQueue;

    hopDistance[sink->getId()] = 0;
    bfsQueue.push(sink);

    while (!bfsQueue.empty()) {
        auto current = bfsQueue.front();
        bfsQueue.pop();

        for (auto& node : nodes) {
            if (!node->isAlive()) continue;
            if (hopDistance.count(node->getId())) continue;

            bool canReach = false;
            for (auto& nb : node->getNeighbors()) {
                if (nb->getId() == current->getId()) {
                    canReach = true;
                    break;
                }
            }

            if (canReach) {
                hopDistance[node->getId()] = hopDistance[current->getId()] + 1;
                bestNextHop[node->getId()] = current;
                bfsQueue.push(node);
            }
        }
    }

    for (auto& node : nodes) {
        if (bestNextHop.count(node->getId()))
            node->setNextHop(bestNextHop[node->getId()]);
        else
            node->setNextHop(nullptr);
    }
}

// ─── Public Simulate Entry Point ────────────────────────────────────────────────

void Network::simulate(int rounds, Protocol protocol, float leachP) {
    if (protocol == Protocol::MULTI_HOP) {
        std::cout << "\n╔══════════════════════════════════════════╗\n";
        std::cout << "║     PROTOCOL: MULTI-HOP (BFS Routing)    ║\n";
        std::cout << "╚══════════════════════════════════════════╝\n";
        runMultiHop(rounds);
        printSummary("Multi-Hop");
    } else {
        std::cout << "\n╔══════════════════════════════════════════╗\n";
        std::cout << "║     PROTOCOL: LEACH (Cluster-Based)      ║\n";
        std::cout << "╚══════════════════════════════════════════╝\n";
        runLEACH(rounds, leachP);
        printSummary("LEACH");
    }
}

// ─── Multi-Hop Simulation ───────────────────────────────────────────────────────

void Network::runMultiHop(int rounds) {
    std::cout << "\n[Network] Discovering neighbors...\n";
    discoverNeighbors();
    for (auto& n : nodes)
        std::cout << "  Node " << n->getId() << ": " << n->getNeighbors().size() << " neighbor(s)\n";

    std::cout << "[Network] Computing BFS routes...\n";
    computeRoutes();
    for (auto& n : nodes) {
        auto nh = n->getNextHop();
        std::cout << "  Node " << n->getId() << " -> "
                  << (nh ? "Node " + std::to_string(nh->getId()) : "NO ROUTE") << "\n";
    }

    std::cout << "\n====== Simulation Start (" << rounds << " rounds) ======\n";

    for (int r = 1; r <= rounds; r++) {
        std::cout << "\n--- Round " << r << " ---\n";
        int msgs = 0;
        float hops = 0;

        for (auto& node : nodes) {
            if (!node->isAlive()) continue;
            if (!node->getNextHop()) {
                std::cout << "[Node " << node->getId() << "] No route.\n";
                continue;
            }

            Message msg = node->collectData();
            std::shared_ptr<Node> cur = node;
            bool delivered = false;

            while (cur && cur != sink) {
                auto sc = std::dynamic_pointer_cast<SensorNode>(cur);
                if (!sc) break;
                auto next = sc->getNextHop();
                if (!next) break;

                float d = cur->distanceTo(*next);
                sc->consumeEnergy(sc->transmitCost(d));
                msg.incrementHop();

                auto sNext = std::dynamic_pointer_cast<SensorNode>(next);
                if (sNext) sNext->consumeEnergy(Config::E_RECEIVE);

                if (next == sink) {
                    sink->receive(msg);
                    delivered = true;
                    break;
                }
                if (!next->isAlive()) break;
                cur = next;
            }

            if (delivered) { msgs++; hops += msg.getHopCount(); }
        }

        discoverNeighbors();
        computeRoutes();

        stats.push_back(collectRoundStats(r, msgs, hops));
        if (stats.back().deadNodes > 0 && firstNodeDeathRound == -1)
            firstNodeDeathRound = r;
        if (stats.back().aliveNodes == 0) {
            lastNodeDeathRound = r;
            std::cout << "\n[Network] All nodes dead. Ending simulation.\n";
            break;
        }
    }

    std::cout << "\n====== Simulation End ======\n";
    sink->printStats();
}

// ─── LEACH Simulation ───────────────────────────────────────────────────────────

void Network::runLEACH(int rounds, float p) {
    std::cout << "\n[LEACH] Cluster head probability p = " << p << "\n";
    std::cout << "[LEACH] Election cycle = " << (int)(1.0f / p) << " rounds\n";

    std::cout << "\n====== Simulation Start (" << rounds << " rounds) ======\n";

    for (int r = 1; r <= rounds; r++) {
        std::cout << "\n--- Round " << r << " ---\n";

        // ── Setup Phase: Reset clusters and elect CHs ──
        for (auto& node : nodes) node->resetClusterState();

        std::vector<std::shared_ptr<SensorNode>> clusterHeads;

        for (auto& node : nodes) {
            if (!node->isAlive()) continue;
            if (node->electAsClusterHead(r, p)) {
                clusterHeads.push_back(node);
                std::cout << "[LEACH] Node " << node->getId() << " elected as CH\n";
            }
        }

        // If no CHs elected, force the node with most energy
        if (clusterHeads.empty()) {
            std::shared_ptr<SensorNode> best = nullptr;
            float maxE = -1;
            for (auto& node : nodes) {
                if (node->isAlive() && node->getEnergy() > maxE) {
                    maxE = node->getEnergy();
                    best = node;
                }
            }
            if (best) {
                best->electAsClusterHead(r, 1.0f); // force election
                clusterHeads.push_back(best);
                std::cout << "[LEACH] Forced Node " << best->getId() << " as CH (no volunteers)\n";
            }
        }

        // ── Non-CH nodes join nearest CH ──
        for (auto& node : nodes) {
            if (!node->isAlive() || node->isClusterHead()) continue;

            float minDist = std::numeric_limits<float>::max();
            std::shared_ptr<SensorNode> nearestCH = nullptr;

            for (auto& ch : clusterHeads) {
                float d = node->distanceTo(*ch);
                if (d < minDist) {
                    minDist = d;
                    nearestCH = ch;
                }
            }

            if (nearestCH) {
                node->setClusterHead(nearestCH);
                nearestCH->addClusterMember(node);
            }
        }

        // Print cluster info
        for (auto& ch : clusterHeads) {
            std::cout << "[LEACH] CH " << ch->getId() << " has "
                      << ch->getClusterMembers().size() << " member(s)\n";
        }

        // ── Steady Phase: Members -> CH -> Sink ──
        int msgs = 0;
        float totalHops = 0;

        for (auto& ch : clusterHeads) {
            if (!ch->isAlive()) continue;

            // Members send data to CH
            for (auto& member : ch->getClusterMembers()) {
                if (!member->isAlive()) continue;

                Message data = member->collectData();
                float d = member->distanceTo(*ch);
                float cost = member->transmitCost(d);
                member->consumeEnergy(cost);

                // CH receives
                ch->consumeEnergy(Config::E_RECEIVE);
            }

            // CH aggregates and sends to sink
            Message agg = ch->aggregateData();
            float dSink = ch->distanceTo(*sink);
            float txCost = ch->transmitCost(dSink);
            ch->consumeEnergy(txCost);

            agg.incrementHop(); // member -> CH
            agg.incrementHop(); // CH -> sink
            sink->receive(agg);
            msgs++;
            totalHops += agg.getHopCount();

            std::cout << "[LEACH] CH " << ch->getId() << " -> Sink (dist: "
                      << std::fixed << std::setprecision(1) << dSink
                      << ", cost: " << std::setprecision(3) << txCost << ")\n";
        }

        // Collect stats
        stats.push_back(collectRoundStats(r, msgs, totalHops));
        if (stats.back().deadNodes > 0 && firstNodeDeathRound == -1)
            firstNodeDeathRound = r;
        if (stats.back().aliveNodes == 0) {
            lastNodeDeathRound = r;
            std::cout << "\n[Network] All nodes dead. Ending simulation.\n";
            break;
        }
    }

    std::cout << "\n====== Simulation End ======\n";
    sink->printStats();
}

// ─── Metrics ────────────────────────────────────────────────────────────────────

RoundStats Network::collectRoundStats(int round, int messagesThisRound, float totalHops) {
    RoundStats rs;
    rs.round = round;
    rs.aliveNodes = 0;
    rs.deadNodes = 0;
    rs.totalEnergyRemaining = 0;
    rs.messagesDelivered = messagesThisRound;
    rs.avgHops = messagesThisRound > 0 ? totalHops / messagesThisRound : 0;

    for (auto& node : nodes) {
        if (node->isAlive()) {
            rs.aliveNodes++;
            rs.totalEnergyRemaining += node->getEnergy();
        } else {
            rs.deadNodes++;
        }
    }
    return rs;
}

void Network::printSummary(const std::string& protocolName) const {
    std::cout << "\n╔══════════════════════════════════════════╗\n";
    std::cout << "║          SIMULATION SUMMARY              ║\n";
    std::cout << "║  Protocol: " << std::left << std::setw(29) << protocolName << "║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    std::cout << "Total nodes: " << nodes.size() << "\n";
    std::cout << "First Node Death (FND): ";
    if (firstNodeDeathRound != -1) std::cout << "Round " << firstNodeDeathRound << "\n";
    else std::cout << "None\n";

    std::cout << "Last Node Death (LND):  ";
    if (lastNodeDeathRound != -1) std::cout << "Round " << lastNodeDeathRound << "\n";
    else std::cout << "None\n";

    std::cout << "Total messages at sink: " << sink->getTotalReceived() << "\n";

    std::cout << "\n" << std::setw(7) << "Round"
              << std::setw(8) << "Alive" << std::setw(7) << "Dead"
              << std::setw(14) << "Energy Left" << std::setw(10) << "Msgs"
              << std::setw(11) << "Avg Hops" << "\n";
    std::cout << std::string(57, '-') << "\n";

    for (const auto& rs : stats) {
        std::cout << std::setw(7) << rs.round
                  << std::setw(8) << rs.aliveNodes
                  << std::setw(7) << rs.deadNodes
                  << std::setw(14) << std::fixed << std::setprecision(2) << rs.totalEnergyRemaining
                  << std::setw(10) << rs.messagesDelivered
                  << std::setw(11) << std::setprecision(1) << rs.avgHops << "\n";
    }
    std::cout << "=========================================\n";
}

void Network::exportCSV(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[Network] Failed to open " << filename << "\n";
        return;
    }

    file << "round,alive_nodes,dead_nodes,energy_remaining,messages_delivered,avg_hops\n";
    for (const auto& rs : stats) {
        file << rs.round << ","
             << rs.aliveNodes << "," << rs.deadNodes << ","
             << std::fixed << std::setprecision(2) << rs.totalEnergyRemaining << ","
             << rs.messagesDelivered << ","
             << std::setprecision(1) << rs.avgHops << "\n";
    }
    file.close();
    std::cout << "[Network] Stats exported to " << filename << "\n";
}

const std::vector<RoundStats>& Network::getStats() const { return stats; }
int Network::getNodeCount() const { return (int)nodes.size(); }
