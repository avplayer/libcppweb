
#pragma once

#include <boost/asio/coroutine.hpp>
#include "proxy_chain.hpp"

namespace cppweb{
namespace proxy{

	namespace impl_detail
	{
		template<typename Handler>
		struct async_connect_op : boost::asio::coroutine
		{
			int proxy_idx = 0;
			connect_chain the_proxy_chain;
			Handler handler;

			async_connect_op(const connect_chain& the_proxy_chain, const Handler& handler)
				: the_proxy_chain(the_proxy_chain)
				, handler(handler)
			{}

			async_connect_op(const connect_chain& the_proxy_chain, Handler&& handler)
				: the_proxy_chain(the_proxy_chain)
				, handler(handler)
			{}

			void operator()(boost::system::error_code ec = boost::system::error_code())
			{
				BOOST_ASIO_CORO_REENTER(this)
				{
					if (ec)
						handler(ec);

					for (; proxy_idx < the_proxy_chain.size(); proxy_idx++)
					{
						BOOST_ASIO_CORO_YIELD the_proxy_chain[proxy_idx].async_connect(*this);
					}

					handler(ec);
				}
			}
		};
	}

	// async connect with chained proxies.
	template<typename Handler>
	void async_connect(const connect_chain& the_proxy_chain, Handler&& handler)
	{
		impl_detail::async_connect_op<typename std::remove_reference<Handler>::type>(the_proxy_chain, handler)();
	}

	template<typename Handler>
	void async_connect(connect_chain&& the_proxy_chain, Handler&& handler)
	{
		impl_detail::async_connect_op<typename std::remove_reference<Handler>::type>(the_proxy_chain, handler)();
	}
}}
