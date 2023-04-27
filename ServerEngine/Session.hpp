/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 19:34:16 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/04/27 14:12:22 by jaehyuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>

#define LEN_SESSION_ID 10
#define MAX_SESSION_NUM 1
//#define SESSION_LIFE_TIME 60 * 5 //5분
#define SESSION_LIFE_TIME 10 //10초
//#define SESSION_LIFE_TIME 0 //0초

class Session {
	private:
		std::map<std::string, time_t> _m_sessions;
		
		std::string _M_generateSessionId();
		void	_M_clearExpieredSession();
	public:
		Session();
		~Session();
		
		std::string createSession();
		bool verifySession(std::string sessionId);
		void	showSessions();
};
