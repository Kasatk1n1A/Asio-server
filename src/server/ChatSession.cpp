#include "ChatSession.hpp"

// Глобальный набор для хранения занятых логинов
std::unordered_set<std::string> logins_;

void ChatSession::deliver(const std::string& message)
{
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(message);
    if (!write_in_progress)
        write_message();
}

void ChatSession::read_message()
{
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::string msg = read_buffer_.substr(0, length);
                read_buffer_.erase(0, length);

                for (auto& session : sessions_)
                    if (session.get() != this)
                        session->deliver(msg);

                read_message();
            }
            else
            {
                // Удаляем сессию при ошибке
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                {
                    sessions_.erase(it);
                    logins_.erase(User_login); // Освобождаем логин при отключении
                }
            }
        });
}

void ChatSession::write_message()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                write_msgs_.pop_front();
                if (!write_msgs_.empty())
                    write_message();
            }
            else
            {
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                {
                    sessions_.erase(it);
                    
                    logins_.erase(User_login); // Освобождаем логин при отключении
                    
                }
            }
        });
}

void ChatSession::authorization()
{
    read_login();
}

void ChatSession::read_login()
{
    auto self(shared_from_this());

    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec) 
            {
                std::string received_login = read_buffer_.substr(0, length-1); // Убираем \n
                read_buffer_.clear();
                
                if (logins_.find(received_login) == logins_.end())
                {
                    // Логин свободен
                    User_login = received_login;
                    logins_.insert(User_login);
                       
                    // Отправляем подтверждение клиенту
                    send_confirm_login();
                }
                else
                {
                    // Логин занят
                    send_login_taken();
                }
            } 
            else 
            {
                socket_.close();
            }            
        });
}

void ChatSession::send_confirm_login()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, 
        boost::asio::buffer("LOGIN_OK\n"),
        [this, self](boost::system::error_code error, std::size_t){
            if (!error)
                read_password();
            else 
            {
                logins_.erase(User_login);
                socket_.close();
            }
        });
}

void ChatSession::send_login_taken()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer("LOGIN_TAKEN\n"),
        [this, self](boost::system::error_code error, std::size_t)
        {
            if (!error)
                // Даем клиенту еще попытку
                read_login();
            else 
                socket_.close();
        });
}

void ChatSession::send_confirm_password()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer("Welcome, " + self->getLogin() + "!\n"),
        [this, self](boost::system::error_code error, std::size_t)
        {
            if (!error)
                read_message();
        });
}

void ChatSession::read_password()
{
    auto self(shared_from_this());

    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                if (User_login.empty()) 
                {
                    // Если вдруг нет логина (не должно происходить)
                    socket_.close();
                    return;
                }
                
                User_password = read_buffer_.substr(0, length-1);
                read_buffer_.clear();
                std::cout << User_login << " connected successfully\n";
                
                // Отправляем подтверждение авторизации
                send_confirm_password();
            }
            else
            {
                logins_.erase(User_login);
                
                socket_.close();
            }
        });
}