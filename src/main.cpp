//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <cstdlib>
#include <filesystem>

#include <naive_gbe/emulator.hpp>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::filesystem::path cmd_path = argv[0];

		std::cerr << "Usage: " << cmd_path.filename() << " <rom_file>\n";

		return EXIT_FAILURE;
	}

	naive_gbe::emulator emulator;
	std::error_code ec;
	std::filesystem::path rom_path = argv[1];

	if (!emulator.load(rom_path, ec))
	{
		std::string error_detail;
		if (ec)
			error_detail = ". Error: " + ec.message();

		std::cerr
			<< "Could not load rom file: " << rom_path
			<< error_detail << ".\n";

		return EXIT_FAILURE;
	}

	auto& cpu = emulator.get_cpu();

	while (true)
		cpu.step();

	return EXIT_SUCCESS;
}
