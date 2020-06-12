//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/fps_counter.hpp>

namespace naive_2dge
{
	fps_counter::fps_counter(std::size_t interval_ms)
		: interval_ms_(interval_ms)
		, last_fps_(clock::now())
	{
	}

	fps_counter& fps_counter::operator++()
	{
		using namespace std::chrono;

		++frames_;

		auto now = clock::now();
		std::size_t elapsed = duration_cast<milliseconds>(now - last_fps_).count();

		if (elapsed >= interval_ms_)
		{
			fps_rate_ = frames_ * 1000.0f / elapsed;
			last_fps_ = now;
			frames_ = 0;
		}

		return *this;
	}

	float fps_counter::get_fps() const
	{
		return fps_rate_;
	}
}