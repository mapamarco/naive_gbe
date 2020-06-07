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

		using screen_size	= std::pair<std::uint8_t, std::uint8_t>;
		using video_ram		= std::vector<std::uint8_t>;

		ppu(mmu& mmu);

		void write_to_video_ram();

		screen_size get_screen_size() const;

		video_ram const& get_video_ram() const;

	private:

		mmu&			mmu_;
		video_ram		vram_;
	};
}