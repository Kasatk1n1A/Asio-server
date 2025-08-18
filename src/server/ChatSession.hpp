#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <deque>
#include <boost/asio.hpp>
#include <unordered_set>
#include <unordered_map>


using boost::asio::ip::tcp;
class ChatRoom;

class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
    ChatSession(tcp::socket socket, std::vector<std::shared_ptr<ChatSession>>& sessions)
        : socket_(std::move(socket)), sessions_(sessions){}

    void start() 
    {
        authorization();
    }
    ~ChatSession();

    void deliver(const std::string& message);

    std::string getLogin() const noexcept(true){
        return User_login;
    }
    
private:

    void handle_command(const std::string& command);
    void join_room(const std::string& room_name);
    void leave_room();
    void list_rooms();

    void read_message();
    void write_message();
    void authorization();
    void read_login();
    void read_password();
    void send_confirm_password();
    void read_new_password();
    void create_room(const std::string& room_name);
    void ProcessingMessage(const std::string& msg);

    tcp::socket socket_;
    std::string read_buffer_;
    std::deque<std::string> write_msgs_;
    std::vector<std::shared_ptr<ChatSession>>& sessions_;

    std::weak_ptr<ChatRoom> current_room_;
    std::string User_login;
    std::string User_password;
};