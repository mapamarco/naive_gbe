//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <cstdint>

namespace naive_2dge
{
	struct colour
	{
		std::uint8_t	r = 0;
		std::uint8_t	g = 0;
		std::uint8_t	b = 0;
		std::uint8_t	a = 0;
	};

	struct rectangle
	{
		std::int16_t	x = 0;
		std::int16_t	y = 0;
		std::uint16_t	w = 0;
		std::uint16_t	h = 0;
	};
}
