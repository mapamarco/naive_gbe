//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <chrono>
#include <thread>

#include <naive_gbe/emulator.hpp>
using namespace naive_gbe;

void log_info(std::ofstream& fs, std::string const& msg)
{
	if (fs.is_open())
		fs << msg << '\n';
}

std::ofstream get_log_stream(fs::path log_path)
{
	std::ofstream fs{ log_path };
	if (!fs.is_open())
		std::cerr << "Could not create the log file: " << log_path << ".\n";

	return fs;
}

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3)
	{
		fs::path cmd_path = argv[0];

		std::cerr << "Usage: " << cmd_path.filename() << " <rom_file> [log_file]\n";

		return EXIT_FAILURE;
	}

	emulator emu;
	std::error_code ec;
	fs::path rom_path = argv[1];

	if (!emu.load_file(rom_path, ec))
	{
		std::string detail;
		if (ec)
			detail = ". Error: " + ec.message();

		std::cerr
			<< "Could not load rom file: " << rom_path
			<< detail << ".\n";

		return EXIT_FAILURE;
	}

	std::ofstream log;
	if (argc == 3)
	{
		fs::path log_path = argv[2];
		log = get_log_stream(log_path);
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
		log_info(log, emu.cpu_state());
	}

	return EXIT_SUCCESS;
}
