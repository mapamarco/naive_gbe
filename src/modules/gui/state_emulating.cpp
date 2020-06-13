//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_emulating.hpp"
using namespace naive_gbe;

state_emulating::state_emulating(naive_2dge::engine& engine, emulator_data& data, naive_gbe::emulator& emulator)
	: state_base(engine, data, emulator, state::EMULATING)
{
	using namespace std::placeholders;
	add_event_handler(SDL_KEYDOWN, std::bind(&state_emulating::on_key_down, this, _1));
	add_event_handler(SDL_KEYUP, std::bind(&state_emulating::on_key_up, this, _1));

	pallete_ = create_pallete();
}

void state_emulating::on_create()
{
	state_base::on_create();

	auto& ppu = emulator_.get_ppu();
	vram_ = engine_.create_texture("vram", ppu.get_screen_width(), ppu.get_screen_height());
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
		using cpu_state = naive_gbe::lr35902::state;
		auto& cpu = emulator_.get_cpu();

		while (steps_to_run_ && cpu.get_state() == cpu_state::READY)
		{
			cpu.step();
			--steps_to_run_;
		}

		update_vram();
	}

	auto [win_w, win_h] = engine_.get_window_size();
	auto [gbe_w, gbe_h] = emulator_.get_ppu().get_window_size();
	auto factor = std::min(win_w / gbe_w, win_h / gbe_h);
	auto width  = gbe_w * factor;
	auto height = gbe_h * factor;

	vram_->set_size(width, height);

	engine_.draw(vram_, (win_w - width) / 2, (win_h - height) / 2, data_.flags_ & flags::STRETCH);

	state_base::on_update();
}

void state_emulating::toggle_pause()
{
	paused_ = !paused_;
	SDL_ShowCursor(paused_ ? SDL_ENABLE : SDL_DISABLE);
}

std::size_t state_emulating::on_key_down(SDL_Event const& event)
{
	using joypad_input = naive_gbe::emulator::joypad_input;

	switch (event.key.keysym.sym)
	{
	case SDLK_1:
		emulator_.set_joypad(joypad_input::START, true);
		break;
	case SDLK_2:
		emulator_.set_joypad(joypad_input::SELECT, true);
		break;
	case SDLK_a:
		emulator_.set_joypad(joypad_input::A, true);
		break;
	case SDLK_s:
		emulator_.set_joypad(joypad_input::B, true);
		break;
	case SDLK_UP:
		emulator_.set_joypad(joypad_input::UP, true);
		break;
	case SDLK_DOWN:
		emulator_.set_joypad(joypad_input::DOWN, true);
		break;
	case SDLK_LEFT:
		emulator_.set_joypad(joypad_input::LEFT, true);
		break;
	case SDLK_RIGHT:
		emulator_.set_joypad(joypad_input::RIGHT, true);
		break;
	case SDLK_F10:
		++steps_to_run_;
		break;
	case SDLK_F11:
		steps_to_run_ += 24902 - 10;
		break;
	case SDLK_p:
		toggle_pause();
		break;
	case SDLK_r:
		emulator_.get_cpu().reset();
		steps_to_run_ = 0;
		break;
	}

	return next_state_;
}

std::size_t state_emulating::on_key_up(SDL_Event const& event)
{
	using joypad_input = naive_gbe::emulator::joypad_input;

	switch (event.key.keysym.sym)
	{
	case SDLK_1:
		emulator_.set_joypad(joypad_input::START, false);
		break;
	case SDLK_2:
		emulator_.set_joypad(joypad_input::SELECT, false);
		break;
	case SDLK_a:
		emulator_.set_joypad(joypad_input::A, false);
		break;
	case SDLK_s:
		emulator_.set_joypad(joypad_input::B, false);
		break;
	case SDLK_UP:
		emulator_.set_joypad(joypad_input::UP, false);
		break;
	case SDLK_DOWN:
		emulator_.set_joypad(joypad_input::DOWN, false);
		break;
	case SDLK_LEFT:
		emulator_.set_joypad(joypad_input::LEFT, false);
		break;
	case SDLK_RIGHT:
		emulator_.set_joypad(joypad_input::RIGHT, false);
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
