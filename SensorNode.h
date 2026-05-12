#pragma once
#include "Node.h"
#include "Config.h"
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <random>

class SinkNode; // forward declaration

class SensorNode : public Node {
private:
    std::string sensorType;
    float range;
    float initialEnergy;

    // Topology (multi-hop)
    std::vector<std::shared_ptr<Node>> neighbors;
    std::shared_ptr<Node> nextHop;

    // LEACH cluster state
    bool chFlag = false;  // cluster head flag
    std::shared_ptr<SensorNode> myClusterHead;
    std::vector<std::shared_ptr<SensorNode>> clusterMembers;
    int lastCHRound = -999; // last round this node was a CH

    // RNG
    static std::mt19937& getRng();
    static std::uniform_real_distribution<float>& getSensorDist();
    static std::uniform_real_distribution<float>& getProbDist();

public:
    SensorNode(int id, float energy, float x, float y,
               const std::string& sensorType, float range);

    void sendData(const Message& msg) override;
    Message receiveData() override;
    Message collectData();

    // Topology
    void addNeighbor(std::shared_ptr<Node> node);
    const std::vector<std::shared_ptr<Node>>& getNeighbors() const;
    float getRange() const;
    void clearNeighbors();

    // Multi-hop routing
    void setNextHop(std::shared_ptr<Node> node);
    std::shared_ptr<Node> getNextHop() const;

    // LEACH methods
    bool electAsClusterHead(int round, float p);
    bool isClusterHead() const;
    void setClusterHead(std::shared_ptr<SensorNode> ch);
    std::shared_ptr<SensorNode> getMyClusterHead() const;
    void addClusterMember(std::shared_ptr<SensorNode> member);
    const std::vector<std::shared_ptr<SensorNode>>& getClusterMembers() const;
    void resetClusterState();
    Message aggregateData();

    // Energy model
    float transmitCost(float distance) const;
    float getInitialEnergy() const;
    void resetEnergy();
};
