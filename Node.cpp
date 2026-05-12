#include "Node.h"

Node::Node(int id, float energy, float x, float y)
    : id(id), energy(energy), x(x), y(y) {}

float Node::getEnergy() const { return energy; }
bool Node::isAlive() const { return energy > 0; }
int Node::getId() const { return id; }
float Node::getX() const { return x; }
float Node::getY() const { return y; }

float Node::distanceTo(const Node& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}

void Node::consumeEnergy(float amount) {
    energy -= amount;
    if (energy < 0) energy = 0;
}
