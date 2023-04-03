/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/29 18:51:49 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/04/03 18:28:11 by jaehyuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

CgiHandler::CgiHandler(){}

CgiHandler::CgiHandler(Request &request)
    : _m_request(request)
{
    _M_initEnv(this->_m_request);

}

void    CgiHandler::_M_initEnv(Request &request)
{
    //std::map< std::string, std::string > header = request.getHeader();
    //this->_m_env["AUTH_TYPE"] = header["AUTHORIZATION"].begin();
    //this->_m_env["CONTENT_LENGTH"] = std::to_string(request._m_body.length());
    //this->_m_env["CONTENT_TYPE"] = header["CONTENT-TYPE"].begin();
    //this->_m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    //this->_m_env["PATH_INFO"] = "./cgi-bin/"; ///////???
    //this->_m_env["PATH_TRANSLATED"] = ""; //////////????
    //this->_m_env["QUERY_STRING"] = request.getQueryString();
    //this->_m_env["REMOTE_ADDR"] = "";
    //this->_m_env["REMOTE_HOST"] = "";
    //this->_m_env["REMOTE_PORT"] = "";
    //this->_m_env["REMOTE_USER"] = "";
    //this->_m_env["REQUEST_METHOD"] = request.getMethod();
    //this->_m_env["REQUEST_URI"] = request.getUrl;
    //this->_m_env["SCRIPT_NAME"] = "";
    //this->_m_env["SERVER_NAME"] = "";
    //this->_m_env["SERVER_PORT"] = "";
    //this->_m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    //this->_m_env["SERVER_SOFTWARE"] = "";
    this->_m_env["AUTH_TYPE"] = "Basic"; //헤더
    this->_m_env["CONTENT_LENGTH"] = "100";
    this->_m_env["CONTENT_TYPE"] = "application/json"; //헤더
    this->_m_env["GATEWAY_INTERFACE"] = "CGI/1.1"; //우리꺼
    this->_m_env["PATH_INFO"] = "/test/cgi_tester"; "/test/cgi_tester.sh" // configure
    this->_m_env["PATH_TRANSLATED"] = "./cgi-bin/cgi_tester"; // configure
    this->_m_env["QUERY_STRING"] = "sortBy=name&limit=10";
    this->_m_env["REMOTE_ADDR"] = "127.0.0.1"; //??
    this->_m_env["REMOTE_HOST"] = "localhost"; //??
    this->_m_env["REMOTE_PORT"] = "8080"; //??
    this->_m_env["REMOTE_USER"] = "jaehyuki"; //어디서오는지 모름??
    this->_m_env["REQUEST_METHOD"] = "GET";
    this->_m_env["REQUEST_URI"] = "http://localhost:4242/test/cgi_tester?sortBy=name&limit=10";
    this->_m_env["SCRIPT_NAME"] = "./cgi-bin/cgi_tester"; // configure
    this->_m_env["SERVER_NAME"] = "localhost"; // 헤더 
    this->_m_env["SERVER_PORT"] = "4242"; // 헤더
    this->_m_env["SERVER_PROTOCOL"] = "HTTP1.1";
    this->_m_env["SERVER_SOFTWARE"] = "webServ";
}

char    **CgiHandler::_M_get_envArr() const {
    char    **env = new char*[this->_m_env.size() + 1];

    int i = 0;
    for (std::map<std::string, std::string>::iterator it = this->_m_env.begin(); it != this->_m_env.end(); it++){
        std::string str = it->first + "=" + it->second;
        env[i] = new char[str.length() + 1];
        env[i] = strcpy(env[i], str.c_str());
    }
    env[i] = NULL;
    
    return (env);
}

std::string CgiHandler::executeCgi() {
    char        **env = _M_get_envArr();
    std::string body = this->request.getBody();
    char        *script_name = this->_m_env.find("SCRIPT_NAME").c_str();
    int         stdin_backup;
    int         stdout_backup;
    int         fd[2];
    int         pid;
    std::string rv;


    stdin_backup = dup(STDIN);
    stdout_backup = dup(STDOUT);
    pipe(fd);
    pid = fork();
    if (pid == 0)
    {
        write(STDIN, body.c_str(), body.length());
        close(fd[0]);
        dup2(fd[1], STDOUT);
        execve(script_name, NULL, env);
    }
    else
    {
        char    buf[BUF_SZIE] = {0};
        int     len_read = 1;

        while (len_read > 0)
        {
            memset(buf, 0, BUF_SIZE);
            len_read = read(fd[0], buf, BUF_SIZE - 1);
            rv += buf;
        }
    }

    return (rv);
}