//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <fstream>
#include <vector>
#include <cstdint>
#include <system_error>
#include <filesystem>

#include <naive_gbe/device.hpp>

namespace naive_gbe
{

	class cartridge
	{
	public:

		bool load(std::filesystem::path const& file_name, std::error_code& ec)
		{
			if (!std::filesystem::exists(file_name, ec))
				return false;

			std::ifstream stream{ file_name, std::ios::binary | std::ios::ate };
			if (!stream.is_open())
			{
				ec = std::make_error_code(std::errc::bad_file_descriptor);
				return false;
			}

			data_.resize(static_cast<std::size_t>(stream.tellg()));

			stream.seekg(std::ios::beg);

			if (!stream.read(reinterpret_cast<char*>(data_.data()), data_.size()))
			{
				ec = std::make_error_code(std::errc::io_error);
				return false;
			}

			stream.close();

			return true;
		}

	private:

		std::vector<std::uint8_t>	data_;
	};

}

