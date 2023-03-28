#pragma once

#include "./ServerEngine/ServerEngine.hpp"
#include <iostream>

class KqueueProcess : public ServerEngine
{
    public :
        KqueueProcess(){}
        virtual ~KqueueProcess(){}

        /* Process Function */
        static void WAIT_CONNECT();
        static void READ_REQUEST();
        static void READ_URL();
        static void READ_CGI_RESULT();
        static void WRITE_RESPONSE();
        static void EXCUTE_CGI();
};