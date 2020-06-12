//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_emulating.hpp"
using namespace naive_gbe;

state_emulating::state_emulating(naive_2dge::engine& engine, naive_gbe::emulator& emulator)
	: state_base(engine, emulator, state::EMULATING)
{
	keymap_ = get_default_keymap();

	using namespace std::placeholders;
	add_event_handler(SDL_KEYDOWN, std::bind(&state_emulating::on_key_down, this, _1));
}

void state_emulating::on_create()
{
	state_base::on_create();

	font_ = engine_.create_font("debug", "JetBrainsMono-Bold.ttf", 24);

	pallete_ = create_pallete();

	keymap_ = get_default_keymap();

	auto& ppu = emulator_.get_ppu();
	vram_ = engine_.create_texture("vram", ppu.get_screen_width(), ppu.get_screen_height());

	set_scale(scale_mode::SCALED_4X);
}

void state_emulating::on_enter(std::size_t prev_state)
{
	next_state_ = state::EMULATING;
	engine_.show_cursor(false);
	state_base::on_enter(prev_state);
}

void state_emulating::on_update()
{
	if (!paused_)
	{
		num_steps_ += emulator_.run();

		update_vram();
	}

	auto [win_w, win_h] = engine_.get_window_size();
	auto [gbe_w, gbe_h] = emulator_.get_ppu().get_window_size();
	auto factor = std::min(win_w / gbe_w, win_h / gbe_h);
	auto width  = gbe_w * factor;
	auto height = gbe_h * factor;

	vram_->set_size(width, height);

	engine_.draw(vram_, (win_w - width) / 2, (win_h - height) / 2, flags_ & flags::STRETCH);

	state_base::on_update();
}

void state_emulating::toggle_pause()
{
	paused_ = !paused_;
	SDL_ShowCursor(paused_ ? SDL_ENABLE : SDL_DISABLE);
}

std::size_t state_emulating::on_key_down(SDL_Event const& event)
{
	switch (event.key.keysym.sym)
	{
	case SDLK_F10:
		++num_steps_;
		break;
	case SDLK_F11:
		num_steps_ += 24902 - 10;
		break;
	case SDLK_RETURN:
		toggle_pause();
		break;
	case SDLK_r:
		emulator_.get_cpu().reset();
		break;
	}

	return next_state_;
}

state_emulating::pallete state_emulating::create_pallete() const
{
	SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
	if (!format)
		throw_error("Could not allocate format", SDL_GetError());

	pallete pallete =
	{
		{ 0, SDL_MapRGBA(format,   0,   0,   0, 255) },
		{ 1, SDL_MapRGBA(format,  63,  63,  63, 255) },
		{ 2, SDL_MapRGBA(format, 127, 127, 127, 255) },
		{ 3, SDL_MapRGBA(format, 255, 255, 255, 255) },
	};

	SDL_FreeFormat(format);

	return pallete;
}

state_emulating::keymap state_emulating::get_default_keymap() const
{
	using input = emulator::joypad_input;

	keymap keys;

	keys[SDLK_1] = input::SELECT;
	keys[SDLK_2] = input::START;
	keys[SDLK_a] = input::A;
	keys[SDLK_s] = input::B;
	keys[SDLK_UP] = input::UP;
	keys[SDLK_DOWN] = input::DOWN;
	keys[SDLK_LEFT] = input::LEFT;
	keys[SDLK_RIGHT] = input::RIGHT;

	return keys;
}

void state_emulating::set_scale(scale_mode mode)
{
	if (engine_.is_fullscreen())
		return;

	auto window = emulator_.get_ppu().get_window();
	auto scale = 1;

	switch (mode)
	{
	case scale_mode::NO_SCALING:
		break;
	case scale_mode::SCALED_2X:
		scale = 2;
		break;
	case scale_mode::SCALED_3X:
		scale = 3;
		break;
	case scale_mode::SCALED_4X:
		scale = 4;
		break;
	}

	auto width = window.width * scale;
	auto height = window.height * scale;

	engine_.set_window_size(width, height);
	vram_->set_size(width, height);
}

void state_emulating::update_vram()
{
	assert(vram_ != nullptr);

	std::uint32_t* pixels = nullptr;
	int pitch = 0;

	SDL_Texture* texture = reinterpret_cast<SDL_Texture*>(vram_->get_resource());
	SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

	auto ppu = emulator_.get_ppu();
	auto vram = ppu.get_video_ram();

	auto screen = ppu.get_screen();
	auto window = ppu.get_window();

	std::size_t total = 0;
	for (auto row = 0; row < window.height; ++row)
	{
		std::size_t offset = window.x_pos + (row + window.y_pos) * screen.width;

		for (auto col = 0; col < window.width; ++col)
		{
			*pixels++ = pallete_[vram[offset + col]];
			total++;
		}
	}

	SDL_UnlockTexture(texture);
}
