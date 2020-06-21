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

#include <naive_gbe/cartridge.hpp>
#include <naive_gbe/address.hpp>
#include <naive_gbe/types.hpp>

namespace naive_gbe
{
	class mmu
	{
	public:

		mmu();

		address& operator[](std::uint16_t addr);

		address const& operator[](std::uint16_t addr) const;

		void set_bootstrap(buffer&& bootstrap);

		void set_cartridge(cartridge&& cartridge);

		virtual void reset();

	protected:

		void disable_bootstrap(std::uint8_t value);

		void assign(std::uint16_t addr, std::size_t size, std::uint8_t* data, address::access_mode mode);

		buffer get_bootstrap() const;

		cartridge						cartridge_;

		buffer							bootstrap_;

		buffer							video_ram_;

		buffer							invalid_;

		std::vector<address>			memory_;
	};
}
