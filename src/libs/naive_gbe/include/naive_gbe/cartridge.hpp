//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <naive_gbe/types.hpp>
#include <naive_gbe/misc.hpp>

namespace naive_gbe
{
	class cartridge
	{
	public:

		cartridge() = default;

		cartridge(buffer&& data);

		cartridge(std::initializer_list<std::uint8_t> data);

		bool load(std::string const& file_name, std::error_code& ec);

		buffer& get_data();

	protected:

		buffer	data_;
	};
}
