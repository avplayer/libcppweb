
#include <iostream>
#include <boost/asio.hpp>
#include "proxy.hpp"

int main(int argc, char **argv)
{
	boost::asio::io_service io;
	boost::asio::ip::tcp::socket s(io);

	cppweb::proxy::connect_chain proxys;
	proxys.push_back(cppweb::proxy::tcp_connect(s, "www.google.com", "80"));

	cppweb::proxy::async_connect(proxys);

	std::cout << "Hello, world!" << std::endl;
	return 0;
}
