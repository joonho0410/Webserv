#pragma once

#include <iostream>
#include <string>
#include <map>

class HttpSession {
	public:
		HttpSession(const std::string& sessionId);
		std::string getSessionId() const;
		void setSessionData(const std::string key, const std::string& value);
		std::string getSessionData(const std::string& key);

		
	private:
		HttpSession();
		HttpSession(const HttpSession& copy);
		~HttpSession();
		std::string generateSessionId();
		HttpSession& operator=(const HttpSession& copy);
		std::string sessionId;
		std::map<std::string, std::string> sessionData;
};