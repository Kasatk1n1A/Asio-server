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

    const std::string& get_login() const { return login_; }

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
                    auto it = std::find(sessions_.begin(), sessions_.end(), self);
                    if (it != sessions_.end())
                        sessions_.erase(it);
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
                   
                    login_ = login_buffer_.substr(0, length);
                    login_buffer_.clear();

                    
                    std::string welcome_msg = "Welcome, " + login_ + "!\n";

                    deliver(welcome_msg);

                    std::cout << login_ << " connected" << std::endl;

                    read_message();
                }
                else
                {
                    auto it = std::find(sessions_.begin(), sessions_.end(), self);
                    if (it != sessions_.end())
                        sessions_.erase(it);
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
                    
                    std::string message = login_ + ": " + read_buffer_.substr(0, length);
                    read_buffer_.erase(0, length);

                    // Рассылаем всем клиентам
                    for (auto& session : sessions_)
                    {
                        session->deliver(message);
                    }

                    // Читаем следующее сообщение
                    read_message();
                }
                else
                {
                    
                    std::cout << login_ << " disconnected" << std::endl;
                    auto it = std::find(sessions_.begin(), sessions_.end(), self);
                    if (it != sessions_.end())
                        sessions_.erase(it);
                }
            });
    }
  
    void write_message()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
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
                    // Удаляем сессию при ошибке
                    auto it = std::find(sessions_.begin(), sessions_.end(), self);
                    if (it != sessions_.end())
                        sessions_.erase(it);
                }
            });
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
};

int main(int argc, char* argv[])
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

        std::thread t([&io_context](){ io_context.run(); });

        std::cout << "Server running on port " << argv[1] << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();

        io_context.stop();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}