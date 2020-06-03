//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <array>

#include <naive_gbe/cartridge.hpp>

namespace naive_gbe
{
	class mmu
	{
	public:

		mmu()
		{
			bootstrap_ = get_bootstrap();

			// http://gameboy.mongenel.com/dmg/asmmemmap.html
			// http://gameboy.mongenel.com/dmg/asmmemmap.html

			memory_.assign(0x10000, 0);
			for (std::size_t indx = 0; indx < bootstrap_.size(); ++indx)
				memory_[indx] = bootstrap_[indx];
		}

		std::uint8_t& operator[](std::uint16_t addr)
		{
			return memory_[addr];
		}

		void set_cartridge(cartridge&& cartridge)
		{
			cartridge_ = cartridge;
			auto data = cartridge_.get_data();

			//for (std::size_t indx = 0; indx < data.size(); ++indx)
			//	memory_[indx] = data[indx];
			//for (std::size_t indx = 0x100; indx < data.size(); ++indx)
			//	memory_[indx] = data[indx];
		}

	protected:

		std::array<std::uint8_t, 256> get_bootstrap() const
		{
			// https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM
			// http://www.neviksti.com/DMG/DMG_ROM.bin
			// https://realboyemulator.wordpress.com/2013/01/03/a-look-at-the-game-boy-bootstrap-let-the-fun-begin/
			return std::array<std::uint8_t, 256>
			{
				0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32,
				0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e,
				0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3,
				0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0,
				0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a,
				0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b,
				0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06,
				0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9,
				0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99,
				0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20,
				0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64,
				0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
				0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90,
				0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2,
				0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62,
				0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06,
				0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42,
				0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20,
				0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04,
				0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17,
				0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9,
				0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b,
				0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d,
				0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
				0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99,
				0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc,
				0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e,
				0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c,
				0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13,
				0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
				0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20,
				0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50
			};
		}

		cartridge						cartridge_;
		std::array<std::uint8_t, 256>	bootstrap_;
		std::vector<std::uint8_t>		memory_;
	};
}
