#include "Response.hpp"

/* CONSTRUCTOR */
Response::Response(){
    _m_errorCode = RESPONSE_OK;
    _M_initStatusCodeMap();
    _M_initStatusCodeBodyMap();
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
    
    // setResponseByErrorCode(400);
    response.append(_m_statusLine + "\n");
    for (std::map<std::string, std::string>::const_iterator it = this->_m_header.begin(); it != this->_m_header.end(); it++){
        std::string str = it->first + ": " + it->second;
        response.append(str);
        if (std::next(it) != _m_header.end())
            response.append("\n");
    }
    addBasicHeader();
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
    time_t now = time(0);  // 현재 시간 가져오기
    char* dt = ctime(&now);  // 문자열로 변환하기

    _m_header["Server"] = "webserv/1.0";
    _m_header["Date"] = std::string(dt);
    _m_header["Content-Length"] = _m_response.length();
    //Content-Type은 리소스 불러올때 적어줘야함
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

void Response::_M_initStatusCodeBodyMap(void){
    _m_statusCodeMessageMap[400] = "The server cannot process the request due to a client error.";
    _m_statusCodeMessageMap[401] = "The requested resource requires authentication.";
    _m_statusCodeMessageMap[403] = "You don't have permission to access this resource.";
    _m_statusCodeMessageMap[404] = "The requested resource could not be found.";
    _m_statusCodeMessageMap[500] = "The server encountered an unexpected condition that prevented it from fulfilling the request.";
}

void    Response::setResponseByErrorCode(int errorCode) {
    _m_errorCode = errorCode;
    setStatusLine(errorCode);
    _m_header["Content-Type"] = "text/html";
    ErrorCodeBody(errorCode);
    addBasicHeader();
}

void    Response::ErrorCodeBody(int errorCode)
{
    std::string body;
    body += "\n";
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
    body += "\n\n";
    appendResponse(body);
}


// Error Response Example
// HTTP/1.1 404 Not Found
// Content-Type: text/html; charset=UTF-8
// Content-Length: 1245
// Date: Wed, 13 Apr 2023 12:34:56 GMT

// <!DOCTYPE html>
// <html>
// <head>
// 	<title>404 Not Found</title>
// </head>
// <body>
// 	<h1>404 Not Found</h1>
// 	<p>The requested resource could not be found.</p>
// </body>
// </html>