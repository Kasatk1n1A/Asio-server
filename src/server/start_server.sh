mkdir build && cd build
cmake -G Ninja ../CMakeLists.txt
cmake --build .
cd build
cd ..
cd cpp

read -p "Enter port number: " port && ./chat_server $port