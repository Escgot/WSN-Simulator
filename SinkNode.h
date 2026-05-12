#pragma once
#include "Node.h"
#include "Config.h"
#include <vector>
#include <iostream>

class SinkNode : public Node {
private:
    std::vector<Message> receivedData;
    int totalReceived;

public:
    SinkNode(int id, float x, float y);

    void sendData(const Message& msg) override;
    Message receiveData() override;
    void receive(const Message& msg);
    void printStats() const;
    int getTotalReceived() const;
};
