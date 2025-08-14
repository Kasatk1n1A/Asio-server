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

    void handle_login(){
        boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n', [this](boost::system::error_code error, std::size_t length){
            if(!error){
                    

                   
                    std::cout << "Enter your login: ";
                    std::getline(std::cin, login_); 
                    login_ += '\n';

                    
                    boost::asio::async_write(socket, boost::asio::buffer(login_),
                        [this](boost::system::error_code ec, std::size_t)
                        {
                            if (!ec)
                            {
                                login_.erase(login_.size() - 1); 
                                std::cout << "Logged in as: " << login_ << std::endl;
                                read(); 
                                write(); 
                            }
                            else
                            {
                                std::cerr << "Login error: " << ec.message() << "\n";
                                socket.close();
                            }
                        });
                }
                else
                {
                    std::cerr << "Login error: " << error.message() << "\n";
                    socket.close();
                }
           
        });
    }
    void read()
    {
        boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
            [this](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {   
                    std::cout << "Received: " << read_buffer.substr(0, length);
                    read_buffer.erase(0, length);
                    read(); // Continue reading
                }
                else
                {   
                    std::cerr << "Read error: " << ec.message() << "\n";
                    socket.close();
                }
            });
    }

    void write()
    {
        boost::asio::post([this]()
        {
            std::string write_buffer;
            std::getline(std::cin, write_buffer);
            
            write_buffer += '\n'; // Add delimiter
            
            boost::system::error_code ec;
            boost::asio::async_write(socket, boost::asio::buffer(write_buffer), 
                [this](boost::system::error_code ec, std::size_t length)
                {
                    if (!ec)
                        write();
                    else
                    {
                        std::cerr << "Write error: " << ec.message() << "\n";
                        socket.close();
                    }
                });
        });
    }

public:
    Client(boost::asio::io_context& io_context, tcp::socket&& sock)
        : socket(std::move(sock))
    {
        handle_login();
        }

    ~Client()
    {
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