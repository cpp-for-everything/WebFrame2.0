#include <interface.h>

namespace protocol
{
    namespace http
    {
        class HTTP_1_0 : public protocol::Abstract
        {
            SOCKET listener;
            virtual SOCKET start_server() final;
            virtual generator<SOCKET> get_client() final;
            virtual SOCKET end_server() final;
        };
    } // namespace http
    
} // namespace protocol
