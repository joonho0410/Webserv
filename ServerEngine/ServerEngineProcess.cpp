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

#include "./ServerEngine.hpp"
#include "../Structure.hpp"
#include "../Request/Request.hpp"
#include "../KqueueUdata.hpp"


void ServerEngine::waitConnect(struct kevent &curr_event){
    int client_socket;
    
    if ((client_socket = accept(curr_event.ident, NULL, NULL)) == -1)
        exit_with_perror("accept() error\n" + std::string(strerror(errno)));
    std::cout << "accept new client: " << client_socket << std::endl;
    fcntl(client_socket, F_SETFL, O_NONBLOCK);

    /* add event for client socket - add read event */
    _M_change_events(_m_change_list, client_socket, EVFILT_READ , EV_ADD | EV_ONESHOT, 0, 0, _M_make_udata(READ_REQUEST));
}

void ServerEngine::readRequest(struct kevent& curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Request &req = udata->getRequest();

    _M_read_request(curr_event, req);
    //_M_check_request_end();
    //_M_check_header();
    //_M_check_
}
void ServerEngine::readUrl(){

}
void ServerEngine::readCgiResult(){

}
void ServerEngine::writeResponse(){

}
void ServerEngine::excuteCgi(){

}
