// test_client.cpp
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 1111));

        // Поток для чтения сообщений от сервера
        std::thread([&socket] {
            std::string message;
            while (true) {
                boost::system::error_code ec;
                size_t len = boost::asio::read_until(socket, boost::asio::dynamic_buffer(message), '\n', ec);
                if (ec) break;
                std::cout << "Received: " << message.substr(0, len);
                message.erase(0, len);
            }
        }).detach();

        // Главный поток для отправки сообщений
        std::string line;
        while (std::getline(std::cin, line)) {
            line += '\n';
            boost::asio::write(socket, boost::asio::buffer(line));
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}