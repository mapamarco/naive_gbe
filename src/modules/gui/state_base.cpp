//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "state_base.hpp"
using namespace naive_gbe;
using namespace naive_2dge;

state_base::state_base(naive_2dge::engine& engine, emulator_data& data, naive_gbe::emulator& emulator, std::size_t next_state)
	: naive_2dge::state(engine)
	, data_(data)
	, emulator_(emulator)
	, next_state_(next_state)
	, prev_state_(next_state)
{
	add_event_handler(SDL_QUIT, std::bind(&state_base::on_quit, this));
	add_event_handler(SDL_KEYDOWN, std::bind(&state_base::on_key_down, this, std::placeholders::_1));
}

void state_base::on_create()
{
	data_.flags_ = flags::DEBUG;
}

void state_base::on_enter(std::size_t prev_state)
{
	prev_state_ = prev_state;
}

void state_base::on_update()
{
	if (data_.flags_ & flags::DEBUG)
		on_update_debug();
}

std::size_t state_base::on_quit()
{
	engine_.exit(EXIT_SUCCESS);
	return next_state_;
}

void state_base::on_update_debug()
{
	if (!(data_.flags_ & flags::DEBUG))
		return;

	debug(fps_fmt(engine_.get_fps()));
	debug("NEXT_ST: " + state_fmt(next_state_));
	debug("PREV_ST: " + state_fmt(prev_state_));

	std::string stretch = data_.flags_ & flags::STRETCH ? "TRUE" : "FALSE";
	debug("STRETCH: " + stretch);
	debug(" ");

	auto& cpu = emulator_.get_cpu();

	std::ostringstream out;
	out << reg_fmt(lr35902::r16::AF) << " "
		<< reg_fmt(lr35902::r16::BC) << " "
		<< reg_fmt(lr35902::r16::DE) << " "
		<< reg_fmt(lr35902::r16::HL);

	debug(out.str());

	out.str("");
	out << reg_fmt(lr35902::r16::SP) << " "
		<< reg_fmt(lr35902::r16::PC) << " "
		<< "Z=" << cpu.get_flag(lr35902::flags::ZERO) << " "
		<< "N=" << cpu.get_flag(lr35902::flags::SUBTRACTION) << " "
		<< "H=" << cpu.get_flag(lr35902::flags::HALF_CARRY) << " "
		<< "C=" << cpu.get_flag(lr35902::flags::CARRY);

	debug(out.str());
	debug("CYCLE: " + std::to_string(cpu.get_cycle()));
	debug("STATE: " + cpu_state_fmt(cpu.get_state()));
	debug("JOYPAD: " + joypad_state_fmt());
	debug("INTERRUPTIONS: " + std::to_string(cpu.get_ime()));
	debug("NEXT_OP: " + emulator_.disassembly());

	draw_debug_overlay();
}

std::size_t state_base::on_key_down(SDL_Event const& event)
{
	switch (event.key.keysym.sym)
	{
	case SDLK_ESCAPE:
	case SDLK_q:
		next_state_ = on_quit();
		break;
	case SDLK_3:
		set_scale(scale_mode::NO_SCALING);
		break;
	case SDLK_4:
		set_scale(scale_mode::SCALED_2X);
		break;
	case SDLK_5:
		set_scale(scale_mode::SCALED_3X);
		break;
	case SDLK_6:
		set_scale(scale_mode::SCALED_4X);
		break;
	case SDLK_F1:
		next_state_ = state::HELP;
		break;
	case SDLK_F2:
		data_.flags_ ^= flags::DEBUG;
		break;
	case SDLK_F3:
		data_.flags_ ^= flags::STRETCH;
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
	std::string text = "UNKNOWN";

	switch (state)
	{
	case state::NO_ROM:
		text = "NO_ROM";
		break;
	case state::HELP:
		text = "HELP";
		break;
	case state::EMULATING:
		text = "EMULATING";
		break;
	}

	return text;
}

std::string state_base::joypad_state_fmt() const
{
	std::ostringstream out;

	out << emulator_.get_joypad();

	return out.str();
}

std::string state_base::cpu_state_fmt(naive_gbe::lr35902::state state)
{
	std::string text = "UNKNOWN";

	switch (state)
	{
	case naive_gbe::lr35902::state::READY:
		text = "READY";
		break;
	case naive_gbe::lr35902::state::STOPPED:
		text = "STOPPED";
		break;
	case naive_gbe::lr35902::state::SUSPENDED:
		text = "SUSPENDED";
		break;
	}

	return text;
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

float state_base::get_scale() const
{
	auto [win_w, win_h] = engine_.get_window_size();
	auto [gbe_w, gbe_h] = emulator_.get_ppu().get_window_size();

	return std::min(win_w / gbe_w, win_h / gbe_h) * 0.25f;
}

void state_base::debug(std::string message)
{
	if (data_.flags_ & flags::DEBUG)
		data_.debug_text_.emplace_back(message);
}

void state_base::draw_debug_overlay()
{
	auto [w, h] = engine_.get_window_size();
	float scale = get_scale();
	std::int16_t margin_left = w / 20;
	std::int16_t margin_top = h / 20;
	std::uint16_t line_height = 30;

	engine_.draw(rectangle{ 0, 0, w, h }, data_.debug_bg_colour_);

	for (auto const& text : data_.debug_text_)
	{
		engine_.draw(text, data_.debug_font_, margin_left, margin_top, data_.debug_text_colour_, scale);
		margin_top += line_height;
	}

	data_.debug_text_.clear();
}

void state_base::throw_error(std::string const& description, std::string const& detail) const
{
	throw std::runtime_error(description + ". Error: " + detail);
}
