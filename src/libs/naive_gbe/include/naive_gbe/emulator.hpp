//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include <naive_gbe/misc.hpp>
#include <naive_gbe/mmu.hpp>
#include <naive_gbe/cpu.hpp>
#include <naive_gbe/cartridge.hpp>
#include <naive_gbe/disassembler.hpp>

namespace naive_gbe
{
	class emulator
	{
	public:

		enum class state : std::uint8_t
		{
			NO_CARTRIDGE,
			READY,
			RUNNING,
			PAUSED
		};

		enum joypad_input : std::uint8_t
		{
			SELECT,
			START,
			A,
			B,
			UP,
			DOWN,
			LEFT,
			RIGHT
		};

		emulator();

		state get_state() const;

		void set_cartridge(cartridge&& cartridge);

		void set_bootstrap(buffer&& bootstrap);

		bool load_rom(std::string const& rom_path, std::error_code& ec);

		lr35902& get_cpu();

		mmu const& get_mmu() const;

		std::size_t run();

		std::string disassembly();

	private:

		using time_point = std::chrono::high_resolution_clock::time_point;

		state			state_		= state::NO_CARTRIDGE;
		time_point		last_run_;
		mmu				mmu_;
		lr35902			cpu_;
		disassembler	disasm_;
	};

}
