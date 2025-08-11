#include"../headers/server.hpp"

void chatSession::start(){

    room_.join(shared_from_this());
    read();
}

void chatSession::deliver(std::string_view message){
    bool write_in_proccess = !messages_to_write.empty();   
    messages_to_write.push_back(std::string(message));
    if(!write_in_proccess) write();
}

void chatSession::read(){
    auto self(shared_from_this());
    asio::async_read_until(
        socket_, 
        asio::dynamic_buffer(read_message_), '\n', 
        [this, self](boost::system::error_code ec, size_t)
        {
        if(!ec){
            if(!read_message_.empty() &&  read_message_.back() == '\n') read_message_.pop_back();
            room_.deliver(read_message_);
            read_message_.clear();
            read();
        }
        else
            room_.leave(shared_from_this());
    });
}

void chatSession::write(){
    auto self(shared_from_this());
    asio::async_write(
        socket_,
        asio::buffer(messages_to_write.front()),
        [this, self](boost::system::error_code ec, size_t){
            if(!ec){
                messages_to_write.pop_front();
                if(!messages_to_write.empty()){
                    write();
                }
            }
            else
                room_.leave(shared_from_this());
        }
        );
}

void chatRoom::join(std::shared_ptr<chatSession> session){
    participants_.emplace(session);
     for (const auto& msg : recent_messages_) {
        session->deliver(msg);
    }
}

void chatRoom::leave(std::shared_ptr<chatSession> session){
    participants_.erase(session);
}

void chatRoom::deliver(std::string_view message){
        recent_messages_.push_back(std::string(message));

        while(recent_messages_.size() > max_recent_msgs)
        {
            recent_messages_.pop_front();
        }

        for(const auto& participant : participants_){
            participant->deliver(message);
        }
}

void chatServer::do_acceptor(){
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket){
            if(!ec) {
                std::make_shared<chatSession>(std::move(socket), room_)->start();
            }
            do_acceptor();
        }
    );
}