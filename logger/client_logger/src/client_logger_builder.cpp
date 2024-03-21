#include <not_implemented.h>

#include "../include/client_logger_builder.h"

client_logger_builder::client_logger_builder(){

}

client_logger_builder::client_logger_builder(client_logger_builder const &other) = default;

client_logger_builder &client_logger_builder::operator=(client_logger_builder const &other) = default;

client_logger_builder::client_logger_builder(client_logger_builder &&other) noexcept = default;

client_logger_builder &client_logger_builder::operator=(client_logger_builder &&other) noexcept = default;

client_logger_builder::~client_logger_builder() noexcept = default;

logger_builder *client_logger_builder::add_file_stream(std::string const &stream_file_path, logger::severity severity)
{
    _logs[stream_file_path].insert(severity);
    return this;
}

logger_builder *client_logger_builder::add_console_stream(logger::severity severity)
{
    add_file_stream(CONSOLE, severity);
    return this;
}

logger_builder* client_logger_builder::transform_with_configuration(std::string const &configuration_file_path, std::string const &configuration_path)
{
    std::runtime_error open_error ("Queue is not open");
    std::runtime_error empty_file ("File is empty");
    std::runtime_error open_file_error ("File is not open");

    nlohmann::json info;
    std::ifstream file(configuration_file_path);

    if(!file.is_open()){
        throw open_file_error;
    }
    if(file.peek() == EOF){
        throw empty_file;
    }
    file>>info;
    if(info.find(configuration_path) == info.end()){
        throw empty_file;
    }

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
    return this;
}

logger_builder *client_logger_builder::clear()
{
    for(auto &file : _logs){
        file.second.clear();
    }
    _logs.clear();
    return this;
}

logger *client_logger_builder::build() const
{
    return new client_logger(_logs, _log_format);
}

logger_builder * client_logger_builder::save_format(std::string &format)
{
    _log_format = format;
}