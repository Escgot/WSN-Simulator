#include "Message.h"

Message::Message(int senderId, const std::string& data)
    : senderId(senderId), data(data), hopCount(0) {}

const std::string& Message::getData() const { return data; }
int Message::getSenderId() const { return senderId; }
int Message::getHopCount() const { return hopCount; }
void Message::incrementHop() { hopCount++; }
