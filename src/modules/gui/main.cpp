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

#include "emulator_gui.hpp"
using namespace naive_gbe;

std::pair<std::string, std::string> split_path(std::string const& path)
{
	auto pos = path.find_last_of("\\/");

	return { path.substr(0, pos), path.substr(pos + 1) };
}

int report_error(std::string const& message, std::error_code ec = {})
{
	std::string detail;
	if (ec)
		detail = ". Error: " + ec.message() + ".";

	std::cerr << message << detail << '\n';

	return EXIT_FAILURE;
}

int main(int argc, char** argv)
{
	auto [assets_path, exe_name] = split_path(argv[0]);

	if (argc > 2)
	{
		std::string error_msg = "Usage: " + exe_name + " [rom_file]";

		return report_error(error_msg);
	}

	try
	{
		emulator_gui emu{ assets_path };

		if (argc == 2)
		{
			std::error_code ec;
			std::string rom_path = argv[1];

			if (!emu.load_rom(rom_path, ec))
			{
				std::string erro_msg = "Could not load rom file: " + rom_path;

				return report_error(erro_msg, ec);
			}
		}

		emu.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << '\n';

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
