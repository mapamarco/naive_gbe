//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <naive_2dge/types.hpp>
#include <naive_2dge/font.hpp>

#include <vector>
#include <string>
#include <cstdint>

struct emulator_data
{
	std::uint32_t				flags_				= 0;

	naive_2dge::colour			debug_bg_colour_	= {  32,  32,  32, 200 };

	naive_2dge::colour			debug_text_colour_	= { 255, 255, 255, 255 };

	naive_2dge::colour			help_text_colour_	= {   0, 128, 128, 255 };

	naive_2dge::font::ptr		debug_font_			= nullptr;

	naive_2dge::font::ptr		help_font_			= nullptr;

	std::vector<std::string>	debug_text_			= {};
};