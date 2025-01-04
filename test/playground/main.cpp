#include <boot_server/server.h>
#include <protocol_handler/protocol_handler.h>

int main()
{
	boot::iserver::allowNetworkOperations();
	protocol::ProtocolManager pm;
	boot::server server(pm);
	server.bind_to(8081);
	server.start();
	boot::iserver::endNetworkOperations();
}