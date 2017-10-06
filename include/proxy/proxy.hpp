
#pragma once

#include "proxy_chain.hpp"

namespace cppweb{
namespace proxy{

	// async connect with chained proxies.
	void async_connect(connect_chain the_proxy_chain);
}}
