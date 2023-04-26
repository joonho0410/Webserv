/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 19:34:20 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/04/26 12:48:56 by jaehyuki         ###   ########.fr       */
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
	for (std::map<std::string, time_t>::iterator it = _m_sessions.begin();
		it != _m_sessions.end();
		it++)
	{
		if (it->second < time(NULL))
			_m_sessions.erase(it);
	}
}

std::string Session::createSession();
{
	std::string sessionId = _M_generateSessionId();
	time_t	now = time(NULL);
	
	if (_m_sessions.size() > MAX_SESSION_NUM)
	{
		_M_clearExpieredSession();
	}
	
	_m_sessions[sessionId] = now;
	
	return (sessionId);
}

bool verifySession(std::string sessionId)
{
	if (_m_sessions.find(sessionId) == _m_sessions.end())
		return false;

	if (_m_sessions[sessionId] < time(NULL))
	{
		_m_sessions.erase(sessionId);
		return (false);
	}
	
	return (true);
}