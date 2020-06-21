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

		enum constants : std::uint32_t
		{
			NUM_SCAN_LINES			= 154,
			CYCLES_PER_SECOND		= 4194304,
			CYCLES_PER_HBLANK		= 456,
			CYCLES_PER_VBLANK		= CYCLES_PER_HBLANK * NUM_SCAN_LINES,
		};

		enum class lcd_control : std::uint16_t
		{
			LCD_DISP_ENABLE			= 1 << 7,
			WND_TITLE_MAP_DISP_SEL	= 1 << 6,
			WND_DISP_ENABLE			= 1 << 5,
			BG_WND_TITLE_DATA_SEL	= 1 << 4,
			BG_TITLE_MAP_DISP_SEL	= 1 << 3,
			OBJ_SPRITE_SIZE			= 1 << 2,
			OBJ_SPRITE_DISP_ENABLE	= 1 << 1,
			BG_WND_DIS_PRIORITY		= 1 << 0,
		};

		enum class lcd_status : std::uint16_t
		{
			COINCIDENCE_INTERRUPT	= 1 << 6,
			MODE_2_OAM_INTERRUPT	= 1 << 5,
			MODE_1_VBLANK_INTERRUPT = 1 << 4,
			MODE_0_HBLANK_INTERRUPT = 1 << 3,
			CONICIDENCE_FLAG		= 1 << 2,
			MODE_FLAG				= 1 << 1 | 1 << 0,
		};

		enum io_register : std::uint16_t
		{
			IO_REG_LCDC				= 0xff40,
			IO_REG_LCDS				= 0xff41,
			IO_REG_SCY				= 0xff42,
			IO_REG_SCX				= 0xff43,
			IO_REG_LY				= 0xff44,
			IO_REG_LYC				= 0xff45,
			IO_REG_DMA				= 0xff46,
			IO_REG_BGP				= 0xff47,
			IO_REG_OBP0				= 0xff48,
			IO_REG_OBP1				= 0xff49,
			IO_REG_WY				= 0xff4a,
			IO_REG_WX				= 0xff4b,
			IO_REG_BPI				= 0xff68,
			IO_REG_BPD				= 0xff69,
			IO_REG_SPI				= 0xff6a,
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

		void run(std::size_t cycle);

		std::size_t get_cycle() const;

	private:

		mmu&			mmu_;
		video_ram		vram_;
		std::size_t		cycle_	= 0;
	};
}