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
    std::string login_;
    std::atomic<bool> running_{true};

    void handle_initial_prompt() {
        boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), ':', 
            [this](boost::system::error_code error, std::size_t length) {
                if (!error) {
                    std::cout << read_buffer.substr(0, length);
                    read_buffer.erase(0, length);
                    
                    std::getline(std::cin, login_);
                    
                    boost::asio::async_write(socket, boost::asio::buffer(login_ + "\n"),
                        [this](boost::system::error_code ec, std::size_t) {
                            if (!ec) {
                            
                                read_welcome();
                            }
                        });
                }
            });
    }

    void read_welcome() {
        boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << read_buffer.substr(1, length);
                    read_buffer.erase(0, length);
                    start_io();
                }
            });
    }

    void start_io() {
        std::thread read_thread([this]() {
            while (running_) {
                boost::asio::streambuf buf;
                boost::asio::read_until(socket, buf, '\n');
                std::cout << &buf;
            }
        });

        std::string message;
        while (running_ && std::getline(std::cin, message)) {
            if (message == "/quit") {
                running_ = false;
                break;
            }
            message += '\n';
            boost::asio::write(socket, boost::asio::buffer(message));
        }

        socket.close();
        if (read_thread.joinable()) {
            read_thread.join();
        }
    }

public:
    Client(boost::asio::io_context& io_context, tcp::socket&& sock)
        : socket(std::move(sock))
    {
        handle_initial_prompt();
    }

    ~Client()
    {
        running_ = false;
        socket.close();
    }
};

int main() 
{
    try 
    {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 1111));
        
        
        Client c(io_context, std::move(socket));
        
        io_context.run(); // Запускаем обработку асинхронных операций
        
    } 
    catch (std::exception& e) 
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}