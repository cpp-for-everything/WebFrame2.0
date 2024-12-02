#include <http/1.0.hpp>

namespace protocol
{
    namespace http
    {
        SOCKET HTTP_1_0::start_server() {}
        generator<SOCKET> HTTP_1_0::get_client() {
            while (true) {
                // Accept a request
                SOCKET client = ACCEPT(listener, NULL, NULL);
                if (client == INVALID_SOCKET) {
                    continue;
                }
                this->logger << "(thread " << std::this_thread::get_id()
                            << ") Client invalid: " << (client == INVALID_SOCKET) << "\n";
                this->logger << "(thread " << std::this_thread::get_id() << ") Client found: " << client << "\n";

                // Check if the socket is valid
                {
                    struct timeval selTimeout;
                    selTimeout.tv_sec = 1;
                    selTimeout.tv_usec = 0;
                    fd_set readSet;
                    FD_ZERO(&readSet);
                    FD_SET(client + 1, &readSet);
                    FD_SET(client, &readSet);

                    int status = SELECT(client + 1, &readSet, nullptr, nullptr, &selTimeout);
                    this->logger << "(thread " << std::this_thread::get_id() << ") SELECT status is " << status << "\n";
                    if (status < 0) {
                        this->errors << "(thread " << std::this_thread::get_id() << ") INVALID SOCKET: " << client
                                    << " was skipped (" << status << ")\n";
                        continue;
                    }

                    this->logger << "(thread " << std::this_thread::get_id() << ") Client " << client
                                << " vs invalid socket " << INVALID_SOCKET << "\n";
                    this->logger << "(thread " << std::this_thread::get_id() << ") Requestor " << client
                                << " is still valid\n";

                    co_yield client;
                }
            }
            end_server();
        }
        SOCKET HTTP_1_0::end_server() {}
    } // namespace http
    
} // namespace protocol
