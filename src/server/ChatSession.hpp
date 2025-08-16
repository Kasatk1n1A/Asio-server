#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <deque>
#include <boost/asio.hpp>
#include<unordered_set>


using boost::asio::ip::tcp;

class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
    ChatSession(tcp::socket socket, std::vector<std::shared_ptr<ChatSession>>& sessions)
        : socket_(std::move(socket)), sessions_(sessions){}

    void start() 
    {
        authorization();
    }
    ~ChatSession(){
        std::cout << User_login + " disconnected\n";
        socket_.close();
    }
    void deliver(const std::string& message);

    std::string getLogin() const noexcept(true){
        return User_login;
    }
    
private:
    void read_message();
    void write_message();
    void authorization();
    void read_login();
    void read_password();
    void send_confirm_login();
    void send_login_taken();
    void send_confirm_password();

    tcp::socket socket_;
    std::string read_buffer_;
    std::deque<std::string> write_msgs_;
    std::vector<std::shared_ptr<ChatSession>>& sessions_;

    std::string User_login;
    std::string User_password;
};