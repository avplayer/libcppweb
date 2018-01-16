
#pragma once

#include <memory>
#include <boost/asio/io_service.hpp>

namespace cppweb
{
	// provide type-erased stream interface
	class generic_stream
	{
		struct generic_stream_impl
		{
			virtual boost::asio::io_service& get_io_service() const = 0;
		};

		template<typename RealAsioType>
		struct generic_stream_impl_adapter : public generic_stream_impl
		{
			virtual boost::asio::io_service& get_io_service() const override
			{
				return real_type.get_io_service();
			}

			RealAsioType& real_type;
		};

	public:
		template<typename Handler>
		void async_write_some(Handler);

		template<typename Handler>
		void async_read_some(Handler);

	public:
		template<typename RealAsioType>
		generic_stream(RealAsioType& realimpl)
		{
			m_impl.reset(new generic_stream_impl_adapter<typename std::remove_reference<RealAsioType>::type>(realimpl));
		}

		boost::asio::io_service& get_io_service()
		{
			return m_impl->get_io_service();
		}

	private:
		std::unique_ptr<generic_stream_impl> m_impl;
	};
}
