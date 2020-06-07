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
		auto screen = get_screen();

		vram_.assign(screen.width * screen.height, 0);
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

	ppu::rect ppu::get_window() const
	{
		return rect{ static_cast<std::uint16_t>(mmu_[registers::WX] + 7), mmu_[registers::WY], 160, 144 };
	}

	ppu::rect ppu::get_screen() const
	{
		return rect{ 0, 0, 256, 256 };
	}
}
