//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <cstdint>
#include <vector>
#include <functional>

namespace naive_gbe
{
	class address
	{
	public:

		using callback_type = std::function<void(std::uint8_t&)>;

		enum class access_mode : std::uint8_t
		{
			READ_ONLY,
			READ_WRITE
		};

		address(std::uint8_t* data, access_mode mode = access_mode::READ_ONLY);

		void set(std::uint8_t* data, access_mode mode);

		void set(callback_type callback);

		operator std::uint8_t() const;

		void operator=(std::uint8_t value);

	private:

		std::uint8_t*	data_		= nullptr;
		callback_type	callback_	= nullptr;
		access_mode		mode_		= access_mode::READ_ONLY;
	};
}
