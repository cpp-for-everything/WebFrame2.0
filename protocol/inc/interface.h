#pragma once

#include <platform.h>
#include <generator.h>

namespace protocol
{
    class Abstract
    {
        virtual SOCKET start_server() = 0;
        virtual generator<SOCKET> get_client() = 0;
        virtual SOCKET end_server() = 0;
    };
} // namespace protocol
