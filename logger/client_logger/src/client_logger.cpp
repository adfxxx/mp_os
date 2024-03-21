#include <not_implemented.h>
#include <ctime>

#include "../include/client_logger.h"

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

void get_format(std::string &log_format, const std::string &text, const std::string &severity){
    std::time_t cur_time = std::time(nullptr);
    std::tm* now = std::localtime(&cur_time);
    char time_str[20];
    std::strftime(time_str, sizeof(time_str), "%T", now);

    size_t pos = log_format.find("%m");
    if(pos != std::string::npos){
        log_format.replace(pos, 2, text);
    }

    pos = log_format.find("%s");
    if(pos != std::string::npos){
        log_format.replace(pos, 2, severity);
    }

    pos = log_format.find("%t");
    if(pos != std::string::npos){
        log_format.replace(pos, 2, time_str);
    }

    std::strftime(time_str, sizeof(time_str), "%F", now);
    pos = log_format.find("%d");
    if(pos != std::string::npos){
        log_format.replace(pos, 2, time_str);
    }

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