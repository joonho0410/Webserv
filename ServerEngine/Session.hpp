/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 19:34:16 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/04/26 12:44:51 by jaehyuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>

#define LEN_SESSION_ID 10
#define MAX_SESSION_NUM 100
#define SESSION_LIFE_TIME 60 * 5 //5분

class Session {
	private:
		std::map<std::string, time_t> _m_sessions;
		
		std::string _M_generateSessionId();
		void	_M_clearExpieredSession();
	public:
		Session();
		~Session();
		
		std::string createSession();
		std::string verifySession(std::string sessionId);
};
