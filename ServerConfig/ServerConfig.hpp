#pragma once
#include "../Structure.hpp"
#include "../ParsingUtility.hpp"

#include <map>
#include <vector>
#include <string>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

/* root 과 alias가 같이 쓰인 경우 error */

class ServerConfig
{
    /* member */
    private:
        std::string _path;
        std::vector<struct server_config_struct> _config_struct;
    
    /*  default function */
    public:
        ServerConfig(){ _path = "./default.conf"; }
        ~ServerConfig(){}
    
    /*  private function */
    private:
        void _M_parse_file();
        void _M_parse_KeyValuePairs(std::string const &input);
        void _M_parse_line(struct server_config_struct &config, std::string &line);

        void _M_check_config();
        void _M_check_serverName(std::vector<struct server_config_struct >::iterator &_temp);
        void _M_check_listen(std::vector<struct server_config_struct>::iterator _temp);
        void _M_check_ip(std::string str);
        void _M_check_root(std::vector<struct server_config_struct>::iterator _temp);
        
    /* public function */
    public :
        void set_path(std::string path);
        void make_config();
        std::vector<struct server_config_struct> get_config() const;      
};