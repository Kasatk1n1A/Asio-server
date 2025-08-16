#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

using boost::asio::ip::tcp;

class Client
{
private:
    tcp::socket socket;
    std::string read_buffer;
    std::string User_login;
    std::string User_password;

    
    void input_login();
    void input_password();
    void read();
    void write();
    void send_login_and_password();
    void send_login();
    void send_password();
    void wait_confirm_login();
    void wait_confirm_password();

public:
    Client(boost::asio::io_context& io_context, tcp::socket&& sock)
        : socket(std::move(sock))
    {
        input_login();

        send_login_and_password();
    }

    ~Client()
    {
        socket.close();
    }
};