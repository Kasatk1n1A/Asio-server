#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <deque>
#include <boost/asio.hpp>
#include "ChatSession.hpp"


using boost::asio::ip::tcp;



class ChatServer
{
public:
    ChatServer(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        accept_connection();
    }

private:
    void accept_connection()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    auto session = std::make_shared<ChatSession>(std::move(socket), sessions_);
                    sessions_.push_back(session);
                    session->start();
                    
                }

                accept_connection();
            });
    }

    

    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<ChatSession>> sessions_;
    
    std::unordered_set<std::string> logins_;
    std::unordered_map<std::string, std::shared_ptr<ChatRoom>> Rooms_list;
    std::unordered_map<std::string, std::string> registered_users_; // login -> password
    std::unordered_set<std::string> active_users_; // currently logged in
    
};