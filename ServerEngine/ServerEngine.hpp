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

#include "../Structure.hpp"
#include "../Request/Request.hpp"
#include "../KqueueUdata.hpp"


void exit_with_perror(const std::string& msg)
{
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
}

class ServerEngine
{
    private:
        std::vector<int>                          _m_server_socket;
        int                                       _m_kq;
        std::map<int, std::string>                _m_clients; // map for client socket:data
        std::vector<struct kevent>                _m_change_list; // kevent vector for changelist
        struct kevent                             _m_event_list[8]; // kevent array for eventlist
        std::vector<struct server_config_struct>  _m_server_config_set;
    
    private:
        void _M_change_events(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
        void _M_disconnect_client(int client_fd, std::map<int, std::string>& clients);
        struct server_config_struct _M_find_server_and_port(std::string ports, std::string server_name);
        struct server_config_struct _M_find_location_block(struct server_config_struct &_server_block, std::string &url);
        KqueueUdata* _M_make_udata(int state = 0);

        int _M_solve_request();
        int _M_serve_response();
        Request _M_make_request(struct kevent& _curr_event);
        void _M_check_request(Request &_req);

        /* switch case */
        void _M_make_client_socket(struct kevent*);
        void _M_read_request(struct kevent&, Request&);

        /* switch case write at "ServerEngineProcess.cpp" */
        void waitConnect(struct kevent& curr_event);
        void readRequest(struct kevent& curr_event);
        void readUrl();
        void readCgiResult();
        void writeResponse();
        void excuteCgi();

    public :
        ServerEngine();
        ~ServerEngine();
        
        void make_serversocket();
        void start_kqueue();
        void set_config_set(std::vector<struct server_config_struct> config);
};