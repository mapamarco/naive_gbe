//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <cstdint>
#include <vector>

#include <naive_gbe/cartridge.hpp>
#include <naive_gbe/types.hpp>

namespace naive_gbe
{

	class address
	{
	public:

		enum class mode : std::uint8_t
		{
			read_only,
			read_write
		};

		address(buffer& data);

		void set_data(buffer& data, std::size_t offset);

		operator std::uint8_t();

		void operator=(std::uint8_t value);

	private:

		std::size_t		offset_		= 0;
		mode			mode_		= mode::read_only;
		buffer&			data_;
	};

	class mmu
	{
	public:

		mmu();

		std::uint8_t& operator[](std::uint16_t addr);

		void set_bootstrap(buffer&& bootstrap);

		void set_cartridge(cartridge&& cartridge);

	protected:

		buffer get_bootstrap() const;

		cartridge						cartridge_;
		buffer							bootstrap_;
		std::vector<std::uint8_t>		memory_;
	};
}
