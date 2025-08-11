#pragma once
#include"../../../include/some_libs.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;

class chatSession;

class chatRoom {
public:
    void join(std::shared_ptr<chatSession> session);
    void leave(std::shared_ptr<chatSession> session);
    void deliver(std::string_view message);

private:
    std::unordered_set<std::shared_ptr<chatSession>> participants_;
    std::deque<std::string> recent_messages_{};
    static constexpr size_t max_recent_msgs = 100;
};

class chatSession: public std::enable_shared_from_this<chatSession>{
    tcp::socket socket_;
    chatRoom& room_;
    std::string read_message_;
    std::deque<std::string> messages_to_write;
    void read();
    void write();
public:
    void start();
    void deliver(std::string_view);
    chatSession(tcp::socket socket, chatRoom& room) : socket_(std::move(socket)), room_(room){}

};

class chatServer{
        tcp::acceptor acceptor_;
        chatRoom room_;
        void do_acceptor();
public:
    chatServer(asio::io_context& io_context, int port) : acceptor_(io_context, port) {do_acceptor(); }
};