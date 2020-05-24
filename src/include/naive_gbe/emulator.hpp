//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

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

	private:

		mmu				mmu_;
		lr35902			cpu_;
		disassembler	disasm_;
	};

}
