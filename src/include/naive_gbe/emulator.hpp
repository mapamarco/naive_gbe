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

		void load_cartridge(cartridge&& cartridge)
		{
			mmu_.set_cartridge(std::move(cartridge));
			cpu_.reset();
		}

		bool load_file(std::filesystem::path const& file_name, std::error_code& ec)
		{
			cartridge cartridge;

			if (!cartridge.load(file_name, ec))
				return false;

			load_cartridge(std::move(cartridge));

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
			std::size_t curr_cycle = cpu_.get_cycle();
			std::size_t last_cycle = curr_cycle + 1;
			std::size_t num_steps = 0;

			if (curr_cycle)
			{
				using namespace std::chrono;

				auto now = high_resolution_clock::now();
				auto elapsed_us = duration_cast<microseconds>(now - last_run_).count();

				if (elapsed_us)
					last_cycle = curr_cycle + (lr35902::frequencies::nominal * 1000) / elapsed_us;
			}

			while (curr_cycle < last_cycle)
			{
				cpu_.step();
				curr_cycle = cpu_.get_cycle();
				++num_steps;
			}

			last_run_ = std::chrono::high_resolution_clock::now();

			return num_steps;
		}

		std::string cpu_state()
		{
			std::ostringstream out;

			out << "af=" << std::setw(4) << std::setfill('0')
				<< std::hex << cpu_.get_register(lr35902::r16::AF) << ' '
				<< "bc=" << std::setw(4) << std::setfill('0')
				<< std::hex << cpu_.get_register(lr35902::r16::BC) << ' '
				<< "de=" << std::setw(4) << std::setfill('0')
				<< std::hex << cpu_.get_register(lr35902::r16::DE) << ' '
				<< "hl=" << std::setw(4) << std::setfill('0')
				<< std::hex << cpu_.get_register(lr35902::r16::HL) << ' '
				<< "sp=" << std::setw(4) << std::setfill('0')
				<< std::hex << cpu_.get_register(lr35902::r16::SP) << ' '
				<< "pc=" << std::setw(4) << std::setfill('0')
				<< std::hex << cpu_.get_register(lr35902::r16::PC) << ' '
				<< "z=" << cpu_.get_flag(lr35902::flags::zero) << ' '
				<< "n=" << cpu_.get_flag(lr35902::flags::subtraction) << ' '
				<< "h=" << cpu_.get_flag(lr35902::flags::half_carry) << ' '
				<< "c=" << cpu_.get_flag(lr35902::flags::carry);

			return out.str();
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
