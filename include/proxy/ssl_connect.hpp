
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

	template<typename SSLStream>
	class ssl_connect
	{
		template<typename Handler>
		struct ssl_connect_op : boost::asio::coroutine
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
					BOOST_ASIO_CORO_YIELD parent.s.async_handshake(boost::asio::ssl::stream_base::client, *this);

					parent.s.get_io_service().post(boost::asio::detail::bind_handler(handler, ec));
				}
			}

			ssl_connect_op(ssl_connect& p, const Handler& handler)
				: parent(p)
				, static_buf(boost::make_local_shared<std::array<char, 96>>())
				, handler(handler)
			{
			}

			ssl_connect_op(ssl_connect& p, Handler&& handler)
				: parent(p)
				, static_buf(boost::make_local_shared<std::array<char, 96>>())
				, handler(handler)
			{
			}

			ssl_connect& parent;
			boost::local_shared_ptr<std::array<char, 96>> static_buf;
			Handler handler;
		};

	public:
		ssl_connect(SSLStream& s, std::string sni_host)
			: s(s)
			, sni_host(sni_host)
		{
			s.set_verify_mode(boost::asio::ssl::verify_peer);
			s.set_verify_callback(boost::asio::ssl::rfc2818_verification(sni_host));
		}

		// TODO
		ssl_connect(SSLStream& s, std::string sni_host, std::string client_cert)
			: s(s)
			, sni_host(sni_host)
		{
		}

		template<typename Handler>
		void async_connect(Handler&& handler)
		{
			ssl_connect_op<typename std::remove_reference<Handler>::type>(*this, handler)
				(boost::system::error_code());
		}

		boost::asio::io_service& get_io_service()
		{
			return s.get_io_service();
		}

	private:
		SSLStream& s;
		std::string sni_host;
	};

}
