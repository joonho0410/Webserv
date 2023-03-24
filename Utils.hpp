#pragma once

#include <string>
#include <map>
#include <vector>
#include "Request.hpp"
#include "ft_Response.hpp"
#define GYIM 1000

#define REQUEST 1
#define RESPONSE 2
#define CGI_REQUEST 3
#define CGI_RESPONSE 4

struct kqueue_data
{
    bool            error_flag;
    bool            is_cgi;
    int             requested_fd;
    int             fd[2];
    int             process_exit_status;
    std::string     response;
    
    Request         _request;
    Response        _response;
};

// map< _server_name, struct server_block>
// ㄴ map < _location_name, struct location_block>
// 서버블록은 location_block을 가지고있는다.
// location 블록은 location_block을 가지고있지않는다 x

struct config_struct_server
{   
    std::map <std::string, struct config_struct_server>         location_block;//< location block name, location block key_and_value>
    std::map <std::string, std::vector<std::string> >           key_and_value;//< server block key_and_value > 
};
