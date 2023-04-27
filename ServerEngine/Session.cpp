/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 19:34:20 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/04/27 14:11:52 by jaehyuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Session.hpp"


Session::Session()
{
}

Session::~Session()
{
}

std::string Session::_M_generateSessionId()
{
	std::string alnum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string sessionId;

	srand(static_cast<unsigned int>(time(NULL)));
	for (int i = 0; i < LEN_SESSION_ID; i++)
		sessionId += alnum[rand() % alnum.length()];
	return (sessionId);
}

void	Session::_M_clearExpieredSession()
{
	std::map<std::string, time_t>::iterator to_erase;
	for (std::map<std::string, time_t>::iterator it = _m_sessions.begin();
		it != _m_sessions.end();)
	{
		if (it->second < time(NULL))
		{
			std::map<std::string, time_t>::iterator to_erase = it;
			it++;
			_m_sessions.erase(to_erase);
		}
		else
			it++;
	}
}

std::string Session::createSession()
{
	std::string sessionId = _M_generateSessionId();
	time_t	now = time(NULL);
	
	if (_m_sessions.size() >= MAX_SESSION_NUM)
	{
		_M_clearExpieredSession();
	}
	_m_sessions[sessionId] = now + SESSION_LIFE_TIME;
	
	return (sessionId);
}

bool Session::verifySession(std::string sessionId)
{
	if (sessionId.length() == 0)
		return false;

	if (_m_sessions.find(sessionId) == _m_sessions.end())
		return false;

	if (_m_sessions[sessionId] < time(NULL))
	{
		_m_sessions.erase(sessionId);
		return (false);
	}
	
	return (true);
}

void Session::showSessions()
{
	int i = 0;

	std::cout << "* ------ SESSIONS -------- *" << std::endl;
	for (std::map<std::string, time_t>::iterator it = _m_sessions.begin(); it != _m_sessions.end(); it++)
	{
		std::cout << "| ";
		std::cout << "Session (" << i << ") : " << it->first;
		std::cout << " |" << std::endl;
		i++;
	}
	std::cout << "* ------------------------ *" << std::endl;
}