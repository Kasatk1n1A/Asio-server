#include "Client.hpp"

void Client::read()
{
    boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::cout << read_buffer.substr(0, length);
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

void Client::write()
{
    boost::asio::post([this]()
    {
        std::string msg = '[' + User_login + "]:";
        std::string write_buffer;
        std::getline(std::cin, write_buffer);
        
        msg += write_buffer + '\n';
        // write_buffer += '\n'; // Add delimiter
        
        boost::system::error_code ec;
        boost::asio::async_write(socket, boost::asio::buffer(msg),
            [this](boost::system::error_code ec, std::size_t /*length*/)
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

void Client::input_login(){
    do  
    {
        User_login.erase(User_login.begin(), User_login.end());
        std::cout << "Enter login: ";
        std::getline(std::cin, User_login);
    } while (User_login.size() == 0 ||  User_login.size() > 32);
}

void Client::input_password(){
 do
    {
        User_password.erase(User_password.begin(), User_password.end());
        std::cout << "Enter password: ";
        std::getline(std::cin, User_password);
    } while (User_password.size() == 0 ||  User_password.size() > 32);
}

void Client::send_login_and_password()
{
    //Сначала отправляется логин, затем будет ожидание подтверждения
    //что логин пришёл успешно и можно отправлять пароль,
    //это сделано потому что елси отправлять  их последовательно то пароль будет утерян
    send_login();
}

//Отправляет логин серверу
void Client::send_login()
{
    boost::asio::async_write(socket, boost::asio::buffer(User_login + '\n'),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
                wait_confirm_login();
            else
            {
                std::cerr << "Send login error: " << ec.message() << "\n";
                socket.close();;
            }
        });
}

void Client::wait_confirm_login()
{
    boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
    [this](boost::system::error_code ec, std::size_t length)
    {
        if (!ec)
        {   
            std::string response = read_buffer.substr(0, length);
            read_buffer.clear();
            
            if (response == "LOGIN_TAKEN\n")
            {
                std::cout << "This login is already taken. Please choose another one.\n";
                input_login();
                // Отправляем новый логин и снова ждем подтверждения
                send_login();
            }
            else if (response == "LOGIN_OK\n")  
            {   
                input_password();
                send_password();
            }                
        }
        else
        {
            socket.close();
        }
    });
}
//Ожидание подтверждения пароля (пока не использовано)
void Client::wait_confirm_password()
{
    boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
    [this](boost::system::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            if (read_buffer == "All good\n")
                {
                    read_buffer.erase(0, length);
                    send_password();
                }
        }
        else
        {
            std::cerr << "Read confirm error: " << ec.message() << "\n";
            socket.close();
        }
    });
}

//отправка пароля серверу
void Client::send_password()
{
    //костыль, чтобы дать время серверу обработать логин и только потом отправить серверу пароль
    boost::asio::async_write(socket, boost::asio::buffer(User_password + '\n'), 
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                
                // Запускаем чтение в основном потоке io_context
                read();
                // Запускаем запись в отдельном потоке
                write();
            }
            else
            {
                std::cerr << "Write error: " << ec.message() << "\n";
                socket.close();
            }
        });
}