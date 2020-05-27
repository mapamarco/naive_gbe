#include <gtest/gtest.h>

#include <naive_gbe/cpu.hpp>
#include <naive_gbe/mmu.hpp>
using namespace naive_gbe;

class cartridge_buf :
	public cartridge
{
public:
	cartridge_buf(buffer&& data)
	{
		data_ = data;
	}
};

const std::vector<lr35902::r8> r8_registers =
{
	lr35902::r8::B,
	lr35902::r8::C,
	lr35902::r8::D,
	lr35902::r8::E,
	lr35902::r8::H,
	lr35902::r8::L,
	lr35902::r8::A,
};

const std::vector<lr35902::r16> r16_registers =
{
	lr35902::r16::BC,
	lr35902::r16::DE,
	lr35902::r16::HL,
	lr35902::r16::SP,
};

void run_n(lr35902& cpu, std::size_t n)
{
	for (auto i = 0; i < n; ++i)
		cpu.step();
}

TEST(registers, reset)
{
	mmu mmu;
	lr35902 cpu{ mmu };

	cpu.reset();

	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r8::F), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r8::B), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r8::C), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r8::D), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r8::E), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r8::H), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r8::L), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0);
	EXPECT_EQ(cpu.get_flags(), cpu.get_register(lr35902::r8::F));
	EXPECT_EQ(cpu.get_cycle(), 0);
}

TEST(instructions, op_ldi_hl)
{
	// LDI (HL), A
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xf1,			// LD A, 0xf1
		0x22,				// LDI (HL), A
	}));

	cpu.reset();
	cpu.step();
	cpu.step();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0006);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0xc001);
	EXPECT_EQ(mmu[0xc000], 0xf1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 28);
}

TEST(instructions, op_ldd_hl)
{
	// LDD (HL), A
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x01, 0xc0,	// LD HL, 0xc001
		0x3e, 0x34,			// LD A, 0x34
		0x32,				// LDD (HL), A
	}));

	cpu.reset();
	cpu.step();
	cpu.step();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0006);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0xc000);
	EXPECT_EQ(mmu[0xc001], 0x34);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 28);
}

TEST(instructions, op_ld_r8)
{
	// LD r8, u8
	// 1 4
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x12,			// LD B, 0x12
		0x0e, 0x23,			// LD C, 0x23
		0x16, 0x34,			// LD D, 0x34
		0x1e, 0x45,			// LD E, 0x45
		0x26, 0x56,			// LD L, 0x56
		0x2e, 0x67,			// LD H, 0x67
		0x3e, 0x78,			// LD A, 0x78
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::B), 0x12);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0002);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 8);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::C), 0x23);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0004);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 16);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::D), 0x34);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0006);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 24);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::E), 0x45);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0008);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 32);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::H), 0x56);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x000a);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 40);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::L), 0x67);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x000c);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 48);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x78);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x000e);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 56);
}

TEST(instructions, op_nop)
{
	// NOP
	// 1 4
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x00,				// NOP
		0x00,				// NOP
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0001);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 4);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0002);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 8);
}

TEST(instructions, op_inc_r16)
{
	// INC r16
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x03,				// INC BC
		0x13,				// INC DE
		0x23,				// INC HL
		0x33,				// INC SP
		}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::BC), 0x0001);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0001);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 8);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::DE), 0x0001);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0002);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 16);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0x0001);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0003);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 24);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0x0001);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0004);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 32);
}

TEST(instructions, op_dec_r16)
{
	// DEC r16
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x0b,				// DEC BC
		0x1b,				// DEC DE
		0x2b,				// DEC HL
		0x3b,				// DEC SP
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::BC), 0xffff);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0001);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 8);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::DE), 0xffff);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0002);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 16);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0xffff);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0003);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 24);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xffff);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0004);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 32);
}

TEST(instructions, op_ld_r16)
{
	// LD r16, d16
	// 3 12
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x01, 0xcd, 0xab,	// LD BC, 0xabcd
		0x11, 0x34, 0x12,	// LD DE, 0x1234
		0x21, 0x21, 0x43,	// LD HL, 0x4321
		0x31, 0x0a, 0xf0,	// LD SP, 0xf00a
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::BC), 0xabcd);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0003);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 12);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::DE), 0x1234);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0006);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 24);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0x4321);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0009);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 36);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xf00a);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x000c);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 48);
}

TEST(instructions, op_bit_r8)
{
	// CB BIT r8
	// 2 8
	// Z 0 1 -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x9d,			// LD B, 0x9d
		0x0e, 0x9d,			// LD C, 0x9d
		0x16, 0x9d,			// LD D, 0x9d
		0x1e, 0x9d,			// LD E, 0x9d
		0x26, 0x9d,			// LD L, 0x9d
		0x2e, 0x9d,			// LD H, 0x9d
		0x3e, 0x9d,			// LD A, 0x9d
		0xcb, 0x40,			// CB BIT 0, B
		0xcb, 0x41,			// CB BIT 0, C
		0xcb, 0x42,			// CB BIT 0, D
		0xcb, 0x43,			// CB BIT 0, E
		0xcb, 0x44,			// CB BIT 0, H
		0xcb, 0x45,			// CB BIT 0, L
		0xcb, 0x47,			// CB BIT 0, A
		0xcb, 0x48,			// CB BIT 1, B
		0xcb, 0x49,			// CB BIT 1, C
		0xcb, 0x4a,			// CB BIT 1, D
		0xcb, 0x4b,			// CB BIT 1, E
		0xcb, 0x4c,			// CB BIT 1, H
		0xcb, 0x4d,			// CB BIT 1, L
		0xcb, 0x4f,			// CB BIT 1, A
		0xcb, 0x50,			// CB BIT 2, B
		0xcb, 0x51,			// CB BIT 2, C
		0xcb, 0x52,			// CB BIT 2, D
		0xcb, 0x53,			// CB BIT 2, E
		0xcb, 0x54,			// CB BIT 2, H
		0xcb, 0x55,			// CB BIT 2, L
		0xcb, 0x57,			// CB BIT 2, A
		0xcb, 0x58,			// CB BIT 3, B
		0xcb, 0x59,			// CB BIT 3, C
		0xcb, 0x5a,			// CB BIT 3, D
		0xcb, 0x5b,			// CB BIT 3, E
		0xcb, 0x5c,			// CB BIT 3, H
		0xcb, 0x5d,			// CB BIT 3, L
		0xcb, 0x5f,			// CB BIT 3, A
		0xcb, 0x60,			// CB BIT 4, B
		0xcb, 0x61,			// CB BIT 4, C
		0xcb, 0x62,			// CB BIT 4, D
		0xcb, 0x63,			// CB BIT 4, E
		0xcb, 0x64,			// CB BIT 4, H
		0xcb, 0x65,			// CB BIT 4, L
		0xcb, 0x67,			// CB BIT 4, A
		0xcb, 0x68,			// CB BIT 5, B
		0xcb, 0x69,			// CB BIT 5, C
		0xcb, 0x6a,			// CB BIT 5, D
		0xcb, 0x6b,			// CB BIT 5, E
		0xcb, 0x6c,			// CB BIT 5, H
		0xcb, 0x6d,			// CB BIT 5, L
		0xcb, 0x6f,			// CB BIT 5, A
		0xcb, 0x70,			// CB BIT 6, B
		0xcb, 0x71,			// CB BIT 6, C
		0xcb, 0x72,			// CB BIT 6, D
		0xcb, 0x73,			// CB BIT 6, E
		0xcb, 0x74,			// CB BIT 6, H
		0xcb, 0x75,			// CB BIT 6, L
		0xcb, 0x77,			// CB BIT 6, A
		0xcb, 0x78,			// CB BIT 7, B
		0xcb, 0x79,			// CB BIT 7, C
		0xcb, 0x7a,			// CB BIT 7, D
		0xcb, 0x7b,			// CB BIT 7, E
		0xcb, 0x7c,			// CB BIT 7, H
		0xcb, 0x7d,			// CB BIT 7, L
		0xcb, 0x7f,			// CB BIT 7, A
	}));

	cpu.reset();

	for (auto i = 0; i < 7; ++i)
		cpu.step();

	struct result
	{
		std::uint8_t	value;
		std::uint8_t	flags;
	};

	std::vector<result> results =
	{
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0x20 },
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0x20 },
		result{ 0x9d, 0x20 },
		result{ 0x9d, 0xa0 },
	};

	std::uint16_t addr = 0x0010;
	std::uint64_t cycle = 64;

	for (auto const& res : results)
	{
		for (auto const& reg : r8_registers)
		{
			cpu.step();
			EXPECT_EQ(cpu.get_register(reg), res.value);
			EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
			EXPECT_EQ(cpu.get_flags(), res.flags);
			EXPECT_EQ(cpu.get_cycle(), cycle);
			addr += 2;
			cycle += 8;
		}
	}
}

TEST(instructions, op_bit_hl)
{
	// CB BIT n, (HL)
	// 2 8
	// Z 0 1 -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x9d,			// LD A, 0x9d
		0x77,				// LD (HL), A
		0xcb, 0x46,			// CB BIT 0, (HL)
		0xcb, 0x4e,			// CB BIT 1, (HL)
		0xcb, 0x56,			// CB BIT 2, (HL)
		0xcb, 0x5e,			// CB BIT 3, (HL)
		0xcb, 0x66,			// CB BIT 4, (HL)
		0xcb, 0x6e,			// CB BIT 5, (HL)
		0xcb, 0x76,			// CB BIT 6, (HL)
		0xcb, 0x7e,			// CB BIT 7, (HL)
	}));

	cpu.reset();

	run_n(cpu, 3);
	std::uint16_t addr  = cpu.get_register(lr35902::r16::PC);
	std::uint64_t cycle = cpu.get_cycle();

	struct result
	{
		std::uint8_t	value;
		std::uint8_t	flags;
	};

	std::vector<result> results =
	{
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0x20 },
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0xa0 },
		result{ 0x9d, 0x20 },
		result{ 0x9d, 0x20 },
		result{ 0x9d, 0xa0 },
	};


	for (auto const& res : results)
	{
		addr += 2;
		cycle += 16;
		cpu.step();
		EXPECT_EQ(mmu[0xc000], res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_res_r8)
{
	// CB RES r8
	// 2 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x06, 0xff,			// LD B, 0xff
		0x0e, 0xff,			// LD C, 0xff
		0x16, 0xff,			// LD D, 0xff
		0x1e, 0xff,			// LD E, 0xff
		0x26, 0xff,			// LD L, 0xff
		0x2e, 0xff,			// LD H, 0xff
		0x3e, 0xff,			// LD A, 0xff
		0xcb, 0x80,			// CB RES 0, B
		0xcb, 0x81,			// CB RES 0, C
		0xcb, 0x82,			// CB RES 0, D
		0xcb, 0x83,			// CB RES 0, E
		0xcb, 0x84,			// CB RES 0, H
		0xcb, 0x85,			// CB RES 0, L
		0xcb, 0x87,			// CB RES 0, A
		0xcb, 0x88,			// CB RES 1, B
		0xcb, 0x89,			// CB RES 1, C
		0xcb, 0x8a,			// CB RES 1, D
		0xcb, 0x8b,			// CB RES 1, E
		0xcb, 0x8c,			// CB RES 1, H
		0xcb, 0x8d,			// CB RES 1, L
		0xcb, 0x8f,			// CB RES 1, A
		0xcb, 0x90,			// CB RES 2, B
		0xcb, 0x91,			// CB RES 2, C
		0xcb, 0x92,			// CB RES 2, D
		0xcb, 0x93,			// CB RES 2, E
		0xcb, 0x94,			// CB RES 2, H
		0xcb, 0x95,			// CB RES 2, L
		0xcb, 0x97,			// CB RES 2, A
		0xcb, 0x98,			// CB RES 3, B
		0xcb, 0x99,			// CB RES 3, C
		0xcb, 0x9a,			// CB RES 3, D
		0xcb, 0x9b,			// CB RES 3, E
		0xcb, 0x9c,			// CB RES 3, H
		0xcb, 0x9d,			// CB RES 3, L
		0xcb, 0x9f,			// CB RES 3, A
		0xcb, 0xa0,			// CB RES 4, B
		0xcb, 0xa1,			// CB RES 4, C
		0xcb, 0xa2,			// CB RES 4, D
		0xcb, 0xa3,			// CB RES 4, E
		0xcb, 0xa4,			// CB RES 4, H
		0xcb, 0xa5,			// CB RES 4, L
		0xcb, 0xa7,			// CB RES 4, A
		0xcb, 0xa8,			// CB RES 5, B
		0xcb, 0xa9,			// CB RES 5, C
		0xcb, 0xaa,			// CB RES 5, D
		0xcb, 0xab,			// CB RES 5, E
		0xcb, 0xac,			// CB RES 5, H
		0xcb, 0xad,			// CB RES 5, L
		0xcb, 0xaf,			// CB RES 5, A
		0xcb, 0xb0,			// CB RES 6, B
		0xcb, 0xb1,			// CB RES 6, C
		0xcb, 0xb2,			// CB RES 6, D
		0xcb, 0xb3,			// CB RES 6, E
		0xcb, 0xb4,			// CB RES 6, H
		0xcb, 0xb5,			// CB RES 6, L
		0xcb, 0xb7,			// CB RES 6, A
		0xcb, 0xb8,			// CB RES 7, B
		0xcb, 0xb9,			// CB RES 7, C
		0xcb, 0xba,			// CB RES 7, D
		0xcb, 0xbb,			// CB RES 7, E
		0xcb, 0xbc,			// CB RES 7, H
		0xcb, 0xbd,			// CB RES 7, L
		0xcb, 0xbf,			// CB RES 7, A
	}));

	cpu.reset();

	for (auto i = 0; i < 7; ++i)
		cpu.step();

	struct result
	{
		std::uint8_t	value;
		std::uint8_t	flags;
	};

	std::vector<result> results =
	{
		result{ 0xfe, 0x00 },
		result{ 0xfc, 0x00 },
		result{ 0xf8, 0x00 },
		result{ 0xf0, 0x00 },
		result{ 0xe0, 0x00 },
		result{ 0xc0, 0x00 },
		result{ 0x80, 0x00 },
		result{ 0x00, 0x00 },
	};

	std::uint16_t addr = 0x0010;
	std::uint64_t cycle = 64;
	for (auto const& res : results)
	{
		for (auto const& reg : r8_registers)
		{
			cpu.step();

			EXPECT_EQ(cpu.get_register(reg), res.value);
			EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
			EXPECT_EQ(cpu.get_flags(), res.flags);
			EXPECT_EQ(cpu.get_cycle(), cycle);
			addr += 2;
			cycle += 8;
		}
	}
}

TEST(instructions, op_res_hl)
{
	// CB RES n, (HL)
	// 2 16
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xff,			// LD A, 0xff
		0x77,				// LD (HL), A
		0xcb, 0x86,			// CB RES 0, (HL)
		0xcb, 0x8e,			// CB RES 1, (HL)
		0xcb, 0x96,			// CB RES 2, (HL)
		0xcb, 0x9e,			// CB RES 3, (HL)
		0xcb, 0xa6,			// CB RES 4, (HL)
		0xcb, 0xae,			// CB RES 5, (HL)
		0xcb, 0xb6,			// CB RES 6, (HL)
		0xcb, 0xbe,			// CB RES 7, (HL)
		}));

	cpu.reset();

	run_n(cpu, 3);
	std::uint16_t addr = cpu.get_register(lr35902::r16::PC);
	std::uint64_t cycle = cpu.get_cycle();

	struct result
	{
		std::uint8_t	value;
		std::uint8_t	flags;
	};

	std::vector<result> results =
	{
		result{ 0xfe, 0x00 },
		result{ 0xfc, 0x00 },
		result{ 0xf8, 0x00 },
		result{ 0xf0, 0x00 },
		result{ 0xe0, 0x00 },
		result{ 0xc0, 0x00 },
		result{ 0x80, 0x00 },
		result{ 0x00, 0x00 },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 16;
		cpu.step();
		EXPECT_EQ(mmu[0xc000], res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_set_r8)
{
	// CB SET r8
	// 2 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0xcb, 0xc0,			// CB SET 0, B
		0xcb, 0xc1,			// CB SET 0, C
		0xcb, 0xc2,			// CB SET 0, D
		0xcb, 0xc3,			// CB SET 0, E
		0xcb, 0xc4,			// CB SET 0, H
		0xcb, 0xc5,			// CB SET 0, L
		0xcb, 0xc7,			// CB SET 0, A
		0xcb, 0xc8,			// CB SET 1, B
		0xcb, 0xc9,			// CB SET 1, C
		0xcb, 0xca,			// CB SET 1, D
		0xcb, 0xcb,			// CB SET 1, E
		0xcb, 0xcc,			// CB SET 1, H
		0xcb, 0xcd,			// CB SET 1, L
		0xcb, 0xcf,			// CB SET 1, A
		0xcb, 0xd0,			// CB SET 2, B
		0xcb, 0xd1,			// CB SET 2, C
		0xcb, 0xd2,			// CB SET 2, D
		0xcb, 0xd3,			// CB SET 2, E
		0xcb, 0xd4,			// CB SET 2, H
		0xcb, 0xd5,			// CB SET 2, L
		0xcb, 0xd7,			// CB SET 2, A
		0xcb, 0xd8,			// CB SET 3, B
		0xcb, 0xd9,			// CB SET 3, C
		0xcb, 0xda,			// CB SET 3, D
		0xcb, 0xdb,			// CB SET 3, E
		0xcb, 0xdc,			// CB SET 3, H
		0xcb, 0xdd,			// CB SET 3, L
		0xcb, 0xdf,			// CB SET 3, A
		0xcb, 0xe0,			// CB SET 4, B
		0xcb, 0xe1,			// CB SET 4, C
		0xcb, 0xe2,			// CB SET 4, D
		0xcb, 0xe3,			// CB SET 4, E
		0xcb, 0xe4,			// CB SET 4, H
		0xcb, 0xe5,			// CB SET 4, L
		0xcb, 0xe7,			// CB SET 4, A
		0xcb, 0xe8,			// CB SET 5, B
		0xcb, 0xe9,			// CB SET 5, C
		0xcb, 0xea,			// CB SET 5, D
		0xcb, 0xeb,			// CB SET 5, E
		0xcb, 0xec,			// CB SET 5, H
		0xcb, 0xed,			// CB SET 5, L
		0xcb, 0xef,			// CB SET 5, A
		0xcb, 0xf0,			// CB SET 6, B
		0xcb, 0xf1,			// CB SET 6, C
		0xcb, 0xf2,			// CB SET 6, D
		0xcb, 0xf3,			// CB SET 6, E
		0xcb, 0xf4,			// CB SET 6, H
		0xcb, 0xf5,			// CB SET 6, L
		0xcb, 0xf7,			// CB SET 6, A
		0xcb, 0xf8,			// CB SET 7, B
		0xcb, 0xf9,			// CB SET 7, C
		0xcb, 0xfa,			// CB SET 7, D
		0xcb, 0xfb,			// CB SET 7, E
		0xcb, 0xfc,			// CB SET 7, H
		0xcb, 0xfd,			// CB SET 7, L
		0xcb, 0xff,			// CB SET 7, A
	}));

	cpu.reset();

	struct result
	{
		std::uint8_t	value;
		std::uint8_t	flags;
	};

	std::vector<result> results =
	{
		result{ 0x01, 0x00 },
		result{ 0x03, 0x00 },
		result{ 0x07, 0x00 },
		result{ 0x0f, 0x00 },
		result{ 0x1f, 0x00 },
		result{ 0x3f, 0x00 },
		result{ 0x7f, 0x00 },
		result{ 0xff, 0x00 },
	};

	std::uint16_t addr = 0x0002;
	std::uint64_t cycle = 8;

	for (auto const& res : results)
	{
		for (auto const& reg : r8_registers)
		{
			cpu.step();
			EXPECT_EQ(cpu.get_register(reg), res.value);
			EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
			EXPECT_EQ(cpu.get_flags(), res.flags);
			EXPECT_EQ(cpu.get_cycle(), cycle);
			addr += 2;
			cycle += 8;
		}
	}
}

TEST(instructions, op_set_hl)
{
	// CB SET n, (HL)
	// 2 16
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xff,			// LD A, 0xff
		0x77,				// LD (HL), A
		0xcb, 0xc6,			// CB SET 0, (HL)
		0xcb, 0xce,			// CB SET 1, (HL)
		0xcb, 0xd6,			// CB SET 2, (HL)
		0xcb, 0xde,			// CB SET 3, (HL)
		0xcb, 0xe6,			// CB SET 4, (HL)
		0xcb, 0xee,			// CB SET 5, (HL)
		0xcb, 0xf6,			// CB SET 6, (HL)
		0xcb, 0xfe,			// CB SET 7, (HL)
	}));

	cpu.reset();

	run_n(cpu, 3);
	std::uint16_t addr = cpu.get_register(lr35902::r16::PC);
	std::uint64_t cycle = cpu.get_cycle();

	struct result
	{
		std::uint8_t	value;
		std::uint8_t	flags;
	};

	std::vector<result> results =
	{
		result{ 0x01, 0x00 },
		result{ 0x03, 0x00 },
		result{ 0x07, 0x00 },
		result{ 0x0f, 0x00 },
		result{ 0x1f, 0x00 },
		result{ 0x3f, 0x00 },
		result{ 0x7f, 0x00 },
		result{ 0xff, 0x00 },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 16;
		cpu.step();
		EXPECT_EQ(mmu[0xc000], res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}
