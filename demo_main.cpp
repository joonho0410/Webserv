#include <iostream>
#include <fstream>

#include <map>
#include <vector>
#include <string>
#include "demo_engine.hpp"
#include "demo_configP.hpp"
#include "Utils.hpp"
#include "Request.hpp"

int main(int argc, char** argv)
{
    ConfigParser parser;
    ServerEngine Server;
    std::vector<struct config_struct_server> config_set;
    
    /* config file parsing */
    if (argc == 1 || argc == 2)
    {
        if (argc == 2)
        {
            std::string path(argv[1]);
            parser.set_path(path);
        }
        parser.make_config();
        parser.show_config();
        config_set = parser.get_config();
        // request.parse_and_check_valid();
        // request.show_save();
    }
    else
        std::cout << "argument error!\n";

    /* start server engine with config file */
    while (1)
    {
        try
        {
            Server.set_config_set(config_set);
            Server.make_serversocket();
            Server.start_kqueue();
        }
        catch(std::exception &e)
        {
            std::cout << "server restart needed\n";
        }
    }
}