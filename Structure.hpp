#pragma once 

#include <string>
#include <map>
#include <vector>

struct server_config_struct
{  
    bool   valid;
    std::string block_name;
    std::map <std::string, struct server_config_struct>                 location_block;//< location block name, location block key_and_value>
    std::map <std::string, std::vector<std::string> >                   key_and_value;//< server block key_and_value >
    std::map <std::string, std::vector< std::vector < std::string > > > duplicate_key_and_value; // for duplicate key and value ex) error_page & cgi
};

struct KeyValue {
    std::string key;
    std::string value;
};

enum kqueue_process
{
    WAIT_CONNECT,
    WAIT_CGI_END,
    READ_REQUEST,
    READ_DOCS,
    READ_CGI_RESULT,
    WRITE_FILE,
    WRITE_RESPONSE,
    WRITE_CGI_BODY,
    EXCUTE_CGI
};