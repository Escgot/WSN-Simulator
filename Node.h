#pragma once
#include "Message.h"
#include <cmath>
#include <memory>

class Node {
protected:
    int id;
    float energy;
    float x, y;

public:
    Node(int id, float energy, float x, float y);
    virtual ~Node() = default;

    virtual void sendData(const Message& msg) = 0;
    virtual Message receiveData() = 0;

    float getEnergy() const;
    bool isAlive() const;
    int getId() const;
    float getX() const;
    float getY() const;

    float distanceTo(const Node& other) const;
    void consumeEnergy(float amount);
};
