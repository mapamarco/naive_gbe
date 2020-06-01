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
			cpu_(mmu_),
			disasm_(mmu_)
		{
		}

		bool load(std::filesystem::path const& file_name, std::error_code& ec)
		{
			cartridge cartridge;

			if (!cartridge.load(file_name, ec))
				return false;

			mmu_.set_cartridge(std::move(cartridge));
			cpu_.reset();

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

		mmu				mmu_;
		lr35902			cpu_;
		disassembler	disasm_;
	};

}
