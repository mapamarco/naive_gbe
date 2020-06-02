#include <gtest/gtest.h>

#include <chrono>
using namespace std::chrono;

#include <naive_gbe/benchmark.hpp>
#include <naive_gbe/emulator.hpp>
using namespace naive_gbe;

TEST(performance, bootstrap)
{
	emulator emu;

	emu.load_cartridge({
		0x10,				// stop
	});

	auto& cpu = emu.get_cpu();

	std::size_t num_samples = 100;
	benchmark<std::chrono::microseconds> b{ num_samples };

	std::size_t num_steps = 0;
	std::size_t num_cycles = 0;
	auto result = b.run("bootstrap", [&]
	{
		cpu.reset();

		while (cpu.get_state() != lr35902::state::stopped)
		{
			cpu.step();
			++num_steps;
		}

		num_cycles += cpu.get_cycle();
	});

	//std::size_t total = 0;
	//auto baseline = b.run("baseline", [&]
	//{
	//	for (auto i = 0; i < 1e5; ++i)
	//		++total;
	//});
	//
	//EXPECT_EQ(total, 1e5 * num_samples);
	//EXPECT_EQ(num_steps, 24902 * num_samples);
	//EXPECT_EQ(num_cycles, 231884 * num_samples);
	//EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0101);
	//EXPECT_LT(result.average, baseline.average * 5.0);
}
