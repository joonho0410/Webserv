#include "HttpSession.hpp"

HttpSession::HttpSession()
{

}

HttpSession::HttpSession(const HttpSession& copy)
{
	this->sessionId = copy.getSessionId();
	this->sessionData = copy.sessionData;
}

HttpSession::~HttpSession()
{

}

HttpSession& HttpSession::operator=(const HttpSession& copy)
{
	if (this == &copy)
		return (*this);
	this->sessionId = copy.getSessionId();
	this->sessionData = copy.sessionData;
	return (*this);
}

HttpSession::HttpSession(const std::string& sessionId)
{
	this->sessionId = sessionId;
}

std::string HttpSession::getSessionId() const
{
	return this->sessionId;
}

void HttpSession::setSessionData(const std::string key, const std::string& value)
{
	this->sessionData[key] = value;
}

std::string HttpSession::getSessionData(const std::string& key)
{
	return (this->sessionData[key]);
}

std::string HttpSession::generateSessionId()
{
	char	randomChar;
	std::srand(std::time(0));


	std::string sessionId = "";
	for (int i = 0; i < 16; i++){
		randomChar = 'A' + (std::rand() % 26);
		sessionId += randomChar;
	}
	return sessionId;
}