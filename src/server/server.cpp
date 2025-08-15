#include <iostream>
#include <vector>
#include <memory>
#include <deque>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
    ChatSession(tcp::socket socket, std::vector<std::shared_ptr<ChatSession>>& sessions)
        : socket_(std::move(socket)), sessions_(sessions)
    {
    }

    void start()
    {   
        request_login();
    }

    void deliver(const std::string& message)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(message);
        if (!write_in_progress)
        {
            write_message();
        }
    }

private:
    void request_login()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer("Enter your login: "), 
            [this, self](boost::system::error_code error, std::size_t)
            {
                if(!error) 
                {
                    read_login();
                }
                else
                {
                    remove_session(self);
                }
            });
    }

    void read_login()
    {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(login_buffer_), '\n', 
            [this, self](boost::system::error_code error, std::size_t length)
            {
                if(!error) 
                {
                    login_ = login_buffer_.substr(0, length - 1);
                    login_buffer_.clear();
                    login_.erase(std::remove(login_.begin(), login_.end(), ' '), login_.end());
                    // надо добавить обработку пустой строки и повторяющихся логинов
                    if(!login_.empty()) {
                    std::string welcome_msg = "Welcome, " + login_ + "!\n";
                    deliver(welcome_msg);
                    
                    std::cout << login_ << " connected" << std::endl;
                    read_message();
                    }
                    else
                        {
                            socket_.close();
                        }
                }
                else
                {
                    remove_session(self);
                }
            });
    }

    void read_message()
    {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
            [this, self](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    std::string msg = read_buffer_.substr(0, length - 1);
                    read_buffer_.erase(0, length);
                    
                    std::string message = login_ + ": " + msg + "\n";
                    
                    // Создаем копию списка сессий для безопасного доступа
                    auto sessions_copy = sessions_;
                    for (auto& session : sessions_copy)
                    {
                        if (session.get() != this && session->socket_.is_open())
                        {
                            session->deliver(message);
                        }
                    }

                    read_message();
                }
                else
                {
                    std::cout << login_ << " disconnected" << std::endl;
                    remove_session(self);
                }
            });
    }
  
    void write_message()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()),
            [this, self](boost::system::error_code ec, std::size_t)
            {
                if (!ec)
                {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty())
                    {
                        write_message();
                    }
                }
                else
                {
                    remove_session(self);
                }
            });
    }

    void remove_session(std::shared_ptr<ChatSession> session)
    {
        auto it = std::find(sessions_.begin(), sessions_.end(), session);
        if (it != sessions_.end())
        {
            sessions_.erase(it);
        }
    }

    tcp::socket socket_;
    std::string login_;
    std::string login_buffer_;
    std::string read_buffer_;
    std::deque<std::string> write_msgs_;
    std::vector<std::shared_ptr<ChatSession>>& sessions_;
};

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
                    std::lock_guard<std::mutex> lock(sessions_mutex_);
                    auto session = std::make_shared<ChatSession>(
                        std::move(socket), sessions_);
                    sessions_.push_back(session);
                    session->start();
                }

                accept_connection();
            });
    }

    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<ChatSession>> sessions_;
    std::mutex sessions_mutex_;
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: chat_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        ChatServer server(io_context, std::atoi(argv[1]));

        std::thread t([&io_context]()
                      { io_context.run(); });

        std::cout << "Server running on port " << argv[1] << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();

        io_context.stop();
        t.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}