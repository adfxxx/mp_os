#include <not_implemented.h>

#include "../include/server_logger_builder.h"

server_logger_builder::server_logger_builder() = default;

server_logger_builder::server_logger_builder(server_logger_builder const &other) = default;

server_logger_builder &server_logger_builder::operator=(server_logger_builder const &other) = default;

server_logger_builder::server_logger_builder(server_logger_builder &&other) noexcept = default;

server_logger_builder &server_logger_builder::operator=(server_logger_builder &&other) noexcept = default;

server_logger_builder::~server_logger_builder() noexcept = default;

logger_builder *server_logger_builder::add_file_stream(std::string const &stream_file_path, logger::severity severity)
{
    std::string file;
    if(stream_file_path[0] != '/'){
#ifdef __linux__
        file = "/";
#elif _WIN32    
        file = "//./pipe/";
#endif
        file += stream_file_path;
    }
    else{
#ifdef __linux__
        file = stream_file_path;
#elif _WIN32
        file = "//./pipe";
#endif
    }
    _logs[file].insert(severity);
    return this;
}

logger_builder *server_logger_builder::add_console_stream(logger::severity severity)
{
    add_file_stream(CONSOLE, severity);
    return this;
}

logger_builder* server_logger_builder::transform_with_configuration(std::string const &configuration_file_path, std::string const &configuration_path)
{
    std::runtime_error open_error ("Queue is not open");
    nlohmann::json info;
    std::ifstream file(configuration_file_path);
    if(!file.is_open()){
        throw std::runtime_error ("File is not open");
    }
    file>>info;
    std::string file_name;
    std::string file_severity;
    logger::severity log_severity;
    for(auto &file_info : info[configuration_path]){
        file_name = file_info[0];
        for(auto &severity : file_info[1]){
            file_severity = severity;
            log_severity = string_to_severity(file_severity);
            _logs[file_name].insert(log_severity);
        }
    }
}

logger_builder *server_logger_builder::clear()
{
    for(auto &[file, pair] : _logs){
        pair.clear();
    }
    _logs.clear();
    return this;
}

logger *server_logger_builder::build() const
{
    return new server_logger(_logs);
}