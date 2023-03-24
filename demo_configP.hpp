#pragma once

#include <map>
#include <vector>
#include <string>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <Utils.hpp>

class ConfigParser
{
    private:
        std::string _m_path;
        std::vector<struct config_struct_server>  _m_config;
        
    private:
        void _M_parse_file();
        void _M_parse_KeyValuePairs(std::string const &input);
        void _M_parse_line(struct config_struct_server &config, std::string &line);
        void _M_check_config();
        void _M_check_listen(std::vector<struct config_struct_server>::iterator _temp);
        void _M_check_root(std::vector<struct config_struct_server>::iterator _temp);
        void _M_check_ip(std::string str);

    public:
        ConfigParser();
         ~ConfigParser();
        
        void make_config();
        void show_config();
        void set_path(std::string path);  
        std::vector<struct config_struct_server> get_config() const;      
};

struct KeyValue {
    std::string key;
    std::string value;
};