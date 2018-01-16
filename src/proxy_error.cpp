
#include "proxy/error.hpp"

namespace proxy{
namespace error{

	const boost::system::error_category& get_proxy_error_category()
	{
		static proxy_error_category error_category_instance;
		return reinterpret_cast<const boost::system::error_category&>(error_category_instance);
	}

	std::string proxy_error_category::message(int value) const
	{
		switch(value)
		{
			case success:
				return "Success";
			case general_socks5_failure:
				return "general SOCKS server failure";
			case connection_not_allowd:
             	return "connection not allowed by ruleset";
			case network_unreachable:
				return "Network unreachable";
			case host_unreachable:
				return "Host unreachable";
			case connection_refused:
				return "Connection refused";
			case ttl_expired:
				return "TTL expired";
			case command_not_supported:
				return "Command not supported";
			case address_type_not_supported:
				return "Address type not supported";
			case proxy_not_athorized:
				return "authorizing to proxy failed";
		}
		return "unknow error";
	}

}}
