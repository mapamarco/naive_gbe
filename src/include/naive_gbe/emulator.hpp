//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <algorithm>

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
			cpu_(mmu_)
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

		std::string disassembly(std::uint8_t opcode, lr35902::operation const& op, bool extended)
		{
			if (opcode == 0xcb)
				return "";

			std::uint8_t params[4] = { 0, };
			std::uint16_t addr = cpu_.get_register(lr35902::r16::PC);

			std::copy(&mmu_[addr], &mmu_[addr + op.size_ - 1], &params[0]);

			return disasm_.disassembly(opcode, extended, op.size_, params);
		}

	private:

		mmu				mmu_;
		lr35902			cpu_;
		disassembler	disasm_;
	};

}
