/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/29 18:51:49 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/04/04 17:26:06 by jaehyuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

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
    this->_m_env["PATH_INFO"] = "/test/cgi_tester"; // configure
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
    this->_m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->_m_env["SERVER_SOFTWARE"] = "webserv/1.0";
}

char    **CgiHandler::_M_get_envArr() const {
    char    **env = new char*[this->_m_env.size() + 1];

    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = this->_m_env.begin(); it != this->_m_env.end(); it++){
        std::string str = it->first + "=" + it->second;
        env[i] = new char[str.length() + 1];
        memset(env[i], 0, str.length() + 1);
        env[i] = strcpy(env[i], str.c_str());
        i++;
    }
    env[i] = NULL;    
    i = 0;
    while (env[i])
    {
        std::cout << env[i] << std::endl;
        i++;
    }
    return (env);
}

#include <unistd.h>

std::string CgiHandler::executeCgi() {
    char        **env = _M_get_envArr();
    std::string body = this->_m_request.getBody();
    const char  *script_name = this->_m_env.find("SCRIPT_NAME")->second.c_str();
    int         stdinBackup = dup(STDIN);
    int         stdoutBackup = dup(STDOUT);
    FILE        *inFile = tmpfile();
    FILE        *outFile = tmpfile();
    int         inFd = fileno(inFile);
    int         outFd = fileno(outFile);
    int         pid;
    std::string rv;
    
    write(inFd, body.c_str(), body.length());
    pid = fork();
    if (pid == 0)
    {
        dup2(inFd, STDIN);
        execve(script_name, NULL, env);
        write(2, "\nEXECUTE ERROR!!\n", strlen("EXECUTE ERROR!!\n"));
    }
    else
    {
        char    buf[BUF_SIZE] = {0};
        int     len_read = 1;

        dup2(outFd, STDOUT);
        waitpid(-1, 0, 0);
        while (len_read > 0)
        {
            memset(buf, 0, BUF_SIZE);
            len_read = read(outFd, buf, BUF_SIZE - 1);
            rv += buf;
        }
    }
    dup2(STDIN, stdinBackup);
    dup2(STDOUT, stdoutBackup);
    fclose(inFile);
    fclose(outFile);
    close(inFd);
    close(outFd);
    return (rv);
}