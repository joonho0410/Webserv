#include "ServerEngine.hpp"

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
    std::cout << "READ_REQUEST OCCURED" << std::endl;
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Request& req = udata->getRequest();
    Response& res = udata->getResponse();
    int state;

    state = req.getState();
    std::cout << "\033[1;31m" << "state : " << state << "\033[0m" << std::endl;
    _M_read_request(curr_event, req);
    std::cout << "\033[1;31m" << "change state : " << state << "\033[0m" << std::endl;
    switch (state)
    {
        case REQUEST_ERROR:
            std::cout << "REQEUST_ERROR OCCURED" << std::endl;
            res.setErrorCode(req.getErrorCode());
            _M_change_events(_m_change_list, curr_event.ident, EVFILT_WRITE , EV_ADD | EV_ONESHOT, 0, 0, udata);
            break;
        case REQUEST_FINISH:
            std::cout << "REQUEST_FINISH then req.show_save()" << std::endl;
            req.show_save();
            break;
        default:
            /* 아직 parsing이 완료되지않았으므로 계속해서 파싱을 받아 온다. 타임아웃이 필요하다 */
            std::cout << "need reading more state : " << udata->getRequest().getState() << std::endl;
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
