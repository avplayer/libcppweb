
#pragma once

#include <type_traits>
#include <vector>
#include <functional>
#include <memory>
#include <boost/asio/io_service.hpp>
#include <boost/tti/has_member_function.hpp>

BOOST_TTI_HAS_MEMBER_FUNCTION(get_io_context)

namespace proxy{

	class proxy_interface
	{
		typedef std::function<void(boost::system::error_code)> connect_handler_t;

		struct proxy_impl_t
		{
			virtual void async_connect(connect_handler_t) = 0;
		};

		template<typename ProxyImpl>
		struct proxy_impl_copyable_adapter : public proxy_impl_t
		{
			proxy_impl_copyable_adapter(const ProxyImpl& _impl)
				: impl(_impl)
			{}

			proxy_impl_copyable_adapter(ProxyImpl&& _impl)
				: impl(_impl)
			{}

			virtual void async_connect(connect_handler_t handler) override
			{
				impl.async_connect(handler);
			}

			ProxyImpl impl;
		};

		template<typename ProxyImpl>
		struct proxy_impl_noncopyable_adapter : public proxy_impl_t
		{
			proxy_impl_noncopyable_adapter(ProxyImpl* _impl)
				: impl(_impl)
			{}

			virtual void async_connect(connect_handler_t handler) override
			{
				impl->async_connect(handler);
			}

			std::shared_ptr<ProxyImpl> impl;
		};

	public:
		template<typename ProxyImpl,
			typename = std::enable_if_t<
				std::conjunction_v<
					std::is_copy_constructible<typename std::remove_reference<ProxyImpl>::type>,
					has_member_function_get_io_context<ProxyImpl, boost::asio::io_service&>
				>
			>
		>
		proxy_interface(ProxyImpl&& proxyimpl)
			: io(proxyimpl.get_io_service())
		{
			proxy_impl.reset(new proxy_impl_copyable_adapter<typename std::remove_reference<ProxyImpl>::type>(proxyimpl));
		}

		template<typename ProxyImpl, typename = std::enable_if_t<has_member_function_get_io_context<ProxyImpl, boost::asio::io_service&>::value>>
		proxy_interface(ProxyImpl* proxyimpl)
			: io(proxyimpl->get_io_service())
		{
			proxy_impl.reset(new proxy_impl_noncopyable_adapter<ProxyImpl>(proxyimpl));
		}

		template<typename Handler>
		void async_connect(Handler&& handler)
		{
			proxy_impl->async_connect(handler);
		}

	private:
		boost::asio::io_service& io;
		std::shared_ptr<proxy_impl_t> proxy_impl;
	};

	typedef std::vector<proxy_interface> connect_chain;
}
