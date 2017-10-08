
#include <iostream>
#include <boost/asio.hpp>

#include <proxy.hpp>

int main(int argc, char **argv)
{
	boost::asio::io_service io;
	boost::asio::ip::tcp::socket s(io);

	cppweb::proxy::connect_chain proxys;
	proxys.push_back(cppweb::proxy::tcp_connect(s, "localhost", "1080"));
	proxys.push_back(cppweb::proxy::socks5_connect<boost::asio::ip::tcp::socket>(s, "www.google.com", 80));

	cppweb::proxy::async_connect(proxys, [](auto ec)
	{
		std::cout << "Hello, world! " << ec.message() << std::endl;
	});
	io.run();
	return 0;
}
