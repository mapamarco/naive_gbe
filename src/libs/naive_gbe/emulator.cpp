//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_gbe/emulator.hpp>

namespace naive_gbe
{
	emulator::emulator()
		: ppu_{ mmu_ }
		, cpu_{ mmu_, ppu_ }
		, disasm_{ mmu_ }
		, state_{ state::NO_CARTRIDGE }
	{
	}

	emulator::state emulator::get_state() const
	{
		return state_;
	}

	void emulator::reset()
	{
		cpu_.reset();
		mmu_.reset();
	}

	void emulator::set_cartridge(cartridge&& cartridge)
	{
		mmu_.set_cartridge(std::move(cartridge));
		cpu_.reset();
		state_ = state::READY;
	}

	void emulator::set_bootstrap(buffer&& bootstrap)
	{
		mmu_.set_bootstrap(std::move(bootstrap));
		cpu_.reset();
	}

	bool emulator::load_rom(std::string const& rom_path, std::error_code& ec)
	{
		cartridge cartridge;

		if (!cartridge.load(rom_path, ec))
			return false;

		set_cartridge(std::move(cartridge));

		return true;
	}

	lr35902& emulator::get_cpu()
	{
		return cpu_;
	}

	mmu const& emulator::get_mmu() const
	{
		return mmu_;
	}

	ppu const& emulator::get_ppu() const
	{
		return ppu_;
	}

	std::size_t emulator::run()
	{
		if (state_ == state::NO_CARTRIDGE)
			return 0;

		using namespace std::chrono;

		std::size_t last_cycle = cpu_.get_cycle();
		std::size_t num_steps = 0;

		if (last_cycle)
		{
			auto now = high_resolution_clock::now();
			auto elapsed_us = duration_cast<microseconds>(now - last_run_).count();

			if (elapsed_us)
				last_cycle += (lr35902::frequencies::NOMINAL * 1000) / elapsed_us;
		}
		else
		{
			++last_cycle;
		}

		while (cpu_.get_cycle() < last_cycle)
		{
			cpu_.step();
			++num_steps;
		}

		ppu_.write_to_video_ram();

		last_run_ = high_resolution_clock::now();

		return num_steps;
	}

	std::string emulator::disassembly()
	{
		std::uint16_t addr = cpu_.get_register(lr35902::r16::PC);

		return disasm_.decode(addr);
	}

	void emulator::set_joypad(joypad_input input, bool value)
	{
		joypad_.set(static_cast<std::size_t>(input), value);
	}

	emulator::joypad_state const& emulator::get_joypad() const
	{
		return joypad_;
	}
}
