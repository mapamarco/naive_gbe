//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <chrono>
#include <cstdlib>

namespace naive_2dge
{
	class fps_counter
	{
		using clock = std::chrono::high_resolution_clock;

		std::size_t			frames_			= 0;
		std::size_t			interval_ms_	= 0;
		float				fps_rate_		= 0.0f;
		clock::time_point	last_fps_		= {};

	public:

		fps_counter(std::size_t interval_ms = 100);

		fps_counter& operator++();

		float get_fps() const;
	};

}
