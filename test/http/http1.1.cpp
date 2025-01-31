#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <protocol_handler/http/1.0.h>

TEST_CASE("HTTP 1.0", "[parse request with pathname]")
{
	protocol::HTTP1 agent;
	auto parser = agent.get_praser();
	parser->process_chunk("GET /index.html HTTP/1.0\r\n\r\n\r\n");

	auto content = parser->get_request()->body.read_next_chunk(1000);

	CHECK(content == "");
	CHECK(parser->get_request()->headers.empty());
	CHECK(parser->get_request()->query_params.empty());
	CHECK(parser->get_request()->pathname == "GET /index.html");
}

TEST_CASE("HTTP 1.0", "[parse request with pathname and query string]")
{
	protocol::HTTP1 agent;
	auto parser = agent.get_praser();
	parser->process_chunk("GET /index.html?a=b&c=d HTTP/1.0\r\n\r\n\r\n");

	auto content = parser->get_request()->body.read_next_chunk(1000);

	CHECK(content == "");
	CHECK(parser->get_request()->headers.empty());
	CHECK(std::any_cast<std::string>(parser->get_request()->query_params.at("a")) == "b");
	CHECK(std::any_cast<std::string>(parser->get_request()->query_params.at("c")) == "d");
	CHECK(parser->get_request()->pathname == "GET /index.html");
}

TEST_CASE("HTTP 1.0", "[parse request with pathname and headers]")
{
	protocol::HTTP1 agent;
	auto parser = agent.get_praser();
	parser->process_chunk(
	    "GET /index.html HTTP/1.0\r\nUser-Agent: NCSA_Mosaic/2.0 (Windows 3.1)\r\nUser-Agent1: NCSA_Mosaic/2.0 "
	    "(Windows 3.1)\r\n\r\n");

	auto content = parser->get_request()->body.read_next_chunk(1000);

	CHECK(content == "");
	CHECK(std::any_cast<std::string>(parser->get_request()->headers.at("User-Agent")) ==
	      "NCSA_Mosaic/2.0 (Windows 3.1)");
	CHECK(std::any_cast<std::string>(parser->get_request()->headers.at("User-Agent1")) ==
	      "NCSA_Mosaic/2.0 (Windows 3.1)");
	CHECK(parser->get_request()->query_params.empty());
	CHECK(parser->get_request()->pathname == "GET /index.html");
}

TEST_CASE("HTTP 1.0", "[parse request with pathname and body]")
{
	protocol::HTTP1 agent;
	auto parser = agent.get_praser();
	parser->process_chunk("GET /index.html HTTP/1.0\r\n\r\n\r\nasdf");

	auto content = parser->get_request()->body.read_next_chunk(1000);

	CHECK(content == "asdf");
	CHECK(parser->get_request()->headers.empty());
	CHECK(parser->get_request()->query_params.empty());
	CHECK(parser->get_request()->pathname == "GET /index.html");
}

TEST_CASE("HTTP 1.0", "[parse request with pathname, query string and headers]")
{
	protocol::HTTP1 agent;
	auto parser = agent.get_praser();
	parser->process_chunk(
	    "GET /index.html?a=b&c=d HTTP/1.0\r\nUser-Agent: NCSA_Mosaic/2.0 (Windows 3.1)\r\nUser-Agent1: NCSA_Mosaic/2.0 "
	    "(Windows 3.1)\r\n\r\nasdf");

	auto content = parser->get_request()->body.read_next_chunk(1000);

	CHECK(content == "asdf");
	CHECK(std::any_cast<std::string>(parser->get_request()->headers.at("User-Agent")) ==
	      "NCSA_Mosaic/2.0 (Windows 3.1)");
	CHECK(std::any_cast<std::string>(parser->get_request()->headers.at("User-Agent1")) ==
	      "NCSA_Mosaic/2.0 (Windows 3.1)");
	CHECK(parser->get_request()->pathname == "GET /index.html");
	CHECK(std::any_cast<std::string>(parser->get_request()->query_params.at("a")) == "b");
	CHECK(std::any_cast<std::string>(parser->get_request()->query_params.at("c")) == "d");
}
