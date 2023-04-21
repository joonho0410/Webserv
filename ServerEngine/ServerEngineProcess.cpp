#include "ServerEngine.hpp"

void ServerEngine::waitCgiEnd(struct kevent &curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Request& req = udata->getRequest();
    Response& res = udata->getResponse();
    int status;

    waitpid(curr_event.ident, &status, 0);//프로세스 회수
    if (WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0 ){
            std::cout << "process not ended with 0 " << WEXITSTATUS(status) << std::endl;// 비 정상 종료 이므로 50x error
            if (_m_clients.find(fileno(udata->getinFile())) != _m_clients.end())
                _m_clients.erase(fileno(udata->getinFile()));
            if (_m_clients.find(fileno(udata->getoutFile())) != _m_clients.end())
                _m_clients.erase(fileno(udata->getoutFile()));
            fclose(udata->getinFile());
            fclose(udata->getoutFile());
            
            udata->setState(WRITE_RESPONSE);
            res.setErrorCode(500);//500 Internal Server Error: 서버에서 처리 중에 예기치 않은 오류가 발생한 경우
            _M_changeEvents(_m_change_list, udata->getRequestedFd(),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
            return ;
        }
    } else {
        if (_m_clients.find(fileno(udata->getinFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getinFile()));
        if (_m_clients.find(fileno(udata->getoutFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getoutFile()));
        fclose(udata->getinFile());
        fclose(udata->getoutFile());
        std::cout << "process ennded with some signal" << std::endl;
        udata->setState(WRITE_RESPONSE);
        res.setErrorCode(500);//500 Internal Server Error: 서버에서 처리 중에 예기치 않은 오류가 발생한 경우
        _M_changeEvents(_m_change_list, udata->getRequestedFd(),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    }
    std::cout << " =========next outfile ======== " << fileno(udata->getoutFile()) << std::endl;
    lseek(fileno(udata->getoutFile()), 0, SEEK_SET);
    udata->setState(READ_CGI_RESULT);
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
            req.show_save();
            res.setErrorCode(req.getErrorCode());
            udata->setState(WRITE_RESPONSE);
            _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE , EV_ADD | EV_ONESHOT, 0, 0, udata);
            break;
        case REQUEST_FINISH:
            std::cout << "REQUEST_FINISH then req.show_save()" << std::endl;
            req.show_save();
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
    bool        isBodyOk = true;

    /* HOST 는 반드시 필요한 헤더이므로 있는지 없는지 확인하는 작업이 필요하다 존재하지않으면 40x error 나중에 header parsing 이 끝나면 check_valid 함수를 새로 파서 체크하는게 더 예쁠듯?*/
    if (header.find("HOST") == header.end()){
        std::cout << "HOST is not available " << std::endl;
        res.setErrorCode(WRONG_PARSING);// 400 error
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
        ports = "80";
    } else {
        serverName = host.substr(0, host.find_first_of(":"));
        ports = host.substr(host.find_first_of(":") + 1 );
    }
    url = req.getUrl();
    std::cout << "url : " << url << std::endl;
    std::cout << ports << std::endl;
    std::cout << serverName << std::endl;
    /* default server 에 대한 생각이 필요하다  && location block 을 찾지못할 경우에 그냥 바로 검색 */
    serv = _M_findServerPort(ports, serverName);
    if (serv.valid == false){
        std::cout << " can't find server block " << std::endl; // can't find server block return error
        res.setErrorCode(404);//404 Not Found: 클라이언트가 요청한 리소스가 서버에 없는 경우
        udata->setState(WRITE_RESPONSE);
        _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    } else {
        loca = _M_findLocationBlock(serv, url);
        if (loca.valid == false)
            loca = serv; // location block 을 찾을 수 없기에 serv 블록에 환경대로 실행한다.
    }
    
    /* check is CGI */
    if (serv.key_and_value.find("cgi") != serv.key_and_value.end()){
        std::vector<std::string> cgi_value = serv.key_and_value.find("cgi")->second;
        std::string temp;

        if (loca.block_name == cgi_value.back())
            req.setIsCgi(true);
        else {
            temp = req.getUrl();
            for (int i = 0; i < cgi_value.size() - 1 ; ++ i){ 
                if (temp.length() < cgi_value[i].length())
                    ;
                else if (temp.substr(temp.length() - cgi_value[i].length()) == cgi_value[i]) {
                    std::string temp = cgi_value.back();
                    loca = _M_findLocationBlock(serv, temp);
                    req.setIsCgi(true);
                    break ;
                }
            }
        }
    }

    if (loca.valid != false){
        if (loca.key_and_value.find("cgi_pass") != loca.key_and_value.end()){
            req.setIsCgi(true);
            serverUrl = loca.key_and_value["cgi_pass"].front();
        }
    }

    /* START CGI_PROCESS */
    if (req.getIsCgi() == true) {
        /* check metohd is allowed */
        if (!_M_checkMethod(serv, loca, req.getMethod())){
            if(req.getMethod().compare("HEAD") == 0)
                res.setAddHead(false);
            udata->setState(WRITE_RESPONSE);
            res.setErrorCode(405);
            _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
            return ;
        }
        /* check Body Size */ 
        if (req.getMethod().compare("POST") == 0) {
            std::cout << " ========== CHECK SIZE ================= " << std::endl;
            if (loca.key_and_value.find("client_max_body_size") != loca.key_and_value.end())
                isBodyOk = req.checkBodySize(loca);
            else if (serv.key_and_value.find("client_max_body_size") != serv.key_and_value.end())
                isBodyOk = req.checkBodySize(serv);
            if (isBodyOk == false){
                std::cout << "============== is BODY IS FALSE =============== " << std::endl;
                udata->setState(WRITE_RESPONSE);
                res.setErrorCode(413);//413 Payload Too Large: 클라이언트가 요청한 엔티티가 서버에서 처리할 수 있는 최대 크기를 초과한 경우
                _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
        }

        if (loca.key_and_value.find("alias") != loca.key_and_value.end())
            serverUrl = *loca.key_and_value["alias"].begin() + url;
        else if (loca.key_and_value.find("root") != loca.key_and_value.end())
            serverUrl = (*loca.key_and_value["root"].begin()) + loca.block_name + url;
        
        if (url == ""){
            /* index 의 마지막까지 순회하면서 맞는파일이 있는지 확인하도록 수정해야함 / done */
            bool checkIndex = false;
            std::string tempServerUrl = serverUrl;

            if (loca.key_and_value.find("index") != loca.key_and_value.end()){
                std::cout << "find index " << std::endl;
                std::vector<std::string> &temp = loca.key_and_value.find("index")->second;
                for (int i = 0; i < temp.size(); ++i) {
                    tempServerUrl =  serverUrl + temp[i];
                    std::cout << serverUrl << std::endl;
                    std::ifstream fileStream(tempServerUrl.c_str());
                    if (fileStream.good()) {
                        std::cout << "File exists." << std::endl;
                        serverUrl = tempServerUrl;
                        checkIndex = true;
                        break;
                    } else {
                        std::cout << "File does not exist." << std::endl;
                    }
                }
            }
            if (checkIndex == false){
                std::cout << "index is not available" << std::endl;        
            }
        }
        req.setServerUrl(serverUrl);
        std::cout << "==================== server ulr ================== " << std::endl;
        std::cout << serverUrl << std::endl;
        /* 함수 나누면 좋을듯? */
        FILE    *outFile = tmpfile();
        FILE    *inFile = tmpfile();
        int     infd = fileno(inFile);
        int     outfd = fileno(outFile);

        udata->setinFile(inFile);
        udata->setoutFile(outFile);
        udata->setRequestedFd(curr_event.ident);

        if (req.getMethod().compare("HEAD") == 0)
            res.setAddHead(false);

        /* body의 존재 유무에 따라서 body를 넣어주고 실행할지 바로 실행할지 결정한다 */
        std::cout << "============= METHOD ==============" << req.getMethod() << std::endl;
         if (req.getBody().size() != 0 && (req.getMethod().compare("POST") == 0 || req.getMethod().compare("PUT") == 0) ) {
            std::cout << "========== WRITE_CGI_BODY ===========" << std::endl;
            std::cout << fileno(inFile) << "  " << fileno(outFile) << std::endl;
            udata->setState(WRITE_CGI_BODY);
            _M_changeEvents(_m_change_list, infd,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        }
        else {
            udata->setState(EXCUTE_CGI);
            _M_changeEvents(_m_change_list, outfd,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
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
        /* check metohd is allowed */ 
        if (!_M_checkMethod(serv, loca, req.getMethod())){
            if(req.getMethod().compare("HEAD") == 0)
                res.setAddHead(false);
            udata->setState(WRITE_RESPONSE);
            res.setErrorCode(405);
            _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
            return ;
        }
        /* check_Body_size */
        if (req.getMethod().compare("POST") == 0) {
            if (loca.key_and_value.find("client_max_body_size") != loca.key_and_value.end())
                isBodyOk = req.checkBodySize(loca);
            else if (serv.key_and_value.find("client_max_body_size") != serv.key_and_value.end())
                isBodyOk = req.checkBodySize(serv);
            if (isBodyOk == false){
                res.setErrorCode(413);//413 Payload Too Large: 클라이언트가 요청한 엔티티가 서버에서 처리할 수 있는 최대 크기를 초과한 경우
                _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
        }
        
        /*check location is redirection block*/
        if(loca.valid != false){
            if (loca.key_and_value.find("return") != loca.key_and_value.end()){
                std::vector< std::string > temp = loca.key_and_value["return"];
                res.setErrorCode(std::atoi(temp[0].c_str()));
                req.setRedirectUrl(req.changeRedirectUrl(temp[1]));
                udata->setState(WRITE_RESPONSE);
                _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
        } else {
            if (serv.key_and_value.find("return") != serv.key_and_value.end()){
                std::vector< std::string > temp = serv.key_and_value["return"];
                res.setErrorCode(std::atoi(temp[1].c_str()));
                req.setRedirectUrl(temp[2]);
                res.setErrorCode(301);
                udata->setState(WRITE_RESPONSE);
                _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
        }
        
        /* SERVING STATIC HTML FILE && NEED CHECK METHOD IS ALLOWED */
        if (loca.key_and_value.find("alias") != loca.key_and_value.end())
            serverUrl = *loca.key_and_value["alias"].begin() + url;
        else if (loca.key_and_value.find("root") != loca.key_and_value.end())
            serverUrl = (*loca.key_and_value["root"].begin()) + loca.block_name + url;
        
        if (url == ""){
            /* index 의 마지막까지 순회하면서 맞는파일이 있는지 확인하도록 수정해야함 / done */
            bool checkIndex = false;
            std::string tempServerUrl = serverUrl;

            if (loca.key_and_value.find("index") != loca.key_and_value.end()){
                std::cout << "find index " << std::endl;
                std::vector<std::string> &temp = loca.key_and_value.find("index")->second;
                for (int i = 0; i < temp.size(); ++i) {
                    tempServerUrl =  serverUrl + temp[i];
                    std::cout << serverUrl << std::endl;
                    std::ifstream fileStream(tempServerUrl.c_str());
                    if (fileStream.good()) {
                        std::cout << "File exists." << std::endl;
                        serverUrl = tempServerUrl;
                        checkIndex = true;
                        break;
                    } else {
                        std::cout << "File does not exist." << std::endl;
                    }
                }
            }
            if (checkIndex == false){
                std::cout << "index is not available" << std::endl;        
            }
        }

        std::cout << "server url is : " << serverUrl << std::endl;
        if (req.getMethod() == "POST")
        {
            int fd = _M_openPOST(serverUrl);

            if (fd >= 0)
            {
                fcntl(fd, F_SETFL, O_NONBLOCK);
                udata->setState(WRITE_FILE);
                udata->setRequestedFd(curr_event.ident);
                _M_changeEvents(_m_change_list, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
            else
            {
                std::cout << "POST open error" << std::endl;
                res.setErrorCode(500);
                udata->setState(WRITE_RESPONSE);
                _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
        }
        else if (req.getMethod() == "PUT")
        {
            int fd = _M_openPUT(serverUrl);

            if (fd >= 0)
            {
                fcntl(fd, F_SETFL, O_NONBLOCK);
                udata->setState(WRITE_FILE);
                udata->setRequestedFd(curr_event.ident);
                _M_changeEvents(_m_change_list, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
            else if (fd == -1)
            {
                std::cout << "PUT open error" << std::endl;
                res.setErrorCode(500);
                udata->setState(WRITE_RESPONSE);
                _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
            else if (fd == -2)
            {
                std::cout << "PUT open error" << std::endl;
                res.setErrorCode(403);
                udata->setState(WRITE_RESPONSE);
                _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
        }
        else if (req.getMethod() == "DELETE")
        {
            if (std::remove(serverUrl.c_str()) != 0)
            {
                res.setErrorCode(500);
                udata->setState(WRITE_RESPONSE);
                _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
        }
        else
        {
            int fd = _M_openDocs(serverUrl);
            //int fd = open(serverUrl.c_str(), O_RDONLY);
            if (fd != -1 && fd != -2){
                if (req.getMethod().compare("HEAD") == 0)
                    res.setAddHead(false);
                _m_clients[fd] = "";
                fcntl(fd, F_SETFL, O_NONBLOCK);
                udata->setState(READ_DOCS);
                udata->setRequestedFd(curr_event.ident);
                _M_changeEvents(_m_change_list, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            } else if (fd == -2){
                std::cout << "open error" << std::endl;
                req.setErrorCode(404);
                udata->setState(WRITE_RESPONSE);
                _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
                return ;
            }
            // open error == 404 error //
            std::cout << "open error" << std::endl;
            res.setErrorCode(404);
            udata->setState(WRITE_RESPONSE);
            _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
            return ;
        // _M_disconnectClient(curr_event, _m_clients);
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
        res.appendResponse(data);
        totalBytesRead += bytesRead; // 읽은 바이트 수 누적
    }

    if (bytesRead < 0){
        udata->setState(WRITE_RESPONSE);
        res.setErrorCode(500);// 500 Internal Server Error: 서버에서 처리 중에 예기치 않은 오류가 발생한 경우
        _M_changeEvents(_m_change_list, udata->getRequestedFd(),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    }
    res.setErrorCode(200);
    res.addHeader("Content-type", "text/html; charset=UTF-8");
    close(curr_event.ident);
    udata->setState(WRITE_RESPONSE);
    _M_changeEvents(_m_change_list, udata->getRequestedFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

void ServerEngine::writeFile(struct kevent& curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Response &res = udata->getResponse();
    Request &req = udata->getRequest();

    if (write(curr_event.ident, req.getBody().c_str(), req.getBody().length() < 0)) {
        res.setErrorCode(500);
        udata->setState(WRITE_RESPONSE);
        _M_changeEvents(_m_change_list, udata->getRequestedFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        close(close(curr_event.ident));
    }
    
    close(curr_event.ident);
    udata->setState(WRITE_RESPONSE);
    _M_changeEvents(_m_change_list, udata->getRequestedFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

void ServerEngine::readCgiResult(struct kevent& curr_event){
    /* read udata->outfile and make response */
    std::cout << "READ CGI RESULT " << std::endl;
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Request& req = udata->getRequest();
    Response& res = udata->getResponse();
    char *buf;

    int outFilefd = fileno(udata->getoutFile());
    int size = lseek(outFilefd, 0, SEEK_END);
    
    if (size == -1){
        if (_m_clients.find(fileno(udata->getinFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getinFile()));
        if (_m_clients.find(fileno(udata->getoutFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getoutFile()));
        fclose(udata->getinFile());
        fclose(udata->getoutFile());
        

        udata->setState(WRITE_RESPONSE);
        res.setErrorCode(500);// 500 Internal Server Error: 서버에서 처리 중에 예기치 않은 오류가 발생한 경우
        _M_changeEvents(_m_change_list, udata->getRequestedFd(),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;        
    }
    
    lseek(outFilefd, 0, SEEK_SET);
    buf = new char[size + 1];
    if (buf == 0){
        if (_m_clients.find(fileno(udata->getinFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getinFile()));
        if (_m_clients.find(fileno(udata->getoutFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getoutFile()));
        fclose(udata->getinFile());
        fclose(udata->getoutFile());

        udata->setState(WRITE_RESPONSE);
        res.setErrorCode(500);// 500 Internal Server Error: 서버에서 처리 중에 예기치 않은 오류가 발생한 경우
        _M_changeEvents(_m_change_list, udata->getRequestedFd(),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    }
    int readsize = read(outFilefd, buf, size);
    
    if (readsize != size){
        if (_m_clients.find(fileno(udata->getinFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getinFile()));
        if (_m_clients.find(fileno(udata->getoutFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getoutFile()));
        fclose(udata->getinFile());
        fclose(udata->getoutFile());
        delete []buf;

        udata->setState(WRITE_RESPONSE);
        res.setErrorCode(500);// 500 Internal Server Error: 서버에서 처리 중에 예기치 않은 오류가 발생한 경우
        _M_changeEvents(_m_change_list, udata->getRequestedFd(),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    }
    
    // buf[readsize] = '\0';
    std::string str = std::string(buf, readsize);
    std::cout << "================== CGI RESULT ================" <<std::endl;
    // sleep(3);
    // std::cout << str << std::endl;
    // std::cout << "write str" << std::endl;
    // sleep(3);
    
    udata->getResponse().setResponseByCgiResult(str);
    
    delete []buf;
    /* 파일의 역할을 모두 했으니 여기서 close */
    if (_m_clients.find(fileno(udata->getinFile())) != _m_clients.end())
            _m_clients.erase(fileno(udata->getinFile()));
    if (_m_clients.find(fileno(udata->getoutFile())) != _m_clients.end())
        _m_clients.erase(fileno(udata->getoutFile()));
    fclose(udata->getinFile());
    fclose(udata->getoutFile());
    res.setErrorCode(200);
    udata->setState(WRITE_RESPONSE);
    std::cout << "READ CGI RESULT END" << std::endl;
    _M_changeEvents(_m_change_list, udata->getRequestedFd(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

void ServerEngine::writeResponse(struct kevent& curr_event){
    std::cout << "WRITE RESPONSE IS OCCURED " << std::endl;
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Response &res = udata->getResponse();
    Request &req = udata->getRequest();
    std::string responseString;
    int &totalSendedBytes = res.getTotalSendedBytes();
    int bytes_written = 0;
    
    if (totalSendedBytes == 0){
        if (req.getErrorCode() != OK)
            res.setErrorCode(req.getErrorCode());
        res.setRedirectUrl(req.getRedirectUrl());
        if (res.getErrorCode() != OK)
            res.setResponseByErrorCode();
        else
            res.addBasicHeader();
    }
    responseString = res.getResponse();

    const char* ret = responseString.c_str();
    int len = responseString.length();
    // std::cout << len << std::endl;
    while (totalSendedBytes != len)
    {
        std::string temp = responseString.substr(totalSendedBytes);
        const char* ret = temp.c_str();
        bytes_written = write(curr_event.ident, ret, temp.length());
        if (bytes_written < 0){
            std::cout << "============= bytes_writen error =============== " << std::endl;
            _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
            return ;
        } else {
            totalSendedBytes += bytes_written;
            std::cout << "total sended bytes: " << totalSendedBytes << std::endl;
        }
    }
    udata->clean();
    std::cout << "WRITE RESPONSE IS OCCURED " << std::endl;
    std::cout << "================= END RESPONSE WAITING ANOTHER REQUEST =====================" << std::endl;
    _M_disconnectClient(curr_event, _m_clients);
    // _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, curr_event.udata);
    return ;
}

void ServerEngine::excuteCgi(struct kevent& curr_event){
    std::cout << "EXCUTE CGI IS OCCURED " << std::endl;
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Request&   req = udata->getRequest();
    CgiHandler cgiHandler(req, fileno(udata->getinFile()), fileno(udata->getoutFile()));

    int pid;
    
    pid = cgiHandler.executeCgi();
    std::cout << "============ pid ========= : " << pid << std::endl;
    udata->setState(WAIT_CGI_END);
    _M_changeEvents(_m_change_list, pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, udata);
    std::cout << "EXCUTE CGI IS DONE " << std::endl;
}

void ServerEngine::writeCgiBody(struct kevent& curr_event){
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);

    Request &req = udata->getRequest();
    Response &res = udata->getResponse();
    std::string str = req.getBody();

    if (write(curr_event.ident, str.c_str(), str.size()) == -1){
        udata->setState(WRITE_RESPONSE);
        res.setErrorCode(500);// 500 Internal Server Error: 서버에서 처리 중에 예기치 않은 오류가 발생한 경우
        _M_changeEvents(_m_change_list, udata->getRequestedFd(),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    }
    lseek(curr_event.ident, 0, SEEK_SET);
    udata->setState(EXCUTE_CGI);
    _M_changeEvents(_m_change_list, fileno(udata->getoutFile()),  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);   
}