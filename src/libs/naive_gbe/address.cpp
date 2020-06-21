//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_gbe/address.hpp>

#include <cassert>

namespace naive_gbe
{
	address::address(std::uint8_t* data, access_mode mode)
		: data_(data)
		, mode_(mode)
	{
	}

	void address::set(std::uint8_t* data, access_mode mode)
	{
		data_ = data;
		mode_ = mode;
	}

	void address::set(callback_type callback)
	{
		callback_ = callback;
	}

	address::operator std::uint8_t() const
	{
		if (!data_)
		{
			assert(data_ != nullptr);
			return 0;
		}

		return *data_;
	}

	void address::operator=(std::uint8_t value)
	{
		if (mode_ == access_mode::READ_ONLY || !data_)
		{
			assert(true);
			return;
		}

		*data_ = value;

		if (callback_)
			callback_(value);
	}
}

