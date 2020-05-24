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
		std::cerr << "Usage: " << argv[0] << " <rom_file>\n";

		return EXIT_FAILURE;
	}

	naive_gbe::emulator emu;
	std::error_code ec;
	std::filesystem::path rom_file = argv[1];

	if (!emu.load(rom_file, ec))
	{
		std::string error_detail;
		if (ec)
			error_detail = ". Error: " + ec.message();

		std::cerr
			<< "Could not load rom file: " << rom_file.filename()
			<< error_detail << ".\n";

		return EXIT_FAILURE;
	}

	auto& cpu = emu.get_cpu();

	while (true)
		cpu.step();

	return EXIT_SUCCESS;
}
