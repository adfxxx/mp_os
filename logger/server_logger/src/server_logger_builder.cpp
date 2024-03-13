#include <not_implemented.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#include "../include/server_logger_builder.h"

server_logger_builder::server_logger_builder() = default;

server_logger_builder::server_logger_builder(server_logger_builder const &other) = default;

server_logger_builder &server_logger_builder::operator=(server_logger_builder const &other) = default;

server_logger_builder::server_logger_builder(server_logger_builder &&other) noexcept = default;

server_logger_builder &server_logger_builder::operator=(server_logger_builder &&other) noexcept = default;

server_logger_builder::~server_logger_builder() noexcept = default;

logger_builder *server_logger_builder::add_file_stream(std::string const &stream_file_path, logger::severity severity)
{
    if(_streams.find(stream_file_path) == _streams.end()){
        _streams[stream_file_path].first = ftok(stream_file_path.c_str(), 'r');
    }
    _streams[stream_file_path].second.insert(severity);
    return this;
}

logger_builder *server_logger_builder::add_console_stream(logger::severity severity)
{
    add_file_stream(CONSOLE, severity);
    return this;
}

logger_builder* server_logger_builder::transform_with_configuration(std::string const &configuration_file_path, std::string const &configuration_path)
{
    nlohmann::json info;
    std::ifstream file(configuration_file_path, std::ios::binary);
    if(!file.is_open()){
        std::runtime_error open_file ("File is not open");
    }
    file>>info;
    key_t key;
    std::string file_name;
    std::string file_severity;
    logger::severity log_severity;
    for(auto &file_info : info[configuration_path]){
        file_name = file_info[0];
        key = ftok(file_name.c_str(), 'r');
        _streams[file_name].first = key;
        for(auto &severity : file_info[1]){
            file_severity = severity;
            log_severity = string_to_severity(file_severity);
            _streams[file_name].second.insert(log_severity);
        }
    }
}

logger_builder *server_logger_builder::clear()
{
    for(auto &[file, pair] : _streams){
        pair.second.clear();
    }
    _streams.clear();
    return this;
}

logger *server_logger_builder::build() const
{
    return new server_logger(_streams);
}