#pragma once
#include <memory>
#include <unordered_map>
#include <string>

class ChatSession;

class ChatRoom {
public:
    ChatRoom(const std::string& name) : room_name(name) {}

    void Connect(std::shared_ptr<ChatSession> session);
    void Disconnect(std::shared_ptr<ChatSession> session);
    void Broadcast(const std::string& message, std::shared_ptr<ChatSession> sender = nullptr);
    
    const std::string& GetName() const { return room_name; }
    size_t GetUserCount() const { return sessions_.size(); }

private:
    std::string room_name;
    std::string room_password;
    std::unordered_map<std::string, std::shared_ptr<ChatSession>> sessions_;
};