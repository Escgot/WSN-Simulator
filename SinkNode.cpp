#include "SinkNode.h"

SinkNode::SinkNode(int id, float x, float y)
    : Node(id, Config::SINK_ENERGY, x, y), totalReceived(0) {}

void SinkNode::sendData(const Message& msg) {
    std::cout << "[SinkNode] does not send data.\n";
}

Message SinkNode::receiveData() {
    if (!receivedData.empty())
        return receivedData.back();
    return Message(-1, "empty");
}

void SinkNode::receive(const Message& msg) {
    receivedData.push_back(msg);
    totalReceived++;
    std::cout << "[SinkNode] Received from Node "
              << msg.getSenderId()
              << " | Data: \"" << msg.getData()
              << "\" | Hops: " << msg.getHopCount() << "\n";
}

void SinkNode::printStats() const {
    std::cout << "\n--- SinkNode Stats ---\n";
    std::cout << "Total messages received: " << totalReceived << "\n";
    for (const auto& m : receivedData) {
        std::cout << "  From Node " << m.getSenderId()
                  << ": \"" << m.getData()
                  << "\" (hops: " << m.getHopCount() << ")\n";
    }
    std::cout << "----------------------\n";
}

int SinkNode::getTotalReceived() const { return totalReceived; }
