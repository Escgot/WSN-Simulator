#pragma once
#include <string>

class Message {
private:
    int senderId;
    std::string data;
    int hopCount;

public:
    Message(int senderId, const std::string& data);

    const std::string& getData() const;
    int getSenderId() const;
    int getHopCount() const;
    void incrementHop();
};
