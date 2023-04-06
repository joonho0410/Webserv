#include "ServerEngine.hpp"

void ServerEngine::waitCgiEnd(struct kevent &curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    int status;

    waitpid(curr_event.ident, &status, 0);
    udata->setState(READ_CGI_RESULT);
    lseek(fileno(udata->getoutFile()), 0, SEEK_SET);
    _M_changeEvents(_m_change_list, fileno(udata->getoutFile()), EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

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
            udata->setState(WRITE_RESPONSE);
            _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE , EV_ADD | EV_ONESHOT, 0, 0, udata);
            break;
        case REQUEST_FINISH:
            std::cout << "REQUEST_FINISH then req.show_save()" << std::endl;
            std::cout << "============= BODY =============" << std::endl;
            write(1, udata->getRequest().getBody().c_str(), udata->getRequest().getBody().length());
            sleep(10);
            _M_executeRequest(curr_event, req);
            break;
        default:
            /* 아직 parsing이 완료되지않았으므로 계속해서 파싱을 받아 온다. 타임아웃이 필요하다 */
            std::cout << "need reading more state : " << udata->getRequest().getState() << std::endl;
            _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_READ , EV_ADD | EV_ONESHOT, 0, 0, udata);
            break;
    }
}

void ServerEngine::_M_executeRequest(struct kevent& curr_event, Request &req){
    std::cout << "_M_executeRequest" << std::endl;

    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    std::map< std::string, std::vector<std::string> > &header = req.getHeader();
    struct server_config_struct serv;
    struct server_config_struct loca;
    std::string host;
    std::string ports;
    std::string serverName;
    std::string url;
    std::string serverUrl; // 실제로 서버부에서 찾아갈 주소 (root 이 존재할 경우 바뀐다.)
    Response& res = udata->getResponse();
    bool        isCgi = false;
    bool        isBodyOk = true;

    /* HOST 는 반드시 필요한 헤더이므로 있는지 없는지 확인하는 작업이 필요하다 존재하지않으면 40x error 나중에 header parsing 이 끝나면 check_valid 함수를 새로 파서 체크하는게 더 예쁠듯?*/
    if (header.find("HOST") == header.end()){
        std::cout << "HOST is not available " << std::endl;
        req.setErrorCode(WRONG_PARSING);
        res.setErrorCode(req.getErrorCode());
        udata->setState(WRITE_RESPONSE);
        _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE , EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    } 
    else
        host = *(header.find("HOST")->second.begin());
    /* 위의 코드는 임시로만든 Host 가 존재하는지 확인하는 코드 */
    if (host.find(":") == std::string::npos){
        serverName = host;
    } else {
        serverName = host.substr(0, host.find_first_of(":"));
        ports = host.substr(host.find_first_of(":") + 1 );
    }
    url = req.getUrl();
    std::cout << "url : " << url << std::endl;
    /* default server 에 대한 생각이 필요하다  && location block 을 찾지못할 경우에 그냥 바로 검색 */
    serv = _M_findServerPort(ports, serverName);
    if (serv.valid == false){
        ;// can't find server block return error 
    }
    loca = _M_findLocationBlock(serv, url);
    if (loca.valid == false){
        loca = serv; // location block 을 찾을 수 없기에 serv 블록에 환경대로 실행한다.
    }
    
    /* check is CGI */
    if (serv.key_and_value.find("cgi") != serv.key_and_value.end()){
        std::vector<std::string> cgi_value = serv.key_and_value.find("cgi")->second;
        std::string temp;

        if (loca.block_name == cgi_value.back())
            isCgi = true;
        else {
            temp = req.getUrl();
            for (int i = 0; i < cgi_value.size() - 1 ; ++ i){ 
                if (temp.length() < cgi_value[i].length())
                    ;
                else if (temp.substr(temp.length() - cgi_value[i].length()) == cgi_value[i]) {
                    std::string temp = cgi_value.back();
                    loca = _M_findLocationBlock(serv, temp);
                    isCgi = true;
                    break ;
                }
            }
        }
    }

    /* START CGI_PROCESS */
    if (isCgi == true) {
        /* check Body Size */ 
        if (loca.key_and_value.find("client_max_body_size") != loca.key_and_value.end())
            isBodyOk = req.checkBodySize(loca);
        else if (serv.key_and_value.find("client_max_body_size") != serv.key_and_value.end())
            isBodyOk = req.checkBodySize(serv);
        if (isBodyOk == false)
            ;/* 413 BAD REQUEST return */

        if (loca.key_and_value.find("alias") != loca.key_and_value.end())
            serverUrl = *loca.key_and_value["alias"].begin() + url;
        if (loca.key_and_value.find("root") != loca.key_and_value.end())
            serverUrl = (*loca.key_and_value["root"].begin()) + loca.block_name + url;
        if (url == ""){
            /* index 의 마지막까지 순회하면서 맞는파일이 있는지 확인하도록 수정해야함 */
            serverUrl = *loca.key_and_value["root"].begin() + *loca.key_and_value["index"].begin();
        }
        else
            serverUrl = req.getUrl();        
        req.setServerUrl(serverUrl);

        /* 함수 나누면 좋을듯? */
        FILE    *outFile = tmpfile();
        FILE    *inFile = tmpfile();
        int     infd = fileno(outFile);
        int     outfd = fileno(inFile);

        udata->setinFile(inFile);
        udata->setoutFile(outFile);
        udata->setRequestedFd(curr_event.ident);

        /* body의 존재 유무에 따라서 body를 넣어주고 실행할지 바로 실행할지 결정한다 */
        if (req.getBody().size() == 0) {
            udata->setState(EXCUTE_CGI);
            _M_changeEvents(_m_change_list, outfd,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        }
        else {
            udata->setState(WRITE_CGI_BODY);
            _M_changeEvents(_m_change_list, infd,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        }
       
        /*
        test form 
        std::cout << "==================" << std::endl;
        std::cout << "CGI EVENT IS ADDED" << std::endl;
        std::cout << "원본 url = " << req.getUrl() << std::endl;
        std::cout << "url = " << url << std::endl;
        std::cout << "serverUrl = " << serverUrl << std::endl;
        */
        return ;

    } else { // cgi가 아닌 요청들 처리
        std::cout << "url is : " << url << std::endl;
        /* check_Body_size */
        if (loca.key_and_value.find("client_max_body_size") != loca.key_and_value.end())
            isBodyOk = req.checkBodySize(loca);
        else if (serv.key_and_value.find("client_max_body_size") != serv.key_and_value.end())
            isBodyOk = req.checkBodySize(serv);
        if (isBodyOk == false)
            ;/* 413 BAD REQUEST return */

        /* SERVING STATIC HTML FILE && NEED CHECK METHOD IS ALLOWED */
        if (loca.key_and_value.find("alias") != loca.key_and_value.end())
            serverUrl = *loca.key_and_value["alias"].begin() + url;
        if (loca.key_and_value.find("root") != loca.key_and_value.end())
            serverUrl = (*loca.key_and_value["root"].begin()) + loca.block_name + url;
        if (url == ""){
            /* index 의 마지막까지 순회하면서 맞는파일이 있는지 확인하도록 수정해야함 */
            serverUrl = *loca.key_and_value["root"].begin() + loca.block_name + *loca.key_and_value["index"].begin();
        }
        std::cout << "server url is : " << serverUrl << std::endl;
        int fd = open(serverUrl.c_str(), O_RDONLY);
        if (fd != -1){
            _m_clients[fd] = "";
            fcntl(fd, F_SETFL, O_NONBLOCK);
            udata->setState(READ_DOCS);
            udata->setRequestedFd(curr_event.ident);
            _M_changeEvents(_m_change_list, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, udata);
        }
        else{
            // open error == 404 error //
            std::cout << "open error" << std::endl;
            _M_disconnectClient(curr_event, _m_clients);
        }
    }                            
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
    while ((bytesRead = read(curr_event.ident, buffer, 1023)) > 0){
        std::string data;

        buffer[bytesRead] = '\0';
        data = std::string(buffer);
        res.apeendResponse(data);
        totalBytesRead += bytesRead; // 읽은 바이트 수 누적
    }

    if (bytesRead < 0){
        std::cout << "Read ERROR ! " << std::endl;
        return ;
    }

    close(curr_event.ident);
    udata->setState(WRITE_RESPONSE);
    _M_changeEvents(_m_change_list, udata->getRequestedFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

void ServerEngine::readCgiResult(struct kevent& curr_event){
    /* read udata->outfile and make response */
    std::cout << "READ CGI RESULT " << std::endl;
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    char *buf;

    int outFilefd = fileno(udata->getoutFile());
    int size = lseek(outFilefd, 0, SEEK_END);
    if (size == -1)
        ;//50x server error
    lseek(outFilefd, 0, SEEK_SET);
    buf = new char[size + 1];
    if (buf == 0)
        ;//50x server error;
    int readsize = read(outFilefd, buf, size);
    if (readsize != size)
        ;//50x server error;
    buf[readsize] = '\0';
    std::string str = std::string(buf);
    udata->getResponse().apeendResponse(str);

    delete []buf;
    /* 파일의 역할을 모두 했으니 여기서 close 해줘야할까? */
    fclose(udata->getinFile());
    fclose(udata->getoutFile());
    udata->setState(WRITE_RESPONSE);
    std::cout << "READ CGI RESULT END" << std::endl;
    _M_changeEvents(_m_change_list, udata->getRequestedFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

void ServerEngine::writeResponse(struct kevent& curr_event){
    std::cout << "WRITE RESPONSE IS OCCURED " << std::endl;
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
    std::cout << "WRITE RESPONSE IS OCCURED " << std::endl;
    _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, curr_event.udata);
    return ;
}

void ServerEngine::excuteCgi(struct kevent& curr_event){
    std::cout << "EXCUTE CGI IS OCCURED " << std::endl;
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Request&   req = udata->getRequest();
    CgiHandler cgiHandler(req, fileno(udata->getinFile()), fileno(udata->getoutFile()));
    int pid;

    pid = cgiHandler.executeCgi();
    udata->setState(WAIT_CGI_END);
    _M_changeEvents(_m_change_list, pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, udata);
    std::cout << "EXCUTE CGI IS DONE " << std::endl;
}

void ServerEngine::writeCgiBody(struct kevent& curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);

    Request &req = udata->getRequest();
    std::string str = req.getBody();
    if (write(curr_event.ident, str.c_str(), str.size()) == -1){
        ;// return server error 50x 
    }
    udata->setState(EXCUTE_CGI);
    _M_changeEvents(_m_change_list, fileno(udata->getoutFile()),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);   
}