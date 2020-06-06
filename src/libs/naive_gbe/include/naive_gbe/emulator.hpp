//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>

#include <naive_gbe/mmu.hpp>
#include <naive_gbe/cpu.hpp>
#include <naive_gbe/cartridge.hpp>
#include <naive_gbe/disassembler.hpp>

namespace naive_gbe
{
	class emulator
	{
	public:

		emulator() :
			cpu_{ mmu_ },
			disasm_{ mmu_ }
		{
		}

		void set_cartridge(cartridge&& cartridge)
		{
			mmu_.set_cartridge(std::move(cartridge));
			cpu_.reset();
		}

		void set_bootstrap(buffer&& bootstrap)
		{
			mmu_.set_bootstrap(std::move(bootstrap));
			cpu_.reset();
		}

		bool load_file(std::string const& file_name, std::error_code& ec)
		{
			cartridge cartridge;

			if (!cartridge.load(file_name, ec))
				return false;

			set_cartridge(std::move(cartridge));

			return true;
		}

		lr35902& get_cpu()
		{
			return cpu_;
		}

		mmu const& get_mmu() const
		{
			return mmu_;
		}

		std::size_t run()
		{
			using namespace std::chrono;

			std::size_t last_cycle = cpu_.get_cycle();
			std::size_t num_steps = 0;

			if (last_cycle)
			{
				auto now = high_resolution_clock::now();
				auto elapsed_us = duration_cast<microseconds>(now - last_run_).count();

				if (elapsed_us)
					last_cycle += (lr35902::frequencies::nominal * 1000) / elapsed_us;
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

			last_run_ = high_resolution_clock::now();


			return num_steps;
		}

		std::string disassembly()
		{
			std::uint16_t addr = cpu_.get_register(lr35902::r16::PC);

			return disasm_.decode(addr);
		}

	private:

		using time_point = std::chrono::high_resolution_clock::time_point;

		mmu				mmu_;
		lr35902			cpu_;
		disassembler	disasm_;
		time_point		last_run_;
	};

}
