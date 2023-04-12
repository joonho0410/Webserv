#include "Response.hpp"


/* CONSTRUCTOR */
Response::Response(){
    _m_errorCode = RESPONSE_OK;
    _M_initStatusCodeMap();
}

Response::~Response(){
    
}

/* Setter */

void Response::setErrorCode(int errorCode){ _m_errorCode = errorCode;}
void Response::setStatusLine(int errorCode){
    _m_statusLine = "HTTP/1.1 ";
    _m_statusLine.append(std::to_string(errorCode));
    _m_statusLine.append(" ");
    _m_statusLine.append(_m_statusCodeMap[errorCode]);
}

void Response::setHeader(std::map<std::string, std::string> header) {
    _m_header = header;
}

/* Getter */

std::string Response::getStatusLine(){ return _m_statusLine;}

std::map<std::string, std::string> Response::getHeader(){ return _m_header;}

std::string Response::getResponse(){
    std::string response;

    response.append(_m_statusLine + "\n");
    for (std::map<std::string, std::string>::const_iterator it = this->_m_header.begin(); it != this->_m_header.end(); it++){
        std::string str = it->first + ": " + it->second;
        response.append(str);
        if (std::next(it) != _m_header.end())
            response.append("\n");
    }
    response.append("\r\n");
    response.append(_m_response);
    return response;
}

/*Functions */

void Response::clean(){
    _m_errorCode = RESPONSE_OK;
    _m_response.clear();
}

void Response::appendResponse(std::string &str){
    _m_response += str;
} 

void Response::addHeader(std::string headerName, std::string content) {
    _m_header[headerName] = content;
}

void Response::addBasicHeader() {
    time_t now = time(0);
    char* dt = ctime(&now);

    _m_header["Server"] = "webserv/1.0";
    _m_header["Date"] = std::string(dt);
    _m_header["Content-Length"] = _m_response.length();
    //Content-Type은 리소스 불러올때 적어줘야함
}

void Response::setResponseByCgiResult(std::string cgiResult) {
    std::string delimiter = "\r\n";
    std::string header;
    std::string body;
    size_t pos = cgiResult.find(delimiter);

    if (pos != std::string::npos) {
        // delimiter found
        header = cgiResult.substr(0, pos);
        body = cgiResult.substr(pos + delimiter.length() + 1 , cgiResult.length());
    } else {
        // delimiter not found
    }
    _M_parseAndSetHeader(header); //set _m_header
    _m_response = body;
}

void Response::_M_initStatusCodeMap(void) {
    _m_statusCodeMap[100] = "Continue";
	_m_statusCodeMap[200] = "OK";
	_m_statusCodeMap[201] = "Created";
	_m_statusCodeMap[204] = "No Content";
	_m_statusCodeMap[400] = "Bad Request";
	_m_statusCodeMap[403] = "Forbidden";
	_m_statusCodeMap[404] = "Not Found";
	_m_statusCodeMap[405] = "Method Not Allowed";
	_m_statusCodeMap[413] = "Payload Too Large";
	_m_statusCodeMap[500] = "Internal Server Error";
}

void Response::_M_parseAndSetHeader(std::string header) {
    bool    valid = true;

    std::vector<std::string> lines;
    std::string line;
    std::istringstream lineStream(header);
    while (std::getline(lineStream, line, '\n')) {
        lines.push_back(line);
    }
    _m_statusLine = *lines.begin();
    std::map<std::string, std::string> headerMap;
    for (std::vector<std::string>::const_iterator it = std::next(lines.begin()); it != lines.end(); it++){
        size_t pos = it->find(":");
        std::string headerName;
        std::string content;
        if (pos != std::string::npos) {
            headerName = (*it).substr(0, pos);
            content = (*it).substr(pos + 1, (*it).length());
            headerMap[headerName] = content;
        } else {
            valid = false;
        }
    }
    if (headerMap.find("Content-Type") != headerMap.end())
        valid = false;
    
    if (valid == false)
        setResponseByErrorCode(500); // ERROR CODE DEFINE??
    else
        _m_header = headerMap;
}

void    Response::setResponseByErrorCode(int errorCode) {
    std::cout << "CGI HEADER RETURN IS WRONG!!!!!!" << std::endl;
}
