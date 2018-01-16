
#pragma once

#include <arpa/inet.h>
#include <boost/endian/arithmetic.hpp>
#include <type_traits>
#include <ostream>
#include <boost/regex.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>
#include "error.hpp"

namespace proxy{

	namespace {
		static inline bool host_name_is_ipv4_string(std::string s)
		{
			boost::smatch w;
			if (boost::regex_match(s, w, boost::regex("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}")))
				return true;
			return false;
		}
	}

	template<typename Stream>
	class socks5_connect
	{
		template<typename Handler>
		struct socks5_connect_op : boost::asio::coroutine
		{
			typedef void result_type;

			void operator()(boost::system::error_code ec, std::size_t bytes_transfered = 0)
			{
				if (ec)
					return handler(ec);

//				const char * buffer = NULL;
///				if (m_buffer)
	//				buffer = boost::asio::buffer_cast<const char*>(m_buffer->data());

				BOOST_ASIO_CORO_REENTER(this)
				{
					// 写入　0x05, 0x01, 0x00
					// TODO,  支持认证.
					(*static_buf)[0] = 0x05;
					(*static_buf)[1] = 0x01;
					(*static_buf)[2] = 0x00;
					BOOST_ASIO_CORO_YIELD boost::asio::async_write(parent.s, boost::asio::buffer(static_buf->data(), 3), *this);
					BOOST_ASIO_CORO_YIELD boost::asio::async_read(parent.s, boost::asio::buffer(*static_buf), boost::asio::transfer_exactly(2), *this);

					// 解析是不是　0x05, 0x00
					if (!((*static_buf)[0] == 5 && (*static_buf)[1] == 0))
					{
						return handler(error::make_error_code(error::proxy_not_athorized));
					}

					{
						(*static_buf)[0] = 0x05;
						(*static_buf)[1] = 0x01;
						(*static_buf)[2] = 0x00;

						if (host_name_is_ipv4_string(parent.host))
						{
							(*static_buf)[3] = 0x01;
							std::memcpy(&((*static_buf)[4]), boost::asio::ip::address_v4::from_string(parent.host).to_bytes().data(), 4);
							std::memcpy(&((*static_buf)[8]), &parent.port, 2);
							bytes_transfered = 10;
						}
						else
						{
							(*static_buf)[3] = 0x03;
							(*static_buf)[4] = parent.host.length();
							std::memcpy(&((*static_buf)[5]), parent.host.c_str(), parent.host.length());
							std::memcpy(&((*static_buf)[5+parent.host.length()]), &parent.port, 2);
							bytes_transfered = 7 + parent.host.length();
						}
					}

					// 执行　connect 命令.
					BOOST_ASIO_CORO_YIELD boost::asio::async_write(parent.s, boost::asio::buffer(static_buf->data(), bytes_transfered), *this);
					BOOST_ASIO_CORO_YIELD boost::asio::async_read(parent.s, boost::asio::buffer(*static_buf), boost::asio::transfer_exactly(5), *this);

					// 解析返回值.
					if (!((*static_buf)[0] == 5 && (*static_buf)[1] == 0 && (*static_buf)[2] == 0 ))
					{
						return handler(error::make_error_code(static_cast<error::err_t>((*static_buf)[1])));
					}

					// ATYPE==ipv4
					if ((*static_buf)[3] == 1)
					{
						(*static_buf)[0] = (*static_buf)[4];
						BOOST_ASIO_CORO_YIELD boost::asio::async_read(parent.s, boost::asio::buffer(static_buf->data() + 1, 5), boost::asio::transfer_exactly(5), *this);
						// [static_buf, 6] ipv4 + port
						parent.bind_addr = boost::asio::ip::address_v4(ntohl(*reinterpret_cast<uint32_t*>(static_buf->data()))).to_string();
						parent.bind_port = ntohs(*reinterpret_cast<uint16_t*>(&(*static_buf)[4]));
					}
					else if ((*static_buf)[3] == 4)
					{
						(*static_buf)[0] = (*static_buf)[4];
						// [static_buf, 18] ipv6 + port
						BOOST_ASIO_CORO_YIELD boost::asio::async_read(parent.s, boost::asio::buffer(static_buf->data() + 1, 17), boost::asio::transfer_exactly(17), *this);

					}
					else if ((*static_buf)[3] == 3)
					{
						bytes_transfered = (*static_buf)[4];
						// static_buf[4] contains address-length
						BOOST_ASIO_CORO_YIELD boost::asio::async_read(parent.s, boost::asio::buffer(*static_buf), boost::asio::transfer_exactly(bytes_transfered), *this);

						// now static_buf contains address and port.
						parent.bind_addr.assign(static_buf->data(), bytes_transfered - 2);
						{
							auto ptr = (char*)(static_buf->data() + bytes_transfered - 2);
							parent.bind_port = ntohs(*ptr);
						}
					}
					else
					{
						// 出错.
						return handler(error::make_error_code(error::unknow_proxy_error));
					}

					parent.s.get_io_service().post(boost::asio::detail::bind_handler(handler, ec));
				}
			}

			socks5_connect_op(socks5_connect& p, const Handler& handler)
				: parent(p)
				, static_buf(boost::make_local_shared<std::array<char, 96>>())
				, handler(handler)
			{
			}

			socks5_connect_op(socks5_connect& p, Handler&& handler)
				: parent(p)
				, static_buf(boost::make_local_shared<std::array<char, 96>>())
				, handler(handler)
			{
			}

			socks5_connect& parent;
			boost::local_shared_ptr<std::array<char, 96>> static_buf;
			Handler handler;
		};

	public:
		socks5_connect(Stream& s, std::string host, int port)
			: s(s)
			, host(host)
			, port(port)
		{
		}

		template<typename Handler>
		void async_connect(Handler&& handler)
		{
			socks5_connect_op<typename std::remove_reference<Handler>::type>(*this, handler)
				(boost::system::error_code());
		}

		boost::asio::io_service& get_io_service()
		{
			return s.get_io_service();
		}

		std::string get_binded() { return bind_addr; }
		int get_binded_port(){ return bind_port; }

	private:
		Stream& s;
		std::string host;
		boost::endian::big_int16_t port;
		u_int16_t bind_port;
		std::string bind_addr;
	};

}
