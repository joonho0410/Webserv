#include "ServerEngine.hpp"

void ServerEngine::waitConnect(struct kevent &curr_event){
    int client_socket;
    
    if ((client_socket = accept(curr_event.ident, NULL, NULL)) == -1)
        exit_with_perror("accept() error\n" + std::string(strerror(errno)));
    std::cout << "accept new client: " << client_socket << std::endl;
    fcntl(client_socket, F_SETFL, O_NONBLOCK);

    /* add event for client socket - add read event */
    _M_changeEvents(_m_change_list, client_socket, EVFILT_READ , EV_ADD | EV_ONESHOT, 0, 0, _M_makeUdata(READ_REQUEST));
}

void ServerEngine::readRequest(struct kevent& curr_event){
    std::cout << "READ_REQUEST OCCURED" << std::endl;
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Request& req = udata->getRequest();
    Response& res = udata->getResponse();
    int state;

    //std::cout << "\033[1;31m" << "state : " << state << "\033[0m" << std::endl;
    _M_readRequest(curr_event, req);
    //std::cout << "\033[1;31m" << "change state : " << state << "\033[0m" << std::endl;
    state = req.getState();
    switch (state)
    {
        case REQUEST_ERROR:
            std::cout << "REQEUST_ERROR OCCURED" << std::endl;
            res.setErrorCode(req.getErrorCode());
            _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE , EV_ADD | EV_ONESHOT, 0, 0, udata);
            break;
        case REQUEST_FINISH:
            std::cout << "REQUEST_FINISH then req.show_save()" << std::endl;
            req.show_save();
            executeRequest(curr_event, req);
            break;
        default:
            /* 아직 parsing이 완료되지않았으므로 계속해서 파싱을 받아 온다. 타임아웃이 필요하다 */
            std::cout << "need reading more state : " << udata->getRequest().getState() << std::endl;
            _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_READ , EV_ADD | EV_ONESHOT, 0, 0, udata);
            break;
    }
}

void ServerEngine::executeRequest(struct kevent& curr_event, Request &req){
    std::cout << "executeRequest" << std::endl;

    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    struct server_config_struct serv;
    struct server_config_struct loca;
    std::string host;
    std::string ports;
    std::string serverName;
    std::string url;
    std::string serverUrl; // 실제로 서버부에서 찾아갈 주소 (root 이 존재할 경우 바뀐다.)

    /* HOST 는 반드시 필요한 헤더이므로 있는지 없는지 확인하는 작업이 필요하다  */
    if (req.getHeader().find("HOST") == req.getHeader().end())
        std::cout << "host error" << std::endl;
    else
        host = *(req.getHeader().find("HOST")->second.begin());
    /* 위의 코드는 임시로만든 Host 가 존재하는지 확인하는 코드 */
    if (host.find(":") == std::string::npos)
        ports = "80";
    else
        ports = host.substr(host.find(':'));
    url = req.getUrl();
    /* default server 에 대한 생각이 필요하다  && location block 을 찾지못할 경우에 Error Page에 대한 생각도 필요 */
    serv = _M_findServerPort(ports, serverName);
    loca = _M_findLocationBlock(serv, url);

    if (loca.key_and_value.find("client_max_body_size") != loca.key_and_value.end()){
        size_t client_body_size;
        std::string temp;

        temp = *(loca.key_and_value.find("client_max_body_size")->second.begin());
        client_body_size = static_cast<size_t> (std::atol(temp.c_str()));
        if (client_body_size < req.getBody().size()){
            /* 413 BAD REQUEST RETURN */
            return ;
        }
    }
    else if (serv.key_and_value.find("client_max_body_size") != serv.key_and_value.end()){       
        size_t client_body_size;
        std::string temp;
        
        temp = *(loca.key_and_value.find("client_max_body_size")->second.begin());
        client_body_size = static_cast<size_t> (std::atol(temp.c_str()));
        if (client_body_size < req.getBody().size()){
            /* 413 BAD REQUEST RETURN */
            return ;
        }
    }
    /* GET DELETE METHOD SERVING STATIC HTML FILE */
    if (loca.key_and_value.find("root") != loca.key_and_value.end())
        serverUrl = (*loca.key_and_value["root"].begin()) + url;
    if (url == "")
    {
        /* index 의 마지막까지 순회하면서 맞는파일이 있는지 확인하도록 수정해야함 */
        serverUrl = *loca.key_and_value["root"].begin() + *loca.key_and_value["index"].begin();
    }
    int fd = open(serverUrl.c_str(), O_RDONLY);
    if (fd != -1)
    {
        _m_clients[fd] = "";
        fcntl(fd, F_SETFL, O_NONBLOCK);
        udata->setState(READ_DOCS);
        udata->setRequestedFd(curr_event.ident);
        _M_changeEvents(_m_change_list, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, udata);
    }
    else
        std::cout << "open error" << std::endl;                            
    /* GET POST METHOD SERVING CGI */
}

void ServerEngine::readDocs(struct kevent& curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Response &res = udata->getResponse();
    Request &req = udata->getRequest();
    char buffer[1024]; // 읽은 데이터를 저장할 버퍼
    int bytesRead = 0; // 읽은 바이트 수
    int totalBytesRead = 0; // 총 읽은 바이트 수

    // 파일 끝에 도달할 때까지 반복
    while ((bytesRead = read(curr_event.ident, buffer, 1023)) > 0)
    {
        std::string data;

        buffer[bytesRead] = '\0';
        data = std::string(buffer);
        res.apeendResponse(data);
        totalBytesRead += bytesRead; // 읽은 바이트 수 누적
    }

    if (bytesRead < 0) // 에러 발생 시 처리
    {
        std::cout << "Read ERROR ! " << std::endl;
        return ;
    }
    close(curr_event.ident);
    udata->setState(WRITE_RESPONSE);
    _M_changeEvents(_m_change_list, udata->getRequestedFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

void ServerEngine::readCgiResult(){

}
void ServerEngine::writeResponse(struct kevent& curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Response &res = udata->getResponse();
    Request &req = udata->getRequest();
    std::string& responseString = res.getResponse();
    const char* ret = responseString.c_str();
    int len = responseString.length();
    int bytes_written = write(curr_event.ident, ret, len);

    if (bytes_written < 0){
        std::cout << "WRITE ERROR" << std::endl;
    }
    udata->clean();
    _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, curr_event.udata);
    return ;
}
void ServerEngine::excuteCgi(){

}