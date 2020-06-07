//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_gbe/ppu.hpp>

#include <random>

namespace naive_gbe
{

	ppu::ppu(mmu& mmu)
		: mmu_(mmu)
	{
		auto [width, height] = get_screen_size();
		std::size_t num_pixesl = width * height;

		vram_.assign(num_pixesl, 0);
	}

	void ppu::write_to_video_ram()
	{
		std::mt19937 urng{ std::random_device{}() };
		std::uniform_int_distribution<int> dist(0, 3);

		for (auto& pixel : vram_)
			pixel = dist(urng);
	}

	ppu::video_ram const& ppu::get_video_ram() const
	{
		return vram_;
	}

	ppu::screen_size ppu::get_screen_size() const
	{
		return { 160, 144 };
	}
}
