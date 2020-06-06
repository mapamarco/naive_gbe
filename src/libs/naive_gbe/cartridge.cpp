//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_gbe/cartridge.hpp>
using namespace naive_gbe;

cartridge::cartridge(buffer&& data)
	: data_(data)
{
}

cartridge::cartridge(std::initializer_list<std::uint8_t> data)
	: data_(data)
{
}

bool cartridge::load(std::string const& file_name, std::error_code& ec)
{
	buffer data = load_file(file_name, ec);

	if (ec)
		return false;

	data_ = std::move(data);

	return true;
}

buffer const& cartridge::get_data() const
{
	return data_;
}
