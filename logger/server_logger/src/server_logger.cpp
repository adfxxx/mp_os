#include <not_implemented.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include "../include/server_logger.h"

server_logger::server_logger(std::map<std::string, std::pair<key_t, std::set<logger::severity>>> streams){
    std::runtime_error open_error ("Queue is not open");
    for(auto &[file, pair] : streams){
        if(_all_streams.find(file) == _all_streams.end()){
            int id = msgget(pair.first, 066 | IPC_CREAT);
            if(id < 0){
                throw open_error;
            }
            _all_streams[file].first = id;
        }
        int id = _all_streams[file].first;
        _streams[file].first = id;
        _streams[file].second = pair.second;
        _all_streams[file].second++;
    }
}

server_logger::server_logger(server_logger const &other) = default;

server_logger &server_logger::operator=(server_logger const &other) = default;

server_logger::server_logger(server_logger &&other) noexcept = default;

server_logger &server_logger::operator=(server_logger &&other) noexcept = default;

server_logger::~server_logger() noexcept
{
    for(auto &[file, pair] : _streams){
        _all_streams[file].second--;
        if(_all_streams[file].second == 0){
            msgctl(pair.first, IPC_RMID, 0);
        }
    }
}

logger const *server_logger::log(const std::string &text,logger::severity severity) const noexcept
{
    int size = sizeof(text)/1024 + 1;
    information msg;
    msg.type = 1;
    msg.info.first = size;
    msg.info.second = severity;
    message msg_2[size];
    for(int i = 0; i < size; i++){
        msg_2[i].type = 1;
        if(i != size - 1){
            strcpy(msg_2[i].text, text.substr(i*1024, 1024).c_str());
        }
        else{
            strcpy(msg_2[i].text, text.substr(i*1024).c_str());
        }
    }
    for(auto &[file, pair] : _streams){
        if(pair.second.find(severity) != pair.second.end()){
            msgsnd(pair.first, &msg, sizeof(msg), 0);
            for(int i = 0; i < size; i++){
                msgsnd(pair.first, &msg_2[i], sizeof(msg_2[i]), 0);
            }
        }
    }
    return this;
}