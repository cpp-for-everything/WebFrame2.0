#include <boot_server/server.h>
#include <protocol_handler/protocol_handler.h>

#include <iostream>
#include <string_view>
#include <string>
#include <chrono>
#include <thread>
int main()
{
	using namespace std::chrono_literals;
	std::string_view str;
	{
		std::string a = "Alex";
		str = a;
		a[0] = 'B';
	}
	char* c = new char[]{"ZZZZ"};
	std::cout << str << "\n";
	delete[] c;
	std::this_thread::sleep_for(10000ms);
	std::cout << str << "\n";

	boot::iserver::allowNetworkOperations();
	protocol::ProtocolManager pm;
	boot::server server(pm);
	server.bind_to(8081);
	server.start();
	boot::iserver::endNetworkOperations();
}