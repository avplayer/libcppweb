
#pragma once

#include <string>
#include "proxy_chain.hpp"
#include "tcp_connect.hpp"
#include "ssl_connect.hpp"

namespace proxy{

	template<typename Socket>
	connect_chain direct_tcp_connect(Socket& s, std::string host, std::string port)
	{
		return { tcp_connect<Socket>(s, host, port) };
	}

	template<typename Socket>
	connect_chain direct_tcp_connect(Socket& s, std::string host, int port)
	{
		return { tcp_connect<Socket>(s, host, std::to_string(port)) };
	}

	template<typename Socket>
	connect_chain direct_ssl_connect(Socket& s, std::string host, int port)
	{
		return
		{
			tcp_connect<typename Socket::next_layer_type>(s.next_layer(), host, std::to_string(port)),
			ssl_connect<Socket>(s, host),
		};
	}

	template<typename Socket>
	connect_chain direct_ssl_connect(Socket& s, std::string host, std::string port)
	{
		return
		{
			tcp_connect<typename Socket::next_layer_type>(s.next_layer(), host, port),
			ssl_connect<Socket>(s, host),
		};
	}

}
