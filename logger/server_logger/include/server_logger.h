#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H

#include <logger.h>
#include "server_logger_builder.h"

class server_logger final:public logger
{
    friend class server_logger_builder;
    
    server_logger(std::map<std::string, std::set<logger::severity>> streams);

    size_t mutable _request;
    pid_t _id;
    
#ifdef _WIN32
    std::map<std::string, std::pair<HANDLE, std::set<logger::severity>>> _streams;
    static std::map<std::string, std::pair<HANDLE, int>> _all_streams;
#elif __linux__
    std::map<std::string, std::pair<mqd_t, std::set<logger::severity>>> _streams;
    static std::map<std::string, std::pair<mqd_t, int>> _all_streams;
#endif


public:

    server_logger(
        server_logger const &other);

    server_logger &operator=(
        server_logger const &other);

    server_logger(
        server_logger &&other) noexcept;

    server_logger &operator=(
        server_logger &&other) noexcept;

    ~server_logger() noexcept final;

public:

    [[nodiscard]] logger const *log(
        const std::string &message,
        logger::severity severity) const noexcept override;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_SERVER_LOGGER_H