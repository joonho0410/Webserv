/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaehyuki <jaehyuki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/29 18:51:49 by jaehyuki          #+#    #+#             */
/*   Updated: 2023/03/29 19:02:37 by jaehyuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

CgiHandler::CgiHandler(){}

CgiHandler::CgiHandler(Request request)
    : _m_request(request)
{
    _M_initEnv(this->_m_request);
}

void    CgiHandler::_M_initEnv(Request request)
{
    
}