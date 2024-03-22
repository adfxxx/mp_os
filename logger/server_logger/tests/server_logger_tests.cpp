#include "../include/server_logger_builder.h"
#include "../include/server_logger.h"

#define MSG_SIZE 100

int main()
{
    std::string file_name = "stream";
    std::runtime_error open_error("Error opening queue");

    std::string file = "/" + file_name;
    mq_unlink(file.c_str());

    struct mq_attr q_attr; 
    q_attr.mq_maxmsg = 10; 
    q_attr.mq_msgsize = MSG_SIZE; 

    mqd_t descr = mq_open(file.c_str(), O_CREAT | O_RDONLY, 0644, &q_attr);
    if (descr < 0){
        throw open_error;
    }

    logger_builder *builder = new server_logger_builder;
    logger *logg = builder -> add_file_stream(file_name, logger::severity::error) 
                           -> add_file_stream(file_name, logger::severity::debug) -> build();
    delete builder;

    logg -> log("deBug", logger::severity::debug)
         -> log("1234567890 qwertyuiop[] asdfghjkl ;' zxcvbnm,. //.,mnbvcxz';lkjhgfdsapoiuytrewq 0987654321", logger::severity::error);

    char message[MSG_SIZE];
    unsigned int prior;
    while (mq_receive(descr, message, MSG_SIZE, &prior))
    {
        char *ptr = message;

        bool is_true = *reinterpret_cast<bool *>(ptr);
        std::string type = is_true ? "info" : "meta";
        std::cout << std::endl << "Type of message: " << type << std::endl;
        ptr += sizeof(bool);

        size_t count = *reinterpret_cast<size_t *>(ptr);
        std::cout << "Number of packets: " << count << std::endl;
        ptr += sizeof(size_t);

        size_t request = *reinterpret_cast<size_t *>(ptr);
        std::cout << "Number of request: " << request << std::endl;
        ptr += sizeof(size_t);

        pid_t id = *reinterpret_cast<pid_t *>(ptr);
        std::cout << "Id: " << id << std::endl;
        ptr += sizeof(pid_t);

        char severity[6];
        strcpy(severity, ptr);

        std::cout << "Severity: " << severity << std::endl;
        std::cout << "Message: " << std::endl;
        for (size_t i = 0; i < count; ++i)
        {
            mq_receive(descr, message, MSG_SIZE, nullptr);
            std::cout << "  Packet " << i + 1 << ": " << std::endl;

            ptr = message;
            is_true = *reinterpret_cast<bool *>(ptr);
            type = is_true ? "info" : "meta";
            std::cout << std::endl << "   Type of message: " << type << std::endl;
            ptr += sizeof(bool);

            size_t request = *reinterpret_cast<size_t *>(ptr);
            std::cout << "   Number of request: " << request << std::endl;
            ptr += sizeof(size_t);

            pid_t id = *reinterpret_cast<pid_t *>(ptr);
            std::cout << "   Id: " << id << std::endl;
            ptr += sizeof(pid_t);
            std::cout << "   Info: " << ptr << std::endl;
        }
        std::cout << std::endl;
    }
    mq_close(descr);
    mq_unlink(file.c_str());

    delete logg;
    return 0;
}