//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_emulating.hpp"

namespace naive_gbe
{
	state_emulating::state_emulating(naive_2dge::engine& engine, emulator& emulator)
		: base_state(engine, emulator, state::EMULATING)
	{
		keymap_ = get_default_keymap();

		using namespace std::placeholders;
		add_event_handler(SDL_KEYDOWN, std::bind(&state_emulating::on_key_down, this, _1));
	}

	void state_emulating::on_enter(std::size_t prev_state)
	{
		next_state_ = state_;

		if (!vram_)
		{
			auto window = emulator_.get_ppu().get_window();
			vram_ = engine_.create_texture("vram", window.width, window.height);
		}

		font_ = engine_.create_font("fps", "JetBrainsMono-Bold.ttf", 24);


		SDL_ShowCursor(SDL_DISABLE);
	}

	void state_emulating::on_update()
	{
		if (paused_)
		{
			engine_.draw_text("PAUSED", font_, 0, 0);
			return;
		}

		engine_.draw_text("EMULATING", font_, 0, 0);
		auto& cpu = emulator_.get_cpu();

		while (num_steps_)
		{
			cpu.step();
			--num_steps_;
		}

		update_vram();

		std::ostringstream out;
		out << "af=" << std::setw(4) << std::setfill('0')
			<< std::hex << cpu.get_register(lr35902::r16::AF) << ' '
			<< "bc=" << std::setw(4) << std::setfill('0')
			<< std::hex << cpu.get_register(lr35902::r16::BC) << ' '
			<< "de=" << std::setw(4) << std::setfill('0')
			<< std::hex << cpu.get_register(lr35902::r16::DE) << ' '
			<< "hl=" << std::setw(4) << std::setfill('0')
			<< std::hex << cpu.get_register(lr35902::r16::HL);

		engine_.draw_text(out.str(), font_, 0, 100);
		out.str("");

		out << "sp=" << std::setw(4) << std::setfill('0')
			<< std::hex << cpu.get_register(lr35902::r16::SP) << ' '
			<< "pc=" << std::setw(4) << std::setfill('0')
			<< std::hex << cpu.get_register(lr35902::r16::PC) << ' '
			<< "z=" << cpu.get_flag(lr35902::flags::ZERO) << ' '
			<< "n=" << cpu.get_flag(lr35902::flags::SUBTRACTION) << ' '
			<< "h=" << cpu.get_flag(lr35902::flags::HALF_CARRY) << ' '
			<< "c=" << cpu.get_flag(lr35902::flags::CARRY);

		engine_.draw_text(out.str(), font_, 0, 130);

		engine_.draw_text(emulator_.disassembly(), font_, 0, 160);

		engine_.draw_texture(vram_, 0, 0);
	}

	void state_emulating::on_exit()
	{
	}

	std::size_t state_emulating::on_key_down(SDL_Event const& event)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_1:
			set_scale(scale_mode::NO_SCALING);
			break;
		case SDLK_2:
			set_scale(scale_mode::SCALED_2x);
			break;
		case SDLK_3:
			set_scale(scale_mode::SCALED_3x);
			break;
		case SDLK_4:
			set_scale(scale_mode::SCALED_4x);
			break;
		case SDLK_F10:
			++num_steps_;
			break;
		case SDLK_F11:
			num_steps_ += 24902 - 10;
			break;
		case SDLK_RETURN:
			if (event.key.keysym.mod & KMOD_ALT)
				engine_.toggle_fullscreen();
			else
				paused_ = !paused_;
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

		switch (mode)
		{
		case scale_mode::NO_SCALING:
			engine_.set_window_size(window.width, window.height);
			break;
		case scale_mode::SCALED_2x:
			engine_.set_window_size(window.width * 2, window.height * 2);
			break;
		case scale_mode::SCALED_3x:
			engine_.set_window_size(window.width * 3, window.height * 3);
			break;
		case scale_mode::SCALED_4x:
			engine_.set_window_size(window.width * 4, window.height * 4);
			break;
		}
	}

	void state_emulating::update_vram()
	{
		if (!vram_)
			return;

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
}