#include "Response.hpp"

/* CONSTRUCTOR */
Response::Response(){
    _m_errorPageCode = 0;
    _m_errorCode = RESPONSE_OK;
    _m_addhead = true;
    _m_totalSendedBytes = 0;
    _M_initStatusCodeMap();
    _M_initStatusCodeBodyMap();
}

Response::~Response(){
    
}

/* Setter */

void  Response::setErrorPageCode(int e){
    _m_errorPageCode = e;
}

void Response::setErrorCode(int errorCode){
    _m_errorCode = errorCode;
    setStatusLine(errorCode);
}

void Response::setStatusLine(int errorCode){
    _m_statusLine = "HTTP/1.1 ";
    _m_statusLine.append(std::to_string(errorCode));
    _m_statusLine.append(" ");
    _m_statusLine.append(_m_statusCodeMap[errorCode]);
}

void Response::setHeader(std::map<std::string, std::string> header) {
    _m_header = header;
}

void Response::setAddHead(bool b){
    _m_addhead = b;
}

void Response::setRedirectUrl(std::string url){
    _m_redirectUrl = url;
}

void Response::setServer(struct server_config_struct& server)
{
    _m_server = server;
}

/* Getter */

struct server_config_struct&     Response::getServer(){ return _m_server; }

int&    Response::getTotalSendedBytes() { return _m_totalSendedBytes; }
std::string Response::getStatusLine(){ return _m_statusLine;}
std::map<std::string, std::string> Response::getHeader(){ return _m_header;}

std::string Response::getResponse(){
    std::string response;
    
    response.append(_m_statusLine + "\n");
    for (std::map<std::string, std::string>::const_iterator it = this->_m_header.begin(); it != this->_m_header.end(); it++){
        std::string str = it->first + ": " + it->second;
        response.append(str);
        response.append("\r\n");
    }
    response.append("\r\n");
    if (_m_addhead)
        response.append(_m_response);
    return response;
}

std::string Response::getRedirectUrl()
{
    return _m_redirectUrl;
}

int         Response::getErrorCode()
{
    return _m_errorCode;
}

int  Response::getErrorPageCode(){
    return _m_errorPageCode;
}

/*Functions */

void Response::clean(){
    _m_errorPageCode = 0;
    _m_errorCode = RESPONSE_OK;
    _m_addhead = true;
    _m_totalSendedBytes = 0;
    _m_response.clear();
}

void Response::appendResponse(std::string &str){
    _m_response += str;
} 

void Response::addHeader(std::string headerName, std::string content) {
    ft_toupper(headerName);
    _m_header[headerName] = content;
}

void Response::addBasicHeader() {
    time_t now = time(0);
    char* dt = ctime(&now);

    std::cout << "=============== add basic header ============== " << std::endl;
    std::cout << _m_response.length() << std::endl;
    dt[std::strlen(dt) - 1] = '\0';
    setStatusLine(_m_errorCode);
    _m_header["SERVER"] = "webserv/1.0";
    _m_header["DATE"] = std::string(dt);
    _m_header["CONTENT-LENGTH"] = std::to_string(_m_response.length());
    std::cout << _m_response.length() << std::endl;
    //Content-Type은 리소스 불러올때 적어줘야함
}

void Response::setResponseByCgiResult(std::string cgiResult) {
    std::string delimiter = "\r\n\r\n";
    std::string header;
    std::string body;
    size_t pos = cgiResult.find(delimiter);
    
    if (pos != std::string::npos) {
        // delimiter found
        header = cgiResult.substr(0, pos);
        // body = cgiResult.substr(pos + delimiter.length() , cgiResult.length());
    } else {
        // delimiter not found
    }
    body = cgiResult.substr(pos + 4);
    _M_parseAndSetHeader(header); //set _m_header
    _m_response = body;
}

void Response::_M_initStatusCodeMap(void) {
    _m_statusCodeMap[100] = "Continue";
	_m_statusCodeMap[200] = "OK";
	_m_statusCodeMap[201] = "Created";
	_m_statusCodeMap[204] = "No Content";
    //Redirection
    _m_statusCodeMap[301] = "Moved Permanently";
    _m_statusCodeMap[302] = "Found";
    //Error
	_m_statusCodeMap[400] = "Bad Request";
	_m_statusCodeMap[403] = "Forbidden";
	_m_statusCodeMap[404] = "Not Found";
	_m_statusCodeMap[405] = "Method Not Allowed";
	_m_statusCodeMap[413] = "Payload Too Large";
	_m_statusCodeMap[500] = "Internal Server Error";
}

void Response::_M_initStatusCodeBodyMap(void){
    _m_statusCodeMessageMap[400] = "The server cannot process the request due to a client error.";
    _m_statusCodeMessageMap[401] = "The requested resource requires authentication.";
    _m_statusCodeMessageMap[403] = "You don't have permission to access this resource.";
    _m_statusCodeMessageMap[404] = "The requested resource could not be found.";
    _m_statusCodeMessageMap[405] = "405 Method Not Allowed The method specified\
 in the request is not allowed for the resource identified by the request URI.";
    _m_statusCodeMessageMap[413] = "413 Payload Too Large\
 The request entity is larger than the server is willing or able to process.";
    _m_statusCodeMessageMap[500] = "The server encountered an unexpected condition\
 that prevented it from fulfilling the request.";
}

void    Response::ErrorCodeBody(int errorCode)
{
    std::string body;

    body += "<!DOCTYPE html>\n";
    body += "<html>\n";
    body += "<head>\n";
    body += "   <title>" + std::to_string(errorCode)
        + " " + _m_statusCodeMap[errorCode]
        + "</title>\n";
    body += "</head>\n";
    body += "<body>\n";
    body += "   <h1>"+ std::to_string(errorCode)
        + " " + _m_statusCodeMap[errorCode]
        + "</h1>\n";
    body += "   <p>" + _m_statusCodeMessageMap[errorCode] + "</p>\n";
    body += "</body>\n";
    body += "</html>";
    body += "\n";
    appendResponse(body);
}

void Response::_M_parseAndSetHeader(std::string header) {
    bool    valid = true;

    std::vector<std::string> lines;
    std::string line;
    std::istringstream lineStream(header);
    while (std::getline(lineStream, line, '\n')) {
        lines.push_back(line);
    }
    std::map<std::string, std::string> headerMap;
    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); it++){
        size_t pos = it->find(":");
        std::string headerName;
        std::string content;
        if (pos != std::string::npos) {
            headerName = (*it).substr(0, pos);
            ft_toupper(headerName);
            content = (*it).substr(pos + 1, (*it).length());
            headerMap[headerName] = content;
        } else {
            valid = false;
        }
    }
    if (headerMap.find("CONTENT-TYPE") == headerMap.end()){
        std::cout << "can't find content -type " << std::endl;
        valid = false;
    }
    if (valid == false)
        setResponseByErrorCode(); // ERROR CODE DEFINE??
    else

        _m_header = headerMap;
}

void    Response::setResponseByErrorCode() {
    setStatusLine(_m_errorCode);
    if (_m_errorCode == 301 || _m_errorCode == 302){
        addHeader("location", getRedirectUrl());
        return ;
    }
    if (_m_addhead && _m_response.empty())
    {
        ErrorCodeBody(_m_errorCode);
    }
    addBasicHeader();
}
