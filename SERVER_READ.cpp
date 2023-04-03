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
                        std::string url = req.getUrl();
                        std::string host = *(req.getHeader().find("Host")->second.begin());
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