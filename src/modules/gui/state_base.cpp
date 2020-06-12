//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_base.hpp"
using namespace naive_gbe;
using namespace naive_2dge;

std::uint32_t state_base::flags_ = 0;

state_base::state_base(naive_2dge::engine& engine, naive_gbe::emulator& emulator, std::size_t next_state)
	: naive_2dge::state(engine)
	, emulator_(emulator)
	, next_state_(next_state)
	, prev_state_(next_state)
{
	using namespace std::placeholders;

	add_event_handler(SDL_QUIT, std::bind(&state_base::on_quit, this));
	add_event_handler(SDL_KEYDOWN, std::bind(&state_base::on_key_down, this, _1));
}

void state_base::on_create()
{
	debug_fnt_ = engine_.create_font("fps", "JetBrainsMono-Bold.ttf", 24);
	flags_ = flags::DEBUG;
}

void state_base::on_enter(std::size_t prev_state)
{
	prev_state_ = prev_state;
}

void state_base::on_update()
{
	if (flags_ & flags::DEBUG)
		on_update_debug();
}

std::size_t state_base::on_quit()
{
	engine_.exit(EXIT_SUCCESS);
	return next_state_;
}

void state_base::on_update_debug()
{
	auto [w, h] = engine_.get_window_size();

	auto margin_left = w / 10;
	auto margin_top = h / 10;
	auto line_height = 30;
	auto line_pos = 0;

	colour bg_colour{ 32, 32, 32, 200 };
	engine_.draw(rectangle{ 0, 0, w, h}, bg_colour);

	engine_.draw(fps_fmt(engine_.get_fps()), debug_fnt_, margin_left, margin_top + line_height * line_pos++);
	engine_.draw("NEXT_ST: " + state_fmt(next_state_), debug_fnt_, margin_left, margin_top + line_height * line_pos++);
	engine_.draw("PREV_ST: " + state_fmt(prev_state_), debug_fnt_, margin_left, margin_top + line_height * line_pos++);
	
	std::string stretch = flags_& flags::STRETCH ? "TRUE" : "FALSE";
	engine_.draw("STRETCH: " + stretch, debug_fnt_, margin_left, margin_top + line_height * line_pos++);
	engine_.draw(" ", debug_fnt_, margin_left, margin_top + line_height * line_pos++);

	auto& cpu = emulator_.get_cpu();

	std::ostringstream out;
	out << reg_fmt(lr35902::r16::AF) << " "
		<< reg_fmt(lr35902::r16::BC) << " "
		<< reg_fmt(lr35902::r16::DE) << " "
		<< reg_fmt(lr35902::r16::HL);

	engine_.draw(out.str(), debug_fnt_, margin_left, margin_top + line_height * line_pos++);

	out.str("");
	out << reg_fmt(lr35902::r16::SP) << " "
		<< reg_fmt(lr35902::r16::PC) << " "
		<< "z=" << cpu.get_flag(lr35902::flags::ZERO) << " "
		<< "n=" << cpu.get_flag(lr35902::flags::SUBTRACTION) << " "
		<< "h=" << cpu.get_flag(lr35902::flags::HALF_CARRY) << " "
		<< "c=" << cpu.get_flag(lr35902::flags::CARRY);

	engine_.draw(out.str(), debug_fnt_, margin_left, margin_top + line_height * line_pos++);

	engine_.draw(emulator_.disassembly(), debug_fnt_, margin_left, margin_top + line_height * line_pos++);
}

std::size_t state_base::on_key_down(SDL_Event const& event)
{
	switch (event.key.keysym.sym)
	{
	case SDLK_ESCAPE:
	case SDLK_q:
		next_state_ = on_quit();
		break;
	case SDLK_1:
		set_scale(scale_mode::NO_SCALING);
		break;
	case SDLK_2:
		set_scale(scale_mode::SCALED_2X);
		break;
	case SDLK_3:
		set_scale(scale_mode::SCALED_3X);
		break;
	case SDLK_4:
		set_scale(scale_mode::SCALED_4X);
		break;
	case SDLK_F1:
		next_state_ = state::HELP;
		break;
	case SDLK_F2:
		flags_ ^= flags::DEBUG;
		break;
	case SDLK_F3:
		flags_ ^= flags::STRETCH;
		break;
	case SDLK_RETURN:
		if (event.key.keysym.mod & KMOD_ALT)
			engine_.toggle_fullscreen();
		break;
	}

	return next_state_;
}

void state_base::set_scale(scale_mode mode)
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
}

std::string state_base::fps_fmt(float fps)
{
	std::ostringstream out;

	out << std::fixed << std::setprecision(2)
		<< "FPS: " << fps;

	return out.str();
}

std::string state_base::state_fmt(std::size_t state)
{
	std::string state_name = "UNKNOWN";

	switch (state)
	{
	case state::NO_ROM:
		state_name = "NO_ROM";
		break;
	case state::HELP:
		state_name = "HELP";
		break;
	case state::EMULATING:
		state_name = "EMULATING";
		break;
	}

	return state_name;
}

std::string state_base::reg_fmt(naive_gbe::lr35902::r16 reg)
{
	std::string reg_name;

	switch (reg)
	{
	case naive_gbe::lr35902::r16::AF:
		reg_name = "AF";
		break;
	case naive_gbe::lr35902::r16::BC:
		reg_name = "BC";
		break;
	case naive_gbe::lr35902::r16::DE:
		reg_name = "DE";
		break;
	case naive_gbe::lr35902::r16::HL:
		reg_name = "HL";
		break;
	case naive_gbe::lr35902::r16::SP:
		reg_name = "SP";
		break;
	case naive_gbe::lr35902::r16::PC:
		reg_name = "PC";
		break;
	}

	std::ostringstream out;
	auto const& cpu = emulator_.get_cpu();

	out << reg_name << "="
		<< std::setw(4) << std::setfill('0')
		<< std::hex << cpu.get_register(reg);

	return out.str();
}

void state_base::throw_error(std::string const& description, std::string const& detail) const
{
	throw std::runtime_error(description + ". Error: " + detail);
}
