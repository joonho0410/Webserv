#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

#include "../Structure.hpp"
#include "../Request/Request.hpp"
#include "../KqueueUdata.hpp"
#include "../ParsingUtility.hpp"
#include "../CGI/CgiHandler.hpp"
#include "Session.hpp"

class ServerEngine : public Session
{
    private:
        std::vector<int>                          _m_server_socket;
        int                                       _m_kq;
        std::map<int, std::string>                _m_clients; // map for client socket:data
        std::vector<struct kevent>                _m_change_list; // kevent vector for changelist
        struct kevent                             _m_event_list[8]; // kevent array for eventlist
        std::vector<struct server_config_struct>  _m_server_config_set;
        
    private:
        struct server_config_struct _M_findServerPort(std::string ports, std::string server_name);
        struct server_config_struct _M_findLocationBlock(struct server_config_struct &_server_block, std::string &url);

        KqueueUdata* _M_makeUdata(int state);
        void _M_autoIndexing(struct kevent &curr_event, std::string serverUrl);
        void _M_disconnectClient(struct kevent& , std::map<int, std::string>& clients);
        void _M_changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
        bool _M_checkMethod(struct server_config_struct &, struct server_config_struct&, std::string method );
        int _M_openDocs(std::string serverUrl);
        int _M_openPOST(std::string serverUrl);
        int _M_openPUT(std::string serverUrl);

        /* switch case */
        void _M_readRequest(struct kevent&, Request&);
        void _M_executeRequest(struct kevent& curr_event, Request &req);

        /* switch case write at "ServerEngineProcess.cpp" */
        void waitCgiEnd(struct kevent &curr_event);
        void waitConnect(struct kevent& curr_event);
        void readRequest(struct kevent& curr_event);
        void readDocs(struct kevent& curr_event);
        void readCgiResult(struct kevent& curr_event);

        void postWrite(struct kevent& curr_event);
        void putWrite(struct kevent& curr_envent);
        void deleteFile(struct kevent& curr_envent);

        void writeFile(struct kevent& curr_evenvt);
        void writeResponse(struct kevent& curr_event);
        void excuteCgi(struct kevent& curr_event);
        void writeCgiBody(struct kevent& curr_event);

    public :
        ServerEngine();
        ~ServerEngine();
        
        void start_kqueue();
        void make_serversocket();
        void set_config_set(std::vector<struct server_config_struct> config);
};