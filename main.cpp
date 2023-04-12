#include <iostream>
#include <fstream>

#include <map>
#include <vector>
#include <string>

#include "./ServerConfig/ServerConfig.hpp"
#include "./ServerEngine/ServerEngine.hpp"

/* this webserver is maded by 100% recycled code, so echo friendly !!! */

int main(int argc, char** argv)
{
    ServerEngine server_engine;
    ServerConfig server_config;
    std::vector<struct server_config_struct> config_set;
    
    /* config file parsing */
    if (argc == 1 || argc == 2)
    {
        if (argc == 2)
        {
            std::string path(argv[1]);
            server_config.set_path(path);
        }
        server_config.make_config();
        server_config.show_config();
        config_set = server_config.get_config();
    }
    server_engine.set_config_set(config_set);
    server_engine.make_serversocket();
    server_engine.start_kqueue();

    return (0);
}