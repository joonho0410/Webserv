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
    int state;

    _M_read_request(curr_event, req);
    state = req.getState();
    switch (state)
    {
        case REQUEST_ERROR:
            break;
        case REQUEST_FINISH:
            break;
        default:
            /* 아직 parsing이 완료되지않았으므로 계속해서 파싱을 받아 온다. 타임아웃이 필요하다 */
            _M_change_events(_m_change_list, curr_event.ident, EVFILT_READ , EV_ADD | EV_ONESHOT, 0, 0, udata);
            break;
    }
}
void ServerEngine::readUrl(){

}
void ServerEngine::readCgiResult(){

}
void ServerEngine::writeResponse(){

}
void ServerEngine::excuteCgi(){

}
