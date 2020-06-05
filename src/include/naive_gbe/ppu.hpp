//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <naive_gbe/mmu.hpp>

namespace naive_gbe
{
	class ppu
	{
	public:

		ppu(mmu& mmu) :
			mmu_(mmu)
		{
		}

	private:

		mmu&			mmu_;
	};
}