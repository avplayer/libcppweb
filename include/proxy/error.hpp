
#pragma once

#include <boost/system/error_code.hpp>

namespace cppweb{
namespace proxy{
namespace error{

	class proxy_error_category;

	const boost::system::error_category& get_proxy_error_category();

	enum err_t
	{
		success = 0,

		general_socks5_failure = 1,
		connection_not_allowd = 2,
		network_unreachable = 3,
		host_unreachable = 4,
		connection_refused = 5,
		ttl_expired = 6,
		command_not_supported = 7,
		address_type_not_supported = 8,

		unknow_proxy_error = 0x100,
		proxy_not_athorized,
		not_a_socks5_proxy,
	};

	class proxy_error_category : public boost::system::error_category {
	public:
		virtual const char* name() const BOOST_NOEXCEPT override
		{
			return "avproxy.proxy";
		}
		virtual std::string message ( int value ) const override;
	};

	inline boost::system::error_code make_error_code(err_t e) noexcept
	{
		return boost::system::error_code(static_cast<int>(e), get_proxy_error_category());
	}

}}}
