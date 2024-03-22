#include "../include/client_logger.h"

int main()
{
    client_logger_builder * builder = new client_logger_builder;
    builder -> add_file_stream("error.txt", logger::severity::error) 
            -> add_file_stream("info.txt", logger::severity::information)
            -> add_console_stream(logger::severity::debug)
            -> add_console_stream(logger::severity::warning);

    std::string log_format = "%m | %s | %d | %t";
    builder -> save_format(log_format);
    logger *logg = builder -> build();

    delete builder;

    logg -> log("Information ", logger::severity::information)
         -> log("trace\n", logger::severity::trace)
         -> log("DeBug", logger::severity::debug);

    delete logg;
}