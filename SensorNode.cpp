#include "SensorNode.h"
#include "SinkNode.h"

// ─── Static RNG ─────────────────────────────────────────────────────────────────

std::mt19937& SensorNode::getRng() {
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

std::uniform_real_distribution<float>& SensorNode::getSensorDist() {
    static std::uniform_real_distribution<float> dist(0.0f, 10.0f);
    return dist;
}

std::uniform_real_distribution<float>& SensorNode::getProbDist() {
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist;
}

// ─── Constructor ────────────────────────────────────────────────────────────────

SensorNode::SensorNode(int id, float energy, float x, float y,
                       const std::string& sensorType, float range)
    : Node(id, energy, x, y), sensorType(sensorType), range(range),
      initialEnergy(energy), nextHop(nullptr) {}

// ─── Core Methods ───────────────────────────────────────────────────────────────

void SensorNode::sendData(const Message& msg) {
    float cost = nextHop ? transmitCost(distanceTo(*nextHop)) : Config::E_ELEC;
    consumeEnergy(cost);
    std::cout << "[SensorNode " << id << "] Sending: \""
              << msg.getData() << "\" | Cost: " << cost
              << " | Energy: " << energy << "\n";
}

Message SensorNode::receiveData() {
    consumeEnergy(Config::E_RECEIVE);
    return Message(id, "ack");
}

Message SensorNode::collectData() {
    consumeEnergy(Config::E_COLLECT);
    float value = getSensorDist()(getRng());
    std::string payload = sensorType + "=" + std::to_string(value);
    return Message(id, payload);
}

// ─── Topology ───────────────────────────────────────────────────────────────────

void SensorNode::addNeighbor(std::shared_ptr<Node> node) { neighbors.push_back(node); }
const std::vector<std::shared_ptr<Node>>& SensorNode::getNeighbors() const { return neighbors; }
float SensorNode::getRange() const { return range; }
void SensorNode::clearNeighbors() { neighbors.clear(); }

// ─── Multi-hop Routing ──────────────────────────────────────────────────────────

void SensorNode::setNextHop(std::shared_ptr<Node> node) { nextHop = node; }
std::shared_ptr<Node> SensorNode::getNextHop() const { return nextHop; }

// ─── LEACH ──────────────────────────────────────────────────────────────────────

bool SensorNode::electAsClusterHead(int round, float p) {
    // LEACH threshold formula: T(n) = p / (1 - p * (r mod 1/p))
    // Only nodes that haven't been CH in last 1/p rounds are eligible
    int cycle = (int)(1.0f / p);
    int rMod = round % cycle;

    if (round - lastCHRound < cycle && lastCHRound >= 0) {
        chFlag = false;
        return false;
    }

    float threshold = p / (1.0f - p * rMod);
    float roll = getProbDist()(getRng());

    if (roll < threshold) {
        chFlag = true;
        lastCHRound = round;
        return true;
    }

    chFlag = false;
    return false;
}

bool SensorNode::isClusterHead() const { return chFlag; }

void SensorNode::setClusterHead(std::shared_ptr<SensorNode> ch) { myClusterHead = ch; }
std::shared_ptr<SensorNode> SensorNode::getMyClusterHead() const { return myClusterHead; }

void SensorNode::addClusterMember(std::shared_ptr<SensorNode> member) {
    clusterMembers.push_back(member);
}

const std::vector<std::shared_ptr<SensorNode>>& SensorNode::getClusterMembers() const {
    return clusterMembers;
}

void SensorNode::resetClusterState() {
    chFlag = false;
    myClusterHead = nullptr;
    clusterMembers.clear();
}

Message SensorNode::aggregateData() {
    // Cluster head aggregates data from all members + itself
    consumeEnergy(Config::E_AGGREGATE);
    std::string aggregated = "AGG[" + sensorType + "=" + std::to_string(getSensorDist()(getRng()));
    for (auto& m : clusterMembers) {
        aggregated += "+" + std::to_string(m->getId());
    }
    aggregated += "]";
    return Message(id, aggregated);
}

// ─── Energy Model ───────────────────────────────────────────────────────────────

float SensorNode::transmitCost(float distance) const {
    return Config::E_ELEC + Config::E_AMP * distance * distance;
}

float SensorNode::getInitialEnergy() const { return initialEnergy; }

void SensorNode::resetEnergy() {
    energy = initialEnergy;
    nextHop = nullptr;
    chFlag = false;
    myClusterHead = nullptr;
    clusterMembers.clear();
    lastCHRound = -999;
    neighbors.clear();
}
