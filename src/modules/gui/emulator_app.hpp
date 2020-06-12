//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <cstdint>
#include <unordered_map>

#include <naive_2dge/detail/sdl.hpp>
#include <naive_2dge/fps_counter.hpp>
#include <naive_2dge/engine.hpp>
#include <naive_2dge/game.hpp>
#include <naive_gbe/emulator.hpp>

class emulator_app :
	public naive_2dge::game
{
public:

	emulator_app(std::string const& assets_dir);

	bool load_rom(std::string const& rom_path, std::error_code& ec);

private:

	void render_display();

	void render_texture(SDL_Texture* texture, SDL_Rect* src = nullptr, SDL_Rect* dst = nullptr);

	naive_gbe::emulator		emulator_;
	naive_2dge::fps_counter	fps_;
	bool					stretch_		= false;
};
