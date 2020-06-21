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
		write_to_video_ram();
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

	void ppu::run(std::size_t cycle)
	{
		cycle_ = cycle % CYCLES_PER_SECOND;

		mmu_[IO_REG_LY] = static_cast<std::uint8_t>(cycle_ / CYCLES_PER_HBLANK) % NUM_SCAN_LINES;
	}

	std::size_t ppu::get_cycle() const
	{
		return cycle_;
	}

	ppu::rect ppu::get_window() const
	{
		return rect{ static_cast<std::uint16_t>(mmu_[IO_REG_WX] + 7), mmu_[IO_REG_WY], 160, 144 };
	}

	std::uint16_t ppu::get_screen_width() const
	{
		return 160;
	}

	std::uint16_t ppu::get_screen_height() const
	{
		return 144;
	}

	ppu::rect ppu::get_screen() const
	{
		return rect{ 0, 0, 256, 256 };
	}

	ppu::win_size ppu::get_window_size() const
	{
		return { 160, 144 };
	}
}
