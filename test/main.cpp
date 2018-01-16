
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "proxy.hpp"

proxy::connect_chain auto_build_connect_chain(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>& s, std::string host, int port)
{
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::next_layer_type ssl_tcp_layer;

	proxy::connect_chain proxys;

	if (getenv("socks5_proxy"))
	{
		// TODO,
		proxys.push_back(proxy::tcp_connect<ssl_tcp_layer>(s.next_layer(), "localhost", "1080"));
		proxys.push_back(proxy::socks5_connect<ssl_tcp_layer>(s.next_layer(), host, port));
	}
	else
	{
		proxys.push_back(proxy::tcp_connect<ssl_tcp_layer>(s.next_layer(), host, std::to_string(port)));
	}

	proxys.push_back(proxy::ssl_connect<decltype(s)>(s, host));
	return proxys;
}

int main(int argc, char **argv)
{
	boost::asio::io_service io;
	//boost::asio::ip::tcp::socket s(io);

	boost::asio::ssl::context sslctx(boost::asio::ssl::context::sslv23);
	sslctx.add_verify_path("/etc/ssl/certs");
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket(io, sslctx);

//	generic_stream = ssl_socket;

	proxy::async_connect(auto_build_connect_chain(ssl_socket, "www.google.com", 443), [](auto ec)
	{
		// generic_stream
		std::cout << "Hello, world! " << ec.message() << std::endl;
	});
	io.run();
	return 0;
}
