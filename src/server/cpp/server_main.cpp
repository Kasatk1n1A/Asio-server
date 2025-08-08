#include"../headers/server.hpp"

int main(int argc, char** argv){
    if(argc != 2) {
        std::cerr << "Usage: <chat_server> <port>\n";
        return 1;
    }

    try{
        asio::io_context io_context;
        chatServer(io_context, std::stoi(argv[1]));

        const auto num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        threads.reserve(num_threads -1);
        for(auto i = 0; i < num_threads - 1; ++i){
            threads.emplace_back([&io_context]{io_context.run(); });
        }

        std::cout << "Chat server started on port " << argv[1] 
                  << " with " << num_threads << " threads\n";
         io_context.run();
         for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    
}