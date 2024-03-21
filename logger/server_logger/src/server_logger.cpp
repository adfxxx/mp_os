#include <not_implemented.h>

#include "../include/server_logger.h"

server_logger::server_logger(std::map<std::string, std::set<logger::severity>> streams){
    std::runtime_error open_error ("Queue is not open");
    for(auto &[file, severities] : streams){
        if(_all_streams.find(file) == _all_streams.end()){
#ifdef _WIN32
            HANDLE id = CreateFile(stream_file_path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr); //путь к файлу, открытие для записи, файл недоступен другим процессам, стандартная защита, открытие только если существует, стандартные атрибуты
            if(id == INVALID_HANDLE_VALUE){
                throw open_error;
            }
#elif __linux__
            mqd_t id = mq_open(file.c_str(), O_WRONLY, 0644, nullptr); //имя очереди, только для записи, стандартные права доступа, стандартные атрибуты
            if(id < 0){
                throw open_error;
            }
#endif
            _all_streams[file].first = id;
        }
        _streams[file].first = _all_streams[file].first;
        _streams[file].second = severities;
        _all_streams[file].second++;
    }
}

server_logger::server_logger(server_logger const &other) = default;

server_logger &server_logger::operator=(server_logger const &other) = default;

server_logger::server_logger(server_logger &&other) noexcept = default;

server_logger &server_logger::operator=(server_logger &&other) noexcept = default;

server_logger::~server_logger() noexcept
{
    std::runtime_error close_error("Queue is not closed");
    for(auto &[file, pair] : _streams){
        _all_streams[file].second--;
        if(_all_streams[file].second == 0){
#ifdef _WIN32
            if(CloseHandle(_all_streams[file].first) == 0){
                throw close_error;
            }
#elif __linux__
            if(mq_close( _all_streams[file].first) < 0){
                throw close_error;
            }
#endif
        }
    }
}

logger const *server_logger::log(const std::string &text,logger::severity severity) const noexcept
{
    std::runtime_error send_error ("Message is not sent");

    size_t meta = sizeof(bool) + sizeof(pid_t) + sizeof(size_t) + sizeof(size_t) + sizeof(pid_t) + sizeof(const char*);
    size_t message = 100 - meta;
    size_t count = text.size()/message + 1;

    char info[meta];
    char *ptr = info;

    *reinterpret_cast<bool*>(ptr) = false;
    ptr += sizeof(bool);
    *reinterpret_cast<size_t*>(ptr) = count;
    ptr += sizeof(size_t);
    *reinterpret_cast<size_t*>(ptr) = _request;
    ptr += sizeof(size_t);
    *reinterpret_cast<pid_t*>(ptr) = _id;
    ptr += sizeof(pid_t);

    char const *severity_str = severity_to_string(severity).c_str();
    strcpy(ptr, severity_str);

    char msg[100];

    for(auto &[file, pair] : _streams){
        if(pair.second.find(severity) != pair.second.end()){ //send meta
#ifdef _WIN32
            if(WriteFile(pair.first, info, 100) == 0){
                throw send_error;
            }
#elif __linux__
            if(mq_send(pair.first, info, 100, 0) < 0){
                throw send_error;
            }
#endif
            ptr = msg;
            *reinterpret_cast<bool*>(ptr) = true;
            ptr += sizeof(bool);
            *reinterpret_cast<size_t*>(ptr) = _request;
            ptr += sizeof(size_t);
            *reinterpret_cast<pid_t*>(ptr) = _id;
            ptr += sizeof(pid_t);
            for(size_t i = 0; i < count; i++){
                size_t position = i*message;
                size_t ost = text.size() - position;
                size_t substr = (ost < message) ? ost : message;
                memcpy(ptr, text.substr(position, substr).c_str(), substr);
                *(ptr + substr) = 0;
#ifdef _WIN32
                if(WriteFile(pair.first, msg, 100) == 0){
                    throw send_error;
                }
#elif __linux__
                if(mq_send(pair.first, msg, 100, 0) < 0){
                    throw send_error;
                }
#endif
            }
        }
    }
    _request++;
    return this;
}
