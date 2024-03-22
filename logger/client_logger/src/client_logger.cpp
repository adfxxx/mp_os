#include <not_implemented.h>
#include <ctime>

#include "../include/client_logger.h"

std::map<std::string, std::pair<std::ofstream, int>> client_logger::_all_streams = std::map<std::string, std::pair<std::ofstream, int>>();

client_logger::client_logger(std::map<std::string, std::set<logger::severity>> streams, std::string log_format){
    std::runtime_error open_error ("Stream is not open");
    for(auto &[file, severities] : streams){
        if(_all_streams.find(file) == _all_streams.end() && _all_streams[file].second == 0){
            _all_streams[file].first.open(file);
            if(!_all_streams[file].first.is_open()){
                throw open_error;
            }
            _streams[file] = severities;
        }
        _all_streams[file].second++;
    }
    _log_format = log_format;
}

client_logger::client_logger(client_logger const &other) = default;

client_logger &client_logger::operator=(client_logger const &other) = default;

client_logger::client_logger(client_logger &&other) noexcept = default;

client_logger &client_logger::operator=(client_logger &&other) noexcept = default;

client_logger::~client_logger() noexcept
{
    for(auto &[file, severities] : _streams){
        _all_streams[file].second--;
        if(_all_streams[file].second == 0){
            _all_streams[file].first.close();
        }
    }
}

void get_string(const std::string &msg, std::string &log_format, const std::string &flag){
    size_t pos = log_format.find(flag);
    if(pos != std::string::npos){
        log_format.replace(pos, flag.size(), msg);
    }
}

void get_format(std::string &log_format, const std::string &text, const std::string &severity){
    std::time_t cur_time = std::time(nullptr);
    std::tm* now = std::localtime(&cur_time);
    char time_str[20];
    std::strftime(time_str, sizeof(time_str), "%T", now);

    get_string(text, log_format, "%m");

    get_string(time_str, log_format, "%t");

    get_string(severity, log_format, "%s");

    std::strftime(time_str, sizeof(time_str), "%F", now);
    get_string(time_str, log_format, "%d");
}

logger const *client_logger::log(const std::string &text, logger::severity severity) const noexcept
{
    std::string severity_str = severity_to_string(severity);

    std::string message = _log_format;
    get_format(message, text, severity_str);

    for(auto &[file, severities] : _streams){
        if(severities.find(severity) != severities.end()){
            _all_streams[file].first << message << std::endl;
        }
    }
    return this;
}