#include "ServerEngine.hpp"


void ServerEngine::_M_change_events(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_event);
}

void ServerEngine::_M_disconnect_client(int client_fd, std::map<int, std::string>& clients)
{
    std::cout << "client disconnected: " << client_fd << std::endl;
    close(client_fd);
    clients.erase(client_fd);
}

struct server_config_struct
ServerEngine::_M_find_server_and_port(std::string _ports, std::string _server_name)
{
    std::vector<struct server_config_struct>::iterator begin = _m_server_config_set.begin();
    std::vector<struct server_config_struct>::iterator end = _m_server_config_set.end();

    for(; begin != end; ++begin)
    {
        if ((*begin).key_and_value.find(_ports) != (*begin).key_and_value.end())
        {
            if ((*begin).key_and_value.find(_server_name) != (*begin).key_and_value.end())
            {
                return (*begin);
            }
        }
    }
    /* return default server ... need change */
    return (*(_m_server_config_set.begin()));
}

struct server_config_struct
ServerEngine::_M_find_location_block(struct server_config_struct &_server_block, std::string &_url)
{
    std::cout << "========== find location block =========== " << std::endl;
    size_t pos = 0;
    std::string str;
    std::string temp;
    struct server_config_struct ret;
    
    pos = _url.find_first_of("/", 0);
    temp = _url.substr(pos);
    while(pos != std::string::npos)
    {
        std::cout << "pos : " << pos << std::endl;
        std::cout << "temp : " << temp << std::endl;
        str = _url.substr(0, pos + 1);
        if (_server_block.location_block.find(str) == _server_block.location_block.end())
            break ;
        ret = _server_block.location_block[str];
        temp = _url.substr(pos + 1);
        pos = _url.find_first_of("/", pos + 1);        
    }
    _url = temp;
    /* return default server ... need change */
    return (ret);
}

void ServerEngine::set_config_set(std::vector<struct server_config_struct> _config)
{
    _m_server_config_set = _config;
}

KqueueUdata* ServerEngine::_M_make_udata(int state = 0)
{
    KqueueUdata* data = new KqueueUdata();
    data->setState(state);
    return data;
}

///////////////////////////
/* SWITCH CASE FUNCTIONS */
///////////////////////////

void ServerEngine::_M_make_client_socket(struct kevent *curr_event){
    int client_socket;
    
    if ((client_socket = accept(curr_event->ident, NULL, NULL)) == -1)
        exit_with_perror("accept() error\n" + std::string(strerror(errno)));
    std::cout << "accept new client: " << client_socket << std::endl;
    fcntl(client_socket, F_SETFL, O_NONBLOCK);

    /* add event for client socket - add read event */
    _M_change_events(_m_change_list, client_socket, EVFILT_READ , EV_ADD | EV_ONESHOT, 0, 0, _M_make_udata(READ_REQUEST));
}


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
        for (int i = 0; i < _m_server_socket.size(); i ++)
            close(_m_server_socket[i]);
    }
}

/* read file and make request form */
int ServerEngine::_M_solve_request()
{
    return (0);
}


void ServerEngine::_M_read_request(struct kevent& _curr_event, Request& req)
{

    char buf[1024];

    int n = read(_curr_event.ident, buf, sizeof(buf) - 1);
    if (n <= 0) {
        if (n < 0)
            std::cerr << "client read error!" << std::endl;
        std::cerr << "read out\n";
        _M_disconnect_client(_curr_event.ident, _m_clients);
    }
    else {
        size_t len = 0;
        std::string temp;
        std::string temp_buf;
        
        buf[n] = '\0';
        temp_buf = std::string(buf);
        if (req.getCheckHeader() == false){
            if (temp_buf.find("\r\n\r\n") != std::string::npos)
                req.setCheckHeader(true);
        }
        else{//header 가 모두 들어오고 content-length 가 존재하거나 chunked data 가 존재할 때

        }
        temp += std::string(buf);
        req.set_buf(std::string(temp));
        std::cout << temp << std::endl;
    }
}

void ServerEngine::_M_check_request(Request& req)
{
    std::string page;
    size_t client_body_size = -1;
    std::string url = req.get_url();
    std::string host = *(req.get_header().find("Host")->second.begin());
    struct server_config_struct serv_temp;
    struct server_config_struct loca_temp;

    std::string server_name = host.substr(0, host.find_first_of(":", 0));
    std::string ports = host.substr(host.find_first_of(":", 0) + 1);
    // std::cout << "host addr  :" << host.substr(0, host.find_first_of(":", 0)) << std::endl;
    // std::cout << "host port  :" << host.substr(host.find_first_of(":", 0)) << std::endl;
    serv_temp = _M_find_server_and_port(ports, server_name);
    loca_temp = _M_find_location_block(serv_temp, url);
    if (loca_temp.key_and_value.find("client_max_body_size") != loca_temp.key_and_value.end())
    {
        std::string temp;
        temp.clear();
        temp = *(loca_temp.key_and_value.find("client_max_body_size")->second.begin());
        client_body_size = std::atoi(temp.c_str());
        // std::cout << "========= client body size ========== " << std::endl;
        // std::cout << client_body_size << std::endl;
        req.check_body_size(client_body_size);
    }
    
    if (*loca_temp.key_and_value.find("root") != *loca_temp.key_and_value.end())
    {
        page = (*loca_temp.key_and_value["root"].begin()) + url;
    }

    if (url == "")
    {
        /* index 의 마지막까지 순회하면서 맞는파일이 있는지 확인하도록 수정해야함 */
        page = *loca_temp.key_and_value["root"].begin() + *loca_temp.key_and_value["index"].begin();
    }
    // find url at serverConfig file;
    std::cout << "url : " <<  page << std::endl;
}

/* write respose form for client */
int ServerEngine::_M_serve_response()
{
    return (0);
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
    size_t             keyStart;
    size_t             keyEnd;

    std::cout << "Server socket creating... " << std::endl;
    for(; begin != end; ++begin)
    {       
        ports_num = std::atoi(begin->key_and_value.find("listen")->second.begin()->c_str()); // ports_num parsing
        if (duplication_check.find(ports_num) == duplication_check.end())
        {
            std::cout << "ports : " << ports_num << std::endl; 
            if ((new_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
                exit_with_perror("socket() error\n" + std::string(strerror(errno)));
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // if ip config is served need change
            server_addr.sin_port = htons(ports_num); // need config file port option;
            if (bind(new_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
                exit_with_perror("bind() error\n" + std::string(strerror(errno)));
            if (listen(new_socket, 5) == -1)
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
    Request req;
    // Response res;

    if ((_m_kq = kqueue()) ==  -1)
         exit_with_perror("kqueue() error\n" + std::string(strerror(errno)));

    /* add event for server socket */
    for (int i = 0; i != _m_server_socket.size(); i++)
        _M_change_events(_m_change_list, _m_server_socket[i], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, _M_make_udata());
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

            /* check error event return */
            if (curr_event->flags & EV_ERROR)
            {
                if (std::find(_m_server_socket.begin(), _m_server_socket.end(), curr_event->ident) != _m_server_socket.end())
                    exit_with_perror("server socket error");
                else
                {
                    std::cerr << "client socket error" << std::endl;
                    _M_disconnect_client(curr_event->ident, _m_clients);
                }
            }
            else if (curr_event->flags & EV_EOF)
            {
                std::cout << "EOF disconnection \n ";
                _M_disconnect_client(curr_event->ident, _m_clients);
            }
            /* Read Event */
            else if (curr_event->filter == EVFILT_READ)
            {
                /* if server socket then make client socket */
                switch(((KqueueUdata*)(curr_event->udata))->getState())
                {
                    case WAIT_CONNECT:
                        waitConnect(*curr_event);
                        break;
                    case READ_REQUEST:
                        readRequest(*curr_event);
                        break;
                    case READ_URL:
                        break;
                    case READ_CGI_RESULT:
                        break;
                    default:
                        std::cout << "undefined state at read event " << std::endl;
                }

                /* if client socket read contents */
                if (_m_clients.find(curr_event->ident)!= _m_clients.end())
                {
                    std::cout << curr_event->ident <<" : READ EVENT" <<std::endl;
                    /* first htpp request is coming */
                    if (curr_event->udata == NULL)
                    {
                        req = _M_make_request(curr_event);
                        req.parse_and_check_valid();

                        /* if required pages */
                        std::string page;
                        size_t client_body_size = -1;
                        std::string url = req.get_url();
                        std::string host = *(req.get_header().find("Host")->second.begin());
                        struct server_config_struct serv_temp;
                        struct server_config_struct loca_temp;
                        std::string server_name = host.substr(0, host.find_first_of(":", 0));
                        std::string ports = host.substr(host.find_first_of(":", 0) + 1);
                        // std::cout << "host addr  :" << host.substr(0, host.find_first_of(":", 0)) << std::endl;
                        // std::cout << "host port  :" << host.substr(host.find_first_of(":", 0)) << std::endl;
                        serv_temp = _M_find_server_and_port(ports, server_name);
                        loca_temp = _M_find_location_block(serv_temp, url);
                        if (loca_temp.key_and_value.find("client_max_body_size") != loca_temp.key_and_value.end())
                        {
                            std::string temp;
                            temp.clear();                        
                            temp = *(loca_temp.key_and_value.find("client_max_body_size")->second.begin());
                            client_body_size = std::atoi(temp.c_str());
                            std::cout << "========= client body size ========== " << std::endl;
                            std::cout << client_body_size << std::endl;
                            req.check_body_size(client_body_size);
                        }
                        
                        page = (*loca_temp.key_and_value["root"].begin()) + url;
                        if (url == "")
                        {
                            /* need rotating index vector and find right page */
                            page = *loca_temp.key_and_value["root"].begin() + *loca_temp.key_and_value["index"].begin();
                        }

                        // find url at serverConfig file;
                        std::cout << "url : " <<  page << std::endl;
                        int fd = open(page.c_str(), O_RDONLY);
                        if (fd != -1)
                        {
                            _m_clients[fd] = "";
                            fcntl(fd, F_SETFL, O_NONBLOCK);
                            kqueue_data *data = _M_make_udata(curr_event->ident);
                            _M_change_events(_m_change_list, fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, data);
                        }
                        else
                            std::cout << "open error" << std::endl;                            
                        
                            /* if required cgi */
                            // test code, just run sh script and return result
                            // kqueue_data *data = _M_make_udata(curr_event->ident, true);
                            // if (pipe(&(data->fd[0])) == -1)
                            //     std::cout << "pipe error" << std::endl;
                            // fcntl(data->fd[0], F_SETFL, O_NONBLOCK);
                            // fcntl(data->fd[1], F_SETFL, O_NONBLOCK);
                            // _m_clients[data->fd[0]] = "";
                            // _M_change_events(_m_change_list, data->fd[0], EVFILT_READ , EV_ADD | EV_ONESHOT, 0, 0, data);
                            // _M_change_events(_m_change_list, data->fd[1], EVFILT_WRITE , EV_ADD | EV_ONESHOT, 0, 0, data);
                    }
                        
                    /* if udata is not null then read something, save at udata->response and make write events on requested_fd */  
                    else
                    {
                        req = _M_make_request(curr_event);
                        kqueue_data* data = ((kqueue_data*)(curr_event->udata));
                        if (data->is_cgi == true)
                        {
                            close(data->fd[1]);
                        }
                        std::cout << "read html file " << std::endl;
                        std::cout << req.get_buf() << std::endl;
                        //std::cout << ((kqueue_data*)(curr_event->udata))->requested_fd << " : append" << std::endl;
                        _m_clients[data->requested_fd] += (req.get_buf());
                        _M_change_events(_m_change_list, data->requested_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, curr_event->udata);
                        _M_disconnect_client(curr_event->ident, _m_clients);
                    }
                }
                std::cout << curr_event->ident << " : READ EVNET IS DONE" << std::endl;
            }
                
            /* Write Event */
            else if (curr_event->filter == EVFILT_WRITE)
            {
                switch(((KqueueUdata*)(curr_event->udata))->getState())
                {
                    case WRITE_RESPONSE:
                        _M_make_client_socket(curr_event);
                        break;
                    case EXCUTE_CGI:
                        break;
                    default:
                        std::cout << "undefined state at write event " << std::endl;
                }
                /* copying response */
                if ((kqueue_data*)curr_event->udata != 0)
                {
                    if (((kqueue_data*)(curr_event->udata))->is_cgi == true)
                    {
                        int pid = fork();
                        if (pid == 0)
                        {
                            close(((kqueue_data*)(curr_event->udata))->fd[0]);
                            dup2(((kqueue_data*)(curr_event->udata))->fd[1], 1);
                            char *argv[] = {"sh", "test.sh", "\0"};
                            execve("/bin/sh", argv, 0);
                        }
                    }
                    else
                    {
                        _m_clients[curr_event->ident].append(((kqueue_data*)(curr_event->udata))->response);
                        delete curr_event->udata;
                    }
                }

                /* send data to client */
                std::map<int, std::string>::iterator it = _m_clients.find(curr_event->ident);
                if (it != _m_clients.end())
                {
                    if (_m_clients[curr_event->ident] != "")
                    {
                        int n;
                        std::cout << "send data to " << curr_event->ident << ": " << _m_clients[curr_event->ident].c_str() << std::endl;
                        if ((n = write(curr_event->ident, _m_clients[curr_event->ident].c_str(),
                                        _m_clients[curr_event->ident].size()) == -1))
                        {
                            std::cerr << "client write error!" << std::endl;
                            _M_disconnect_client(curr_event->ident, _m_clients);  
                        }
                        else
                            _m_clients[curr_event->ident].clear();
                        _M_change_events(_m_change_list, curr_event->ident, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                    }
                }
                std::cout <<curr_event->ident << " : WRITE EVENT IS DONE " << std::endl;       
            }
        }
    }
    return ;
}