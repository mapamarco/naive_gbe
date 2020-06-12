//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <vector>
#include <cstdint>

#include <naive_gbe/mmu.hpp>

namespace naive_gbe
{
	class ppu
	{
	public:

		using win_size = std::pair<std::uint16_t, std::uint16_t>;

		struct rect
		{
			std::uint16_t x_pos		= 0;
			std::uint16_t y_pos		= 0;
			std::uint16_t width		= 0;
			std::uint16_t height	= 0;
		};

		using video_ram		= std::vector<std::uint8_t>;

		ppu(mmu& mmu);

		void write_to_video_ram();

		std::uint16_t get_screen_width() const;

		std::uint16_t get_screen_height() const;

		rect get_screen() const;

		rect get_window() const;

		win_size get_window_size() const;

		video_ram const& get_video_ram() const;

	private:

		enum registers : std::uint16_t
		{
			SCY			= 0xff42,
			SCX			= 0xff43,
			LY			= 0xff44,
			LYC			= 0xff45,
			DMA			= 0xff46,
			BGP			= 0xff47,
			OBP0		= 0xff48,
			OBP1		= 0xff49,
			WY			= 0xff4a,
			WX			= 0xff4b,
			BPI			= 0xff68,
			BPD			= 0xff69,
			SPI			= 0xff6a,
		};

		mmu&			mmu_;
		video_ram		vram_;
	};
}