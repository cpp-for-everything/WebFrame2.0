#include <boot_server/server.h>
#include <protocol_handler/protocol_handler.h>

int main() {
	protocol::ProtocolManager pm;
	boot::server server;
	server.bind_to(8080);
	server.set_handler(&pm);
	server.start();
}