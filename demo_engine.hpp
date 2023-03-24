#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <cstdlib>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "Utils.hpp"
#include "Request.hpp"

class ServerEngine
{
    private:
        std::vector<int>                          _m_server_socket;
        int                                       _m_kq;
        std::map<int, std::string>                _m_clients; // map for client socket:data
        std::vector<struct kevent>                _m_change_list; // kevent vector for changelist
        struct kevent                             _m_event_list[8]; // kevent array for eventlist
        std::vector<struct config_struct_server>  _m_server_config_set;
    
    private:
        void _M_change_events(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
        void _M_disconnect_client(int client_fd, std::map<int, std::string>& clients);
        struct config_struct_server _M_find_server_and_port(std::string ports, std::string server_name);
        struct config_struct_server _M_find_location_block(struct config_struct_server &_server_block, std::string &url);
        int _M_solve_request();
        int _M_serve_response();
        kqueue_data* _M_make_udata(int current_fd, bool is_cgi);

        Request _M_make_request(struct kevent* _curr_event);
        void _M_check_request(Request &_req);

    public :
        ServerEngine();
        ~ServerEngine();
        
        void make_serversocket();
        void start_kqueue();
        void set_config_set(std::vector<struct config_struct_server> config);
};