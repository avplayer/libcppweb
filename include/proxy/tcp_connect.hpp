
#pragma once

#include <type_traits>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

namespace cppweb{
namespace proxy{

	class tcp_connect
	{
		template<typename Handler>
		struct tcp_connect_op : boost::asio::coroutine
		{
			void operator()(boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator iterator = boost::asio::ip::tcp::resolver::iterator())
			{
				if (ec)
					return handler(ec);

				BOOST_ASIO_CORO_REENTER(this)
				{
					BOOST_ASIO_CORO_YIELD r->async_resolve(
						boost::asio::ip::tcp::resolver::query(parent.host, parent.port), *this
					);

					BOOST_ASIO_CORO_YIELD boost::asio::async_connect(
						parent.s, iterator, *this
					);

					handler(ec);
				}
			}

			tcp_connect_op(tcp_connect& p, const Handler& handler)
				: parent(p)
				, r(boost::make_local_shared<boost::asio::ip::tcp::resolver>(p.get_io_service()))
			{
			}

			tcp_connect_op(tcp_connect& p, Handler&& handler)
				: parent(p)
				, r(boost::make_local_shared<boost::asio::ip::tcp::resolver>(p.get_io_service()))
			{
			}

			tcp_connect& parent;
			boost::local_shared_ptr<boost::asio::ip::tcp::resolver> r;
			Handler handler;
		};

	public:
		tcp_connect(boost::asio::ip::tcp::socket& s, std::string host, std::string port)
			: s(s)
			, host(host)
			, port(port)
		{
		}

		template<typename Handler>
		void async_connect(Handler&& handler)
		{
			tcp_connect_op<typename std::remove_reference<Handler>::type>(*this, handler)
				(boost::system::error_code());
		}

		boost::asio::io_service& get_io_service()
		{
			return s.get_io_service();
		}

	private:
		boost::asio::ip::tcp::socket& s;
		std::string host, port;
	};

}}
