#include "ServerEngine.hpp"

int ServerEngine::_M_openDocs(std::string serverUrl)
{
    int fd;
    struct stat file_stat;

    if (stat(serverUrl.c_str(), &file_stat) == -1)
        return (-1);

    if (S_ISDIR(file_stat.st_mode))
        return (-2);
    else if (!S_ISREG(file_stat.st_mode))
        return (-1);
    
    fd = open(serverUrl.c_str(), O_RDONLY);
    if (fd == -1)
        return (-1);

    return (fd);
}

int ServerEngine::_M_openPOST(std::string serverUrl)
{
    int fd = open(serverUrl.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
             S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return (fd);
}

int ServerEngine::_M_openPUT(std::string serverUrl)
{
    struct stat file_stat;

	if (stat(serverUrl.c_str(), &file_stat) != 0)
        return (-3);
    if (!S_ISREG(file_stat.st_mode))
        return (-2);

    int fd = open(serverUrl.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
             S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return (fd);
}

void ServerEngine::_M_autoIndexing(struct kevent &curr_event, std::string serverUrl)
{
    KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event.udata);
    Response &res = udata->getResponse();

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(serverUrl.c_str())) == NULL) {
        res.setErrorCode(404);
        udata->setState(WRITE_RESPONSE);
        _M_changeEvents(_m_change_list, curr_event.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
        return ;
    }

    /*-------Make Body----------*/
    std::string body;
    body += "<!DOCTYPE html>\n";
    body += "<html>\n";
    body += "<head>\n";
    body += "   <title>"
        + std::string("Index of ") + serverUrl
        + "</title>\n";
    body += "</head>\n";
    body += "<body>\n";
    body += "<h1>";
    body += "Index of ";
    body += serverUrl;
    body += "</h1>\n";
    body += "<pre>   Name   Last modified   Size\n";
    body += "<hr>\n";
    while ((ent = readdir(dir)) != NULL) {
        std::string fileTag = "";
        if (ent->d_name[0] != '.') {
            struct stat fileInfo;
            std::string filePath = serverUrl + "/" + std::string(ent->d_name);
            if (stat(filePath.c_str(), &fileInfo) == -1) {
                std::cerr << "Error getting file info for " << filePath << std::endl;
                continue ;
            }
            filePath = filePath.substr(1);
            fileTag += "<img src=\"";
            fileTag += std::string(filePath);
            fileTag += "\"";
            // 파일 타입 출력
            if (S_ISREG(fileInfo.st_mode)) {
                fileTag += "alt=\"[FILE]\">";
            } else if (S_ISDIR(fileInfo.st_mode)) {
                fileTag += "alt=\"[DIR]\">";
            } else {
                fileTag += "alt=\"[???]\">";
            }
            fileTag += "<a href=\"";
            fileTag += filePath;
            fileTag += "\">";
            fileTag += std::string(ent->d_name);
            fileTag += "</a>";
            char buffer[80];
            strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&fileInfo.st_mtime));
            fileTag += "         ";
            fileTag += std::string(buffer);
            fileTag += "         ";
            fileTag += std::to_string(fileInfo.st_size) + " Bytes\n";
        }
        body += fileTag;
    }
    body += "</pre><hr>\n";
    body += "</body>\n";
    body += "</html>";
    body += "\n";
    /*------------------------*/

    closedir(dir);
    res.appendResponse(body);
    //res.addHeader("Content-Security-Policy", "img-src 'none'"); //다른 이미지 소스 요청 블로킹
    res.addHeader("Content-type", "text/html; charset=UTF-8");
    udata->setState(WRITE_RESPONSE);
    res.setErrorCode(200);
    _M_changeEvents(_m_change_list, curr_event.ident,  EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata);
}

void ServerEngine::_M_changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_event);
}

void ServerEngine::_M_disconnectClient(struct kevent& curr_event, std::map<int, std::string>& clients)
{
    std::cout << "client disconnected: " << curr_event.ident << std::endl;
    //shutdown(curr_event.ident, SHUT_WR);
    close(curr_event.ident);
    std::cout << "erase ident" << std::endl;
    if (clients.find(curr_event.ident) != clients.end())
        clients.erase(curr_event.ident);
    std::cout << "erase udata" << std::endl;
    if (curr_event.udata){
        delete (reinterpret_cast<KqueueUdata*> (curr_event.udata));
        curr_event.udata = 0;
    }
    std::cout << " done " << std::endl;
}

bool ServerEngine::_M_checkMethod(struct server_config_struct& serv, struct server_config_struct& loca, std::string method)
{
    std::vector<std::string> temp;
    
    std::cout << "_M_CHECKMETHOD IS OCCURED " << std::endl;
    /* first check location block if not thne check serv block */
    if (loca.valid != false && loca.key_and_value.find("allow") != loca.key_and_value.end()) {
        temp = loca.key_and_value.find("allow")->second;
        for (size_t i = 0; i < temp.size(); ++i) {
            if (method.compare(temp[i]) == 0)
                return true;
        }
        return false;
    } else if (serv.key_and_value.find("allow") != serv.key_and_value.end()){
        temp = serv.key_and_value.find("allow")->second;
        for (size_t i = 0; i < temp.size(); ++i) {
            if (method.compare(temp[i]) == 0)
                return true;
        }
        return false;
    }
    
    if (loca.valid != false && loca.key_and_value.find("deny") != loca.key_and_value.end()) {
        temp = loca.key_and_value.find("deny")->second;
        for (size_t i = 0; i < temp.size(); ++i) {
            if (method.compare(temp[i]) == 0)
                return false;
        }
    } else {
        if (serv.key_and_value.find("deny") != serv.key_and_value.end()){
            temp = serv.key_and_value.find("deny")->second;
            for (size_t i = 0; i < temp.size(); ++i) {
                if (method.compare(temp[i]) == 0)
                    return false;
            }
        }
    }
    return true;
}

struct server_config_struct
ServerEngine::_M_findServerPort(std::string _ports, std::string _server_name)
{
    std::vector<struct server_config_struct>::iterator begin = _m_server_config_set.begin();
    std::vector<struct server_config_struct>::iterator end = _m_server_config_set.end();
    std::vector<std::string> temp;
    struct server_config_struct defaultServer;
    bool findDefaultServer = false;

    if(_server_name == "localhost")
        _server_name = "127.0.0.1";

    for(; begin != end; ++begin)
    { 
        if ((*begin).key_and_value.find("listen")->second.front() == _ports)
        {
            if (findDefaultServer == false){
                findDefaultServer = true;
                defaultServer = *begin;
            }
            temp = (*begin).key_and_value.find("server_name")->second;
            for (size_t i = 0; i < temp.size(); ++ i){
                if (temp[i] == _server_name)
                    return (*begin);
            } 
        }
    }
    /* return default server ... need change */
    // if default serverblock is defined return default block 
    // else return valid false
    return defaultServer;
}

struct server_config_struct
ServerEngine::_M_findLocationBlock(struct server_config_struct &server_block, std::string &url)
{
    std::cout << "========== find location block =========== " << std::endl;
    std::string str;
    std::string temp;
    struct server_config_struct ret;
    bool        valid = false;

    // if location block is empty return valid=false block; 
    if (server_block.location_block.empty()) {
        struct server_config_struct temp;
        temp.valid = false;
        return temp;
    }

    int result;
    size_t max = 0;
    std::map <std::string, struct server_config_struct>::iterator begin = server_block.location_block.begin();
    std::map <std::string, struct server_config_struct>::iterator end = server_block.location_block.end();

    for (; begin != end; ++begin){
        size_t locaLen = begin->first.length();
        size_t urlLen = url.length();

        if (locaLen < urlLen){
            result = begin->first.compare(0, locaLen, url, 0, locaLen);
            if (result == 0){
                if (max < locaLen){
                    valid = true;
                    max = locaLen;
                    temp = url.substr(locaLen);
                    ret = begin->second;
                }
            }
        } else {
            result = begin->first.compare(0, urlLen, url, 0, urlLen);
            if (result == 0){
                if (max < urlLen){
                    valid = true;
                    max = urlLen;
                    temp = url.substr(urlLen);
                    ret = begin->second;
                }
            }
        }
    }
    
    if (valid == false){
        struct server_config_struct temp;
        temp.valid = false;
        return temp;
    } else if (!ret.location_block.empty()){
        struct server_config_struct tempRet;
        std::string temp2 = temp;

        tempRet = _M_findLocationBlock(ret, temp2);
        if (tempRet.valid != false){
            url = temp2;
            return (tempRet);
        }
    }

    url = temp;
    return (ret);
}

void ServerEngine::set_config_set(std::vector<struct server_config_struct> _config)
{
    _m_server_config_set = _config;
}

KqueueUdata* ServerEngine::_M_makeUdata(int state = 0)
{
    KqueueUdata* data = new KqueueUdata();
    data->setState(state);
    return data;
}

///////////////////////////
/* SWITCH CASE FUNCTIONS */
///////////////////////////


ServerEngine::ServerEngine()
{
    std::cout << "Server engins created" << std::endl;
    _m_server_socket.clear();
    _m_kq = -1;
}

ServerEngine::~ServerEngine()
{
    if (_m_server_socket.size() != 0)
    {
        for (size_t i = 0; i < _m_server_socket.size(); i ++)
            close(_m_server_socket[i]);
    }
}

void ServerEngine::_M_readRequest(struct kevent& _curr_event, Request& req)
{

    char buf[1024];

    while (1) {
        int n = read(_curr_event.ident, buf, sizeof(buf) - 1);
        if (n <= 0) {
            if (n < 0){
                std::cerr << "client read error!" << std::endl;
                std::cerr << "read out\n";
                return ;
            }
            std::cout << "read all ! " << std::endl;
            return ;
        }
        else {            
            std::string temp = std::string(buf, n);
            req.appendBuf(temp);
            req.parseBuf();
            if (req.getState() == REQUEST_FINISH || req.getState() == REQUEST_ERROR)
                break;
            memset(buf, 0, sizeof(buf));
        }
        if (n < 1023)
            break;
    }
}

/* make server socket using configFile */
void ServerEngine::make_serversocket()
{
    struct sockaddr_in server_addr;
    int                new_socket;
    int                ports_num;
    std::vector<struct server_config_struct>::iterator begin = _m_server_config_set.begin();
    std::vector<struct server_config_struct>::iterator end = _m_server_config_set.end();
    std::map<int, int> duplication_check; // for check duplication open ports
    std::string        key;
    // struct linger      optLinger;
    int                optVal;

    optVal = 1;
    // optLinger.l_onoff = 1;
    // optLinger.l_linger = 0;
    // 종료함수가 실행되면 안에있는 버퍼들은 모두 폐기하고 바로 종료시킨다.

    std::cout << "Server socket creating... " << std::endl;
    for(; begin != end; ++begin)
    {       
        ports_num = std::atoi(begin->key_and_value.find("listen")->second.begin()->c_str()); // ports_num parsing
        if (duplication_check.find(ports_num) == duplication_check.end())
        {
            std::cout << "ports : " << ports_num << std::endl; 
            if ((new_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
                exit_with_perror("socket() error\n" + std::string(strerror(errno)));
            /* setting socket option REUSEADDR & LINGER */
            if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == -1)
                exit_with_perror("socket() error\n" + std::string(strerror(errno)));
            // if (setsockopt(new_socket, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger)) == -1)
            //     exit_with_perror("socket() error\n" + std::string(strerror(errno)));
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // if ip config is served need change
            server_addr.sin_port = htons(ports_num); // need config file port option;
            if (bind(new_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
                exit_with_perror("bind() error\n" + std::string(strerror(errno)));
            if (listen(new_socket, 128) == -1)
                exit_with_perror("listen() error\n" + std::string(strerror(errno)));
            fcntl(new_socket, F_SETFL, O_NONBLOCK);
            _m_server_socket.push_back(new_socket);
            duplication_check.insert(std::make_pair(ports_num, ports_num));
            std::cout << "server socket is created : " << ports_num << std::endl;
        }
    }
    std::cout << "Server socket is ready" << std::endl;
}

void ServerEngine::start_kqueue()
{
    signal(SIGPIPE, SIG_IGN);
    if ((_m_kq = kqueue()) ==  -1)
         exit_with_perror("kqueue() error\n" + std::string(strerror(errno)));

    /* add event for server socket */
    for (size_t i = 0; i != _m_server_socket.size(); i++)
        _M_changeEvents(_m_change_list, _m_server_socket[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, _M_makeUdata());
    std::cout << "echo server started" << std::endl;
    /* main loop */
    int new_events;
    struct kevent* curr_event;
    while (1)
    {
        /*  apply changes and return new events(pending events) */
        new_events = kevent(_m_kq, &_m_change_list[0], _m_change_list.size(), _m_event_list, 8, 0);
        if (new_events == -1)
            exit_with_perror("kevent() error\n" + std::string(strerror(errno)));
        _m_change_list.clear(); // clear _m_change_list for new changes

        for (int i = 0; i < new_events; ++i)
        {
            curr_event = &_m_event_list[i];

            // 순서에 따라 동작이 달라진다 EVENT filter 와 flags 에 대한 이해가 필요하다.
            /* catch cgi process is end */
            if (curr_event->filter == EVFILT_PROC && curr_event->fflags == NOTE_EXIT){
                std::cout << "CGI PROCESS IS DONE" << std::endl;
                waitCgiEnd(*curr_event);
            }

            /* check error event return */
            else if (curr_event->flags & EV_ERROR) {
                std::cout << "ERROR ! : " << curr_event->ident << std::endl;
                if (std::find(_m_server_socket.begin(), _m_server_socket.end(), curr_event->ident) != _m_server_socket.end())
                    exit_with_perror("server socket error");
                else
                {
                    std::cerr << "client socket error" << std::endl;
                    _M_disconnectClient(*curr_event, _m_clients);
                }
            }

            /* EOF is coming disconnect client */
            else if (curr_event->flags & EV_EOF) {
                std::cout << curr_event->ident << " : EOF disconnection \n ";
                close(curr_event->ident);
                //_M_disconnectClient(*curr_event, _m_clients);
            }

            /* Read Event */
            else if (curr_event->filter == EVFILT_READ) {
                KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event->udata);
                std::cout << curr_event->ident << " : READ EVENT IS OCCUR " << std::endl;

                switch(udata->getState()) {
                    case WAIT_CONNECT:      waitConnect(*curr_event); break;
                    case READ_REQUEST:      readRequest(*curr_event); break;
                    case READ_DOCS:         readDocs(*curr_event); break;
                    case READ_CGI_RESULT:   readCgiResult(*curr_event); break;
                }

                std::cout << curr_event->ident << " : READ EVNET IS DONE" << std::endl;
            }
                
            /* Write Event */
            else if (curr_event->filter == EVFILT_WRITE) {
                KqueueUdata *udata = reinterpret_cast<KqueueUdata *>(curr_event->udata);
                std::cout << curr_event->ident << " : WRITE EVENT IS OCCURED " << std::endl;

                switch(udata->getState()) {
                    case WRITE_FILE:        writeFile(*curr_event); break;
                    case WRITE_RESPONSE:    writeResponse(*curr_event); break;
                    case EXCUTE_CGI:        excuteCgi(*curr_event); break;
                    case WRITE_CGI_BODY:    writeCgiBody(*curr_event); break;
                }
                std::cout <<curr_event->ident << " : WRITE EVENT IS DONE " << std::endl;
            }
        }
    }
    return ;
}
