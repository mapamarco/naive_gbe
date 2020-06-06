//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_gbe/misc.hpp>

#include <fstream>

namespace naive_gbe
{
	buffer load_file(std::string const& file_name, std::error_code& ec)
	{
		std::ifstream stream{ file_name, std::ios::binary | std::ios::ate };

		if (!stream.is_open())
		{
			ec = std::make_error_code(std::errc::bad_file_descriptor);

			return {};
		}

		buffer data;
		data.resize(static_cast<std::size_t>(stream.tellg()));

		stream.seekg(std::ios::beg);

		if (!stream.read(reinterpret_cast<char*>(data.data()), data.size()))
		{
			ec = std::make_error_code(std::errc::io_error);

			return {};
		}

		return data;
	}
}
