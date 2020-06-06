//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

#include <naive_gbe/emulator.hpp>
using namespace naive_gbe;

void log_info(std::ofstream& fs, std::string const& msg)
{
	if (fs.is_open())
		fs << msg << std::endl;
}

std::ofstream get_log_stream(std::string const& log_path)
{
	std::ofstream fs{ log_path };
	if (!fs.is_open())
		std::cerr << "Could not create the log file: " << log_path << ".\n";

	return fs;
}

std::string cpu_state(naive_gbe::lr35902 const& cpu)
{
	std::ostringstream out;

	out << "af=" << std::setw(4) << std::setfill('0')
		<< std::hex << cpu.get_register(lr35902::r16::AF) << ' '
		<< "bc=" << std::setw(4) << std::setfill('0')
		<< std::hex << cpu.get_register(lr35902::r16::BC) << ' '
		<< "de=" << std::setw(4) << std::setfill('0')
		<< std::hex << cpu.get_register(lr35902::r16::DE) << ' '
		<< "hl=" << std::setw(4) << std::setfill('0')
		<< std::hex << cpu.get_register(lr35902::r16::HL) << ' '
		<< "sp=" << std::setw(4) << std::setfill('0')
		<< std::hex << cpu.get_register(lr35902::r16::SP) << ' '
		<< "pc=" << std::setw(4) << std::setfill('0')
		<< std::hex << cpu.get_register(lr35902::r16::PC) << ' '
		<< "z=" << cpu.get_flag(lr35902::flags::zero) << ' '
		<< "n=" << cpu.get_flag(lr35902::flags::subtraction) << ' '
		<< "h=" << cpu.get_flag(lr35902::flags::half_carry) << ' '
		<< "c=" << cpu.get_flag(lr35902::flags::carry);

	return out.str();
}

int report_error(std::string const& message, std::error_code& ec)
{
	std::string detail;
	if (ec)
		detail = ". Error: " + ec.message();

	std::cerr << message << detail << ".\n";

	return EXIT_FAILURE;
}

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3)
	{
		std::string file_name = argv[0];
		auto pos = file_name.find_last_of("\\/");

		std::cerr
			<< "Usage: " << file_name.substr(pos + 1)
			<< " <rom_file> [log_file] [bootstrap_file]\n";

		return EXIT_FAILURE;
	}

	emulator emu;
	std::error_code ec;

	std::string rom_path = argv[1];
	if (!emu.load_file(rom_path, ec))
	{
		std::string error = "Could not open rom file: " + rom_path;

		return report_error(error, ec);
	}

	std::ofstream log;
	if (argc == 3)
	{
		std::string log_path = argv[2];

		log = get_log_stream(log_path);
	}
	else
	{
		std::string bootstrap_path = argv[3];
		buffer data = load_file(bootstrap_path, ec);

		if (ec)
		{
			std::string error = "Could not open bootstrap file: " + bootstrap_path;

			return report_error(error, ec);
		}

		emu.set_bootstrap(std::move(data));
	}

	auto& cpu = emu.get_cpu();

	//std::size_t num_steps = 0;
	//for (auto i = 0; i < 60; ++i)
	//{
	//	num_steps += emu.run();
	//	log_info(log, std::to_string(num_steps));
	//	std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
	//}
	//return 0;

	while (cpu.get_state() != lr35902::state::stopped)
	{
		log_info(log, emu.disassembly());
		cpu.step();
		log_info(log, cpu_state(cpu));
	}

	return EXIT_SUCCESS;
}
