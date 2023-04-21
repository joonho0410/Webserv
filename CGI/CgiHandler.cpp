/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gyim <gyim@student.42seoul.kr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/29 18:51:49 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/04/21 16:41:43 by gyim             ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

CgiHandler::CgiHandler(Request &request,int infile, int outfile)
    : _m_request(request), _m_inFilefd(infile), _m_outFilefd(outfile)
{
    _M_initEnv(this->_m_request);

}

inline void CgiHandler::_M_findAndInit(std::string env, std::string headerEnv, std::map< std::string, std::vector< std::string > > &header)
{
    if (header.find(headerEnv) != header.end())
        this->_m_env[env] = *(header[headerEnv].begin());
    else
        this->_m_env[env] = "";
}

void    CgiHandler::_M_initEnv(Request &request )
{
    std::cout << "init Env " << std::endl;
    std::map< std::string, std::vector< std::string > > header = request.getHeader();
    std::string hostHeader;
    size_t      deli;
    
    hostHeader = *(header["HOST"].begin());
    deli = hostHeader.find_first_of(":");
    if (deli != std::string::npos){
        this->_m_env["SERVER_NAME"] = hostHeader.substr(0, deli);
        this->_m_env["SERVER_PORT"] = hostHeader.substr(deli + 1);
    } else {
        this->_m_env["SERVER_NAME"] = hostHeader.substr(0, deli);
        this->_m_env["SERVER_PORT"] = "80";
    }
    _M_findAndInit("AUTH_TYPE", "AUTHORIZATION", header);
    _M_findAndInit("CONTENT_TYPE", "CONTENT-TYPE", header);
    this->_m_env["CONTENT_LENGTH"] = std::to_string(request.getBody().length()); 
    this->_m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->_m_env["PATH_INFO"] = request.getUrl();
    this->_m_env["PATH_TRANSLATED"] = request.getServerUrl();
    this->_m_env["QUERY_STRING"] = request.getQueryString();
    this->_m_env["REMOTE_ADDR"] = "";
    this->_m_env["REMOTE_HOST"] = "";
    this->_m_env["REMOTE_PORT"] = "";
    this->_m_env["REMOTE_USER"] = "";
    this->_m_env["REQUEST_METHOD"] = request.getMethod();
    this->_m_env["REQUEST_URI"] = request.getUrl();
    this->_m_env["SCRIPT_NAME"] = request.getServerUrl();
    this->_m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->_m_env["SERVER_SOFTWARE"] = "webserv/1.0";
    this->_m_env["HTTP_X_SECRET_HEADER_FOR_TEST"] = "1";
    
    std::cout << "init Env is DONE " << std::endl;
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

int CgiHandler::executeCgi() {
    char        **env = _M_get_envArr();
    std::string body = this->_m_request.getBody();
    const char  *script_name = this->_m_env.find("SCRIPT_NAME")->second.c_str();
    int         pid;
    std::string rv;
    
    pid = fork();
    if (pid == 0)
    {
        dup2(_m_inFilefd, STDIN);
        dup2(_m_outFilefd, STDOUT);
        execve(script_name, NULL, env);
        write(2, "CGI FAILED\n", 11);
        std::cout << "EXECUTE CGI FAIL!" << std::endl;
    }
    else
    {
        std::cout << "waiting cgi is done" << std::endl;
        // lseek(_m_outFilefd, 0, SEEK_SET);
    }
    std::cout << "executed CGI is done !" << std::endl;
    int i = 0;
    while (env[i] != 0){
        delete env[i];
        ++i;
    }
    delete []env;

    return (pid);
}