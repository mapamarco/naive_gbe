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

struct result
{
	std::uint8_t	value	= 0x00;
	std::uint8_t	flags	= 0x00;
	lr35902::r8		reg = lr35902::r8::A;
};

void step_n(lr35902& cpu, std::size_t n, std::uint16_t& addr, std::uint64_t& cycle)
{
	for (auto i = 0; i < n; ++i)
		cpu.step();

	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();
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

TEST(instructions, op_ret)
{
	// RET addr
	// 1 16
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x31, 0xfe, 0xff,	// LD SP, 0xfffe
		0xcf,				// RST 0x08
		0x00,				// NOP
		0x00,				// NOP
		0x00,				// NOP
		0x00,				// NOP
		0xc9,				// RET
	}));

	cpu.reset();

	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xfffc);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 0x0008);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xfffe);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 32);
}

TEST(instructions, op_rst)
{
	// RST addr
	// 1 16
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	struct rst_result
	{
		std::uint8_t opcode;
		std::uint16_t addr;
	};

	std::vector<rst_result> rst_ops =
	{
		{ 0xc7, 0x0000 },
		{ 0xcf, 0x0008 },
		{ 0xd7, 0x0010 },
		{ 0xdf, 0x0018 },
		{ 0xe7, 0x0020 },
		{ 0xef, 0x0028 },
		{ 0xf7, 0x0030 },
		{ 0xff, 0x0038 },
	};

	for (auto const& res : rst_ops)
	{
		mmu.set_cartridge(cartridge_buf({
			0x31, 0xfe, 0xff,	// LD SP, 0xfffe
			res.opcode,			// RST 0x00
		}));

		cpu.reset();

		step_n(cpu, 1, addr, cycle);

		cpu.step();
		EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xfffc);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), res.addr);
		EXPECT_EQ(cpu.get_flags(), 0x00);
		EXPECT_EQ(cpu.get_cycle(), cycle + 16);
	}
}

TEST(instructions, op_push)
{
	// PUSH r16
	// 1 16
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x31, 0xfe, 0xff,	// LD SP, 0xfffe
		0x01, 0x22, 0x11,	// LD BC, 0x1122
		0x11, 0x44, 0x33,	// LD DE, 0x3344
		0x21, 0x66, 0x55,	// LD HL, 0x5566
		0x3e, 0x77,			// LD A, 0x77
		0x37,				// CCF
		0xc5,				// PUSH BC
		0xd5,				// PUSH DE
		0xe5,				// PUSH HL
		0xf5,				// PUSH AF
	}));

	cpu.reset();

	step_n(cpu, 6, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xfffc);
	EXPECT_EQ(mmu[0xfffd], 0x11);
	EXPECT_EQ(mmu[0xfffc], 0x22);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xfffa);
	EXPECT_EQ(mmu[0xfffb], 0x33);
	EXPECT_EQ(mmu[0xfffa], 0x44);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 32);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 3);
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xfff8);
	EXPECT_EQ(mmu[0xfff9], 0x55);
	EXPECT_EQ(mmu[0xfff8], 0x66);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 48);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 4);
	EXPECT_EQ(cpu.get_register(lr35902::r16::SP), 0xfff6);
	EXPECT_EQ(mmu[0xfff7], 0x77);
	EXPECT_EQ(mmu[0xfff6], 0x10);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 64);
}

TEST(instructions, op_pop)
{
	// POP r16
	// 1 12
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x31, 0xfe, 0xff,	// LD SP, 0xfffe
		0x01, 0x22, 0x11,	// LD BC, 0x1122
		0x11, 0x44, 0x33,	// LD DE, 0x3344
		0x21, 0x66, 0x55,	// LD HL, 0x5566
		0x3e, 0x77,			// LD A, 0x77
		0x37,				// CCF
		0xc5,				// PUSH BC
		0xd5,				// PUSH DE
		0xe5,				// PUSH HL
		0xf5,				// PUSH AF
		0x01, 0x00, 0x00,	// LD BC, 0x0000
		0x11, 0x00, 0x00,	// LD DE, 0x0000
		0x21, 0x00, 0x00,	// LD HL, 0x0000
		0x3e, 0x00,			// LD A, 0x00
		0x3f,				// SCF
		0xf1,				// POP AF
		0xe1,				// POP HL
		0xd1,				// POP DE
		0xc1,				// POP BC
	}));

	cpu.reset();

	step_n(cpu, 15, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::AF), 0x7710);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 12);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0x5566);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 24);


	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 3);
	EXPECT_EQ(cpu.get_register(lr35902::r16::DE), 0x3344);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 36);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 4);
	EXPECT_EQ(cpu.get_register(lr35902::r16::BC), 0x1122);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 48);

}

TEST(instructions, op_adc_hl)
{
	// ADC A, (HL)
	// 2 8
	// Z 0 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xc6,			// LD A, 0xc6
		0x77,				// LD (HL), A
		0x3e, 0x3a,			// LD A, 0x3a
		0x8e,				// ADC A, (HL)
		0x3e, 0x0f,			// LD A, 0x0f
		0x8e,				// ADC A, (HL)
	}));

	cpu.reset();

	step_n(cpu, 4, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_flags(), 0xb0);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(mmu[0xc000], 0x10);
	EXPECT_EQ(cpu.get_flags(), 0x20);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_adc_r8)
{
	// ADC A, r8
	// 1 4
	// Z 0 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0xc6,			// LD B, 0xc6
		0x0e, 0x0f,			// LD C, 0x0f
		0x16, 0x01,			// LD D, 0x01
		0x1e, 0x02,			// LD E, 0x02
		0x26, 0x03,			// LD H, 0x03
		0x2e, 0x04,			// LD L, 0x04
		0x3e, 0x3a,			// LD A, 0x3a
		0x88,				// ADC A, B
		0x89,				// ADC A, C
		0x8a,				// ADC A, D
		0x8b,				// ADC A, E
		0x8c,				// ADC A, L
		0x8d,				// ADC A, H
		0x8f,				// ADC A, A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x00, 0xb0 },
		result{ 0x10, 0x20 },
		result{ 0x11, 0x00 },
		result{ 0x13, 0x00 },
		result{ 0x16, 0x00 },
		result{ 0x1a, 0x00 },
		result{ 0x34, 0x20 },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(lr35902::r8::A), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_scf)
{
	// SCF
	// 1 4
	// - 0 0 1

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x3f,				// SCF
		0x3f,				// SCF
		0x3f,				// SCF
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 1);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), 4);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), 8);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 3);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), 12);
}

TEST(instructions, op_ccf)
{
	// CCF
	// 1 4
	// - 0 0 1

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x37,				// CCF
		0x37,				// CCF
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 1);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), 4);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 2);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), 8);
}

TEST(instructions, op_sbc_r8)
{
	// SBC A, r8
	// 1 4
	// Z 1 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x01,			// LD B, 0x01
		0x0e, 0x02,			// LD C, 0x02
		0x16, 0x03,			// LD D, 0x03
		0x1e, 0x04,			// LD E, 0x04
		0x26, 0x05,			// LD H, 0x05
		0x2e, 0x06,			// LD L, 0x06
		0x3e, 0x0f,			// LD A, 0x0f
		0x98,				// SBC A, B
		0x99,				// SBC A, C
		0x9a,				// SBC A, D
		0x9b,				// SBC A, E
		0x9c,				// SBC A, H
		0x9d,				// SBC A, L
		0x9f,				// SBC A, A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x0e, 0x40, lr35902::r8::B },
		result{ 0x0c, 0x40, lr35902::r8::C },
		result{ 0x09, 0x40, lr35902::r8::D },
		result{ 0x05, 0x40, lr35902::r8::E },
		result{ 0x00, 0xc0, lr35902::r8::H },
		result{ 0xfa, 0x70, lr35902::r8::L },
		result{ 0xff, 0x70, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(lr35902::r8::A), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_sbc_hl)
{
	// SBC A, (HL)
	// 1 8
	// Z 1 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x36, 0xff,			// LD (HL), 0xff
		0x9e,				// SBC A, (HL)
		0x3e, 0xff,			// LD A, 0xff
		0x9e,				// SBC A, (HL)
		0x36, 0x03,			// LD (HL), 0x03
		0x37,				// CCF
		0x9e,				// SBC A, (HL)
	}));

	cpu.reset();

	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x01);
	EXPECT_EQ(cpu.get_flags(), 0x70);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xff);
	EXPECT_EQ(cpu.get_flags(), 0x70);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xfb);
	EXPECT_EQ(cpu.get_flags(), 0x40);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_sub_r8)
{
	// SUB A, r8
	// 1 4
	// Z 1 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x01,			// LD B, 0x01
		0x0e, 0x02,			// LD C, 0x02
		0x16, 0x03,			// LD D, 0x03
		0x1e, 0x04,			// LD E, 0x04
		0x26, 0x05,			// LD H, 0x05
		0x2e, 0x06,			// LD L, 0x06
		0x3e, 0x0f,			// LD A, 0x0f
		0x90,				// SUB A, B
		0x91,				// SUB A, C
		0x92,				// SUB A, D
		0x93,				// SUB A, E
		0x94,				// SUB A, H
		0x95,				// SUB A, L
		0x97,				// SUB A, A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x0e, 0x40, lr35902::r8::B },
		result{ 0x0c, 0x40, lr35902::r8::C },
		result{ 0x09, 0x40, lr35902::r8::D },
		result{ 0x05, 0x40, lr35902::r8::E },
		result{ 0x00, 0xc0, lr35902::r8::H },
		result{ 0xfa, 0x70, lr35902::r8::L },
		result{ 0x00, 0xc0, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(lr35902::r8::A), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_sub_hl)
{
	// SUB A, (HL)
	// 1 8
	// Z 1 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x36, 0xff,			// LD (HL), 0xff
		0x96,				// SUB A, (HL)
		0x3e, 0xff,			// LD A, 0xff
		0x96,				// SUB A, (HL)
		0x36, 0x03,			// LD (HL), 0x03
		0x37,				// CCF
		0x96,				// SUB A, (HL)
	}));

	cpu.reset();

	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x01);
	EXPECT_EQ(cpu.get_flags(), 0x70);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x00);
	EXPECT_EQ(cpu.get_flags(), 0xc0);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xfd);
	EXPECT_EQ(cpu.get_flags(), 0x70);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_add_r8)
{
	// ADD A, r8
	// 1 4
	// Z 0 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0xc6,			// LD B, 0xc6
		0x0e, 0x0f,			// LD C, 0x0f
		0x16, 0x01,			// LD D, 0x01
		0x1e, 0x02,			// LD E, 0x02
		0x26, 0x03,			// LD H, 0x03
		0x2e, 0x04,			// LD L, 0x04
		0x3e, 0x3a,			// LD A, 0x3a
		0x80,				// ADD A, B
		0x81,				// ADD A, C
		0x82,				// ADD A, D
		0x83,				// ADD A, E
		0x84,				// ADD A, L
		0x85,				// ADD A, H
		0x87,				// ADD A, A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x00, 0xb0 },
		result{ 0x0f, 0x00 },
		result{ 0x10, 0x20 },
		result{ 0x12, 0x00 },
		result{ 0x15, 0x00 },
		result{ 0x19, 0x00 },
		result{ 0x32, 0x20 },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(lr35902::r8::A), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_add_r8_hl)
{
	// ADD A, (HL)
	// 2 8
	// Z 0 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xc6,			// LD A, 0xc6
		0x77,				// LD (HL), A
		0x3e, 0x3a,			// LD A, 0x3a
		0x86,				// ADD A, (HL)
		0x3e, 0x0f,			// LD A, 0x0f
		0x86,				// ADD A, (HL)
	}));

	cpu.reset();

	step_n(cpu, 4, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x00);
	EXPECT_EQ(cpu.get_flags(), 0xb0);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xd5);
	EXPECT_EQ(cpu.get_flags(), 0x20);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_di)
{
	// DI
	// 1 4
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0xfb,				// EI
		0xf3,				// DI
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_ime(), 1);
	EXPECT_EQ(cpu.get_cycle(), 4);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_ime(), 0);
	EXPECT_EQ(cpu.get_cycle(), 8);
}

TEST(instructions, op_ei)
{
	// EI
	// 1 4
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0xf3,				// DI
		0xfb,				// EI
	}));

	cpu.reset();

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_ime(), 0);
	EXPECT_EQ(cpu.get_cycle(), 4);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_ime(), 1);
	EXPECT_EQ(cpu.get_cycle(), 8);
}

TEST(instructions, op_add_hl_r16)
{
	// ADD HL, r16
	// 1 8
	// - 0 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x31, 0x22, 0x11,	// LD SP, 0x1122
		0x01, 0x44, 0x33,	// LD BC, 0x3344
		0x11, 0x66, 0x55,	// LD DE, 0x5566
		0x21, 0x01, 0x00,	// LD HL, 0x0001
		0x09,				// ADD HL, BC
		0x19,				// ADD HL, DE
		0x29,				// ADD HL, HL
		0x39,				// ADD HL, SP
	}));

	cpu.reset();

	step_n(cpu, 4, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0x3345);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0x88ab);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 3);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0x1156);
	EXPECT_EQ(cpu.get_flags(), 0x30);
	EXPECT_EQ(cpu.get_cycle(), cycle + 24);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 4);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0x2278);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 32);
}

TEST(instructions, op_ldi_r8)
{
	// LDI A, (HL)
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xf1,			// LD A, 0xf1
		0x77,				// LD (HL), A
		0xaf,				// XOR A
		0x2a,				// LDI A, (HL)
	}));

	cpu.reset();

	step_n(cpu, 4, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xf1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0xc001);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_ldi_hl)
{
	// LDI (HL), A
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xf1,			// LD A, 0xf1
		0x22,				// LDI (HL), A
	}));

	cpu.reset();

	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0xc001);
	EXPECT_EQ(mmu[0xc000], 0xf1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_ldd_r8)
{
	// LDD A, (HL)
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x01, 0xc0,	// LD HL, 0xc001
		0x3e, 0xf1,			// LD A, 0xf1
		0x77,				// LD (HL), A
		0xaf,				// XOR A
		0x3a,				// LDD A, (HL)
	}));

	cpu.reset();

	step_n(cpu, 4, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xf1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0xc000);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_ldd_hl)
{
	// LDD (HL), A
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x01, 0xc0,	// LD HL, 0xc001
		0x3e, 0x34,			// LD A, 0x34
		0x32,				// LDD (HL), A
	}));

	cpu.reset();
	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_register(lr35902::r16::HL), 0xc000);
	EXPECT_EQ(mmu[0xc001], 0x34);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_ld_r8_hl)
{
	// LD r8, (HL)
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x77,			// LD A, 0x11
		0x77,				// LD (HL), A
		0x46,				// LD B, (HL)
		0x77,				// LD (HL), A
		0x4e,				// LD C, (HL)
		0x77,				// LD (HL), A
		0x56,				// LD D, (HL)
		0x77,				// LD (HL), A
		0x5e,				// LD E, (HL)
		0x77,				// LD (HL), A
		0x66,				// LD H, (HL)
		0x77,				// LD (HL), A
		0x6e,				// LD L, (HL)
		0x77,				// LD (HL), A
		0x7e,				// LD A, (HL)
		}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x77, 0x00, lr35902::r8::B },
		result{ 0x77, 0x00, lr35902::r8::C },
		result{ 0x77, 0x00, lr35902::r8::D },
		result{ 0x77, 0x00, lr35902::r8::E },
		result{ 0x77, 0x00, lr35902::r8::H },
		result{ 0x77, 0x00, lr35902::r8::L },
		result{ 0x77, 0x00, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 8;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);

		step_n(cpu, 1, addr, cycle);
	}
}

TEST(instructions, op_ld_hl)
{
	// LD (HL), u8
	// 2 12
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x36, 0x12,			// LD (HL), 0x12
		0x36, 0x23,			// LD (HL), 0x23
	}));

	cpu.reset();

	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x12);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 12);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x23);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 4);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 24);
}

TEST(instructions, op_ld_hl_r8)
{
	// LD (HL), r8
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x12,			// LD B, 0x12
		0x0e, 0x23,			// LD C, 0x23
		0x16, 0x34,			// LD D, 0x34
		0x1e, 0x45,			// LD E, 0x45
		0x26, 0xc0,			// LD H, 0xc0
		0x2e, 0x00,			// LD L, 0x00
		0x3e, 0x78,			// LD A, 0x78
		0x70,				// LD (HL), B
		0x71,				// LD (HL), C
		0x72,				// LD (HL), D
		0x73,				// LD (HL), E
		0x74,				// LD (HL), H
		0x75,				// LD (HL), L
		0x77,				// LD (HL), A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x12, 0x00, lr35902::r8::B },
		result{ 0x23, 0x00, lr35902::r8::C },
		result{ 0x34, 0x00, lr35902::r8::D },
		result{ 0x45, 0x00, lr35902::r8::E },
		result{ 0xc0, 0x00, lr35902::r8::H },
		result{ 0x00, 0x00, lr35902::r8::L },
		result{ 0x78, 0x00, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 8;

		EXPECT_EQ(mmu[0xc000], res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_xor_r8)
{
	// XOR r8
	// 1 4
	// Z 0 0 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x12,			// LD B, 0x12
		0x0e, 0x23,			// LD C, 0x23
		0x16, 0x34,			// LD D, 0x34
		0x1e, 0x45,			// LD E, 0x45
		0x26, 0x56,			// LD H, 0x56
		0x2e, 0x67,			// LD L, 0x67
		0x3e, 0x78,			// LD A, 0x78
		0xa8,				// XOR B
		0xa9,				// XOR C
		0xaa,				// XOR D
		0xab,				// XOR E
		0xac,				// XOR H
		0xad,				// XOR L
		0xaf,				// XOR A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x6a, 0x00, lr35902::r8::A },
		result{ 0x49, 0x00, lr35902::r8::A },
		result{ 0x7d, 0x00, lr35902::r8::A },
		result{ 0x38, 0x00, lr35902::r8::A },
		result{ 0x6e, 0x00, lr35902::r8::A },
		result{ 0x09, 0x00, lr35902::r8::A },
		result{ 0x00, 0x80, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_xor_hl)
{
	// XOR (HL)
	// 1 8
	// Z 0 0 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x11,			// LD A, 0x11
		0x77,				// LD (HL), A
		0x3e, 0x22,			// LD A, 0x22
		0xae,				// XOR (HL)
	}));

	cpu.reset();

	step_n(cpu, 4, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x33);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_or_r8)
{
	// OR r8
	// 1 4
	// Z 0 0 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x00,			// LD B, 0x00
		0x0e, 0x01,			// LD C, 0x01
		0x16, 0x02,			// LD D, 0x02
		0x1e, 0x04,			// LD E, 0x04
		0x26, 0x08,			// LD H, 0x08
		0x2e, 0x10,			// LD L, 0x10
		0x3e, 0x00,			// LD A, 0x00
		0xb0,				// OR B
		0xb1,				// OR C
		0xb2,				// OR D
		0xb3,				// OR E
		0xb4,				// OR H
		0xb5,				// OR L
		0xb7,				// OR A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x00, 0x80, lr35902::r8::A },
		result{ 0x01, 0x00, lr35902::r8::A },
		result{ 0x03, 0x00, lr35902::r8::A },
		result{ 0x07, 0x00, lr35902::r8::A },
		result{ 0x0f, 0x00, lr35902::r8::A },
		result{ 0x1f, 0x00, lr35902::r8::A },
		result{ 0x1f, 0x00, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_or_hl)
{
	// OR (HL)
	// 1 8
	// Z 0 0 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0xb6,				// OR (HL)
		0x3e, 0xc3,			// LD A, 0xc3
		0x77,				// LD (HL), A
		0x3e, 0xa5,			// LD A, 0xa5
		0xb6,				// OR (HL)
		}));

	cpu.reset();
	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xe7);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_cp_r8)
{
	// CP r8
	// 1 4
	// Z 1 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x00,			// LD B, 0x00
		0x0e, 0x40,			// LD C, 0x40
		0x16, 0x3c,			// LD D, 0x3c
		0x1e, 0x00,			// LD E, 0x00
		0x26, 0x40,			// LD H, 0x40
		0x2e, 0x3c,			// LD L, 0x3c
		0x3e, 0x3c,			// LD A, 0x3c
		0xb8,				// CP B
		0xb9,				// CP C
		0xba,				// CP D
		0xbb,				// CP E
		0xbc,				// CP H
		0xbd,				// CP L
		0xbf,				// CP A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x00, 0x40 },
		result{ 0x00, 0x50 },
		result{ 0x00, 0xc0 },
		result{ 0x00, 0x40 },
		result{ 0x00, 0x50 },
		result{ 0x00, 0xc0 },
		result{ 0x00, 0xc0 },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_cp_hl)
{
	// CP (HL)
	// 1 8
	// Z 1 H C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x40,			// LD A, 0x40
		0x77,				// LD (HL), A
		0x3e, 0x3c,			// LD A, 0x3c
		0xbe,				// CP (HL)
		0x3e, 0x40,			// LD A, 0x40
		0xbe,				// CP (HL)
	}));

	cpu.reset();
	step_n(cpu, 4, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x50);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 1, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0xc0);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_undefined)
{
	// UNDEF
	// 1 4
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0xd3,				// UNDEF
		0xdb,				// UNDEF
		0xdd,				// UNDEF
		0xe3,				// UNDEF
		0xe4,				// UNDEF
		0xeb,				// UNDEF
		0xec,				// UNDEF
		0xed,				// UNDEF
		0xf4,				// UNDEF
		0xfc,				// UNDEF
		0xfd,				// UNDEF
	}));

	cpu.reset();

	std::vector<result> results =
	{
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
		result{ 0x00, 0x00 },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_cb)
{
	// CB
	// 0 0
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x3e, 0x0f,			// LD A, 0x0f
		0xcb, 0x37,			// CB SWAP A
	}));

	cpu.reset();

	step_n(cpu, 1, addr, cycle);

	cpu.step();

	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0xf0);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_and_r8)
{
	// AND r8
	// 1 4
	// Z 0 1 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0xfe,			// LD B, 0xfe
		0x0e, 0xfc,			// LD C, 0xfc
		0x16, 0xf8,			// LD D, 0xf8
		0x1e, 0xf0,			// LD E, 0xf0
		0x26, 0xe0,			// LD H, 0xe0
		0x2e, 0x0f,			// LD L, 0x0f
		0x3e, 0xff,			// LD A, 0xff
		0xa0,				// AND B
		0xa1,				// AND C
		0xa2,				// AND D
		0xa3,				// AND E
		0xa4,				// AND H
		0xa5,				// AND L
		0xa7,				// AND A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0xfe, 0x20, lr35902::r8::A },
		result{ 0xfc, 0x20, lr35902::r8::A },
		result{ 0xf8, 0x20, lr35902::r8::A },
		result{ 0xf0, 0x20, lr35902::r8::A },
		result{ 0xe0, 0x20, lr35902::r8::A },
		result{ 0x00, 0xa0, lr35902::r8::A },
		result{ 0x00, 0xa0, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_and_hl)
{
	// AND (HL)
	// 1 8
	// Z 0 1 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xc3,			// LD A, 0xc3
		0xa6,				// AND (HL)
		0x3e, 0xc3,			// LD A, 0xc3
		0x77,				// LD (HL), A
		0x3e, 0xa5,			// LD A, 0xa5
		0xa6,				// AND (HL)
	}));

	cpu.reset();
	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0xa0);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x81);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x20);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
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
		0x26, 0x56,			// LD H, 0x56
		0x2e, 0x67,			// LD L, 0x67
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

TEST(instructions, op_ld_r8_r8)
{
	// LD r8, r8
	// 1 4
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x00,			// LD B, 0x00
		0x0e, 0x01,			// LD C, 0x01
		0x16, 0x02,			// LD D, 0x02
		0x1e, 0x03,			// LD E, 0x03
		0x26, 0x04,			// LD H, 0x04
		0x2e, 0x05,			// LD L, 0x05
		0x3e, 0x06,			// LD A, 0x06
		0x40,				// LD B, B
		0x41,				// LD B, C
		0x42,				// LD B, D
		0x43,				// LD B, E
		0x44,				// LD B, L
		0x45,				// LD B, H
		0x47,				// LD B, A
		0x48,				// LD C, B
		0x49,				// LD C, C
		0x4a,				// LD C, D
		0x4b,				// LD C, E
		0x4c,				// LD C, L
		0x4d,				// LD C, H
		0x4f,				// LD C, A
		0x50,				// LD D, B
		0x51,				// LD D, C
		0x52,				// LD D, D
		0x53,				// LD D, E
		0x54,				// LD D, L
		0x55,				// LD D, H
		0x57,				// LD D, A
		0x58,				// LD E, B
		0x59,				// LD E, C
		0x5a,				// LD E, D
		0x5b,				// LD E, E
		0x5c,				// LD E, L
		0x5d,				// LD E, H
		0x5f,				// LD E, A
		0x60,				// LD H, B
		0x61,				// LD H, C
		0x62,				// LD H, D
		0x63,				// LD H, E
		0x64,				// LD H, L
		0x65,				// LD H, H
		0x67,				// LD H, A
		0x68,				// LD L, B
		0x69,				// LD L, C
		0x6a,				// LD L, D
		0x6b,				// LD L, E
		0x6c,				// LD L, L
		0x6d,				// LD L, H
		0x6f,				// LD L, A
		0x78,				// LD A, B
		0x79,				// LD A, C
		0x7a,				// LD A, D
		0x7b,				// LD A, E
		0x7c,				// LD A, L
		0x7d,				// LD A, H
		0x7f,				// LD A, A
	}));

	cpu.reset();
	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x00, 0x00, lr35902::r8::B },
		result{ 0x01, 0x00, lr35902::r8::B },
		result{ 0x02, 0x00, lr35902::r8::B },
		result{ 0x03, 0x00, lr35902::r8::B },
		result{ 0x04, 0x00, lr35902::r8::B },
		result{ 0x05, 0x00, lr35902::r8::B },
		result{ 0x06, 0x00, lr35902::r8::B },
		result{ 0x06, 0x00, lr35902::r8::C },
		result{ 0x06, 0x00, lr35902::r8::C },
		result{ 0x02, 0x00, lr35902::r8::C },
		result{ 0x03, 0x00, lr35902::r8::C },
		result{ 0x04, 0x00, lr35902::r8::C },
		result{ 0x05, 0x00, lr35902::r8::C },
		result{ 0x06, 0x00, lr35902::r8::C },
		result{ 0x06, 0x00, lr35902::r8::D },
		result{ 0x06, 0x00, lr35902::r8::D },
		result{ 0x06, 0x00, lr35902::r8::D },
		result{ 0x03, 0x00, lr35902::r8::D },
		result{ 0x04, 0x00, lr35902::r8::D },
		result{ 0x05, 0x00, lr35902::r8::D },
		result{ 0x06, 0x00, lr35902::r8::D },
		result{ 0x06, 0x00, lr35902::r8::E },
		result{ 0x06, 0x00, lr35902::r8::E },
		result{ 0x06, 0x00, lr35902::r8::E },
		result{ 0x06, 0x00, lr35902::r8::E },
		result{ 0x04, 0x00, lr35902::r8::E },
		result{ 0x05, 0x00, lr35902::r8::E },
		result{ 0x06, 0x00, lr35902::r8::E },
		result{ 0x06, 0x00, lr35902::r8::H },
		result{ 0x06, 0x00, lr35902::r8::H },
		result{ 0x06, 0x00, lr35902::r8::H },
		result{ 0x06, 0x00, lr35902::r8::H },
		result{ 0x06, 0x00, lr35902::r8::H },
		result{ 0x05, 0x00, lr35902::r8::H },
		result{ 0x06, 0x00, lr35902::r8::H },
		result{ 0x06, 0x00, lr35902::r8::L },
		result{ 0x06, 0x00, lr35902::r8::L },
		result{ 0x06, 0x00, lr35902::r8::L },
		result{ 0x06, 0x00, lr35902::r8::L },
		result{ 0x06, 0x00, lr35902::r8::L },
		result{ 0x06, 0x00, lr35902::r8::L },
		result{ 0x06, 0x00, lr35902::r8::L },
		result{ 0x06, 0x00, lr35902::r8::A },
		result{ 0x06, 0x00, lr35902::r8::A },
		result{ 0x06, 0x00, lr35902::r8::A },
		result{ 0x06, 0x00, lr35902::r8::A },
		result{ 0x06, 0x00, lr35902::r8::A },
		result{ 0x06, 0x00, lr35902::r8::A },
		result{ 0x06, 0x00, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
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

TEST(instructions, op_inc_r8)
{
	// INC r8
	// 1 4
	// Z 0 H -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x00,			// LD B, 0x00
		0x0e, 0x01,			// LD C, 0x01
		0x16, 0x02,			// LD D, 0x02
		0x1e, 0x03,			// LD E, 0x03
		0x26, 0x04,			// LD H, 0x04
		0x2e, 0x0f,			// LD L, 0x0f
		0x3e, 0xff,			// LD A, 0xff
		0x04,				// INC B
		0x0c,				// INC C
		0x14,				// INC D
		0x1c,				// INC E
		0x24,				// INC H
		0x2c,				// INC L
		0x3c,				// INC A
	}));

	cpu.reset();
	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x01, 0x00, lr35902::r8::B },
		result{ 0x02, 0x00, lr35902::r8::C },
		result{ 0x03, 0x00, lr35902::r8::D },
		result{ 0x04, 0x00, lr35902::r8::E },
		result{ 0x05, 0x00, lr35902::r8::H },
		result{ 0x10, 0x20, lr35902::r8::L },
		result{ 0x00, 0x80, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}

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

TEST(instructions, op_inc_hl)
{
	// INC (HL)
	// 1 12
	// Z 0 H -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xff,			// LD A, 0xff
		0x77,				// LD (HL), A
		0x34,				// INC (HL)
		0x34,				// INC (HL)
	}));

	cpu.reset();
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 12);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x01);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 24);
}


TEST(instructions, op_dec_hl)
{
	// DEC (HL)
	// 1 12
	// Z 1 H -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x01,			// LD A, 0x01
		0x77,				// LD (HL), A
		0x35,				// DEC (HL)
		0x35,				// DEC (HL)
	}));

	cpu.reset();
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0xc0);
	EXPECT_EQ(cpu.get_cycle(), cycle + 12);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0xff);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x60);
	EXPECT_EQ(cpu.get_cycle(), cycle + 24);
}

TEST(instructions, op_dec_r8)
{
	// DEC r8
	// 1 4
	// Z 1 H -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x00,			// LD B, 0x00
		0x0e, 0x01,			// LD C, 0x01
		0x16, 0x02,			// LD D, 0x02
		0x1e, 0x03,			// LD E, 0x03
		0x26, 0x04,			// LD H, 0x04
		0x2e, 0x0f,			// LD L, 0x0f
		0x3e, 0xff,			// LD A, 0xff
		0x05,				// DEC B
		0x0d,				// DEC C
		0x15,				// DEC D
		0x1d,				// DEC E
		0x25,				// DEC H
		0x2d,				// DEC L
		0x3d,				// DEC A
	}));

	cpu.reset();
	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0xff, 0x60, lr35902::r8::B },
		result{ 0x00, 0xc0, lr35902::r8::C },
		result{ 0x01, 0x40, lr35902::r8::D },
		result{ 0x02, 0x40, lr35902::r8::E },
		result{ 0x03, 0x40, lr35902::r8::H },
		result{ 0x0e, 0x40, lr35902::r8::L },
		result{ 0xfe, 0x60, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		cpu.step();
		addr += 1;
		cycle += 4;

		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}

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

TEST(instructions, op_ld_bc_r8)
{
	// LD (BC), A
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x3e, 0x33,			// LD A, 0x33
		0x01, 0x00, 0xc0,	// LD BC, 0xc000
		0x02,				// LD (BC), A
	}));

	cpu.reset();
	step_n(cpu, 2, addr, cycle);

	cpu.step();

	EXPECT_EQ(mmu[0xc000], 0x33);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_ld_de_r8)
{
	// LD (DE), A
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x3e, 0x33,			// LD A, 0x33
		0x11, 0x00, 0xc0,	// LD DE, 0xc000
		0x12,				// LD (DE), A
	}));

	cpu.reset();
	step_n(cpu, 2, addr, cycle);

	cpu.step();

	EXPECT_EQ(mmu[0xc000], 0x33);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}


TEST(instructions, op_ld_r8_bc)
{
	// LD A, (BC)
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x3e, 0x22,			// LD A, 0x22
		0x01, 0x00, 0xc0,	// LD BC, 0xc000
		0x02,				// LD (BC), A
		0x3e, 0x44,			// LD A, 0x44
		0x0a,				// LD A, (BC)
	}));

	cpu.reset();
	step_n(cpu, 4, addr, cycle);

	cpu.step();

	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x22);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
}

TEST(instructions, op_ld_r8_de)
{
	// LD A, (DE)
	// 1 8
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x3e, 0x22,			// LD A, 0x22
		0x11, 0x00, 0xc0,	// LD DE, 0xc000
		0x12,				// LD (DE), A
		0x3e, 0x44,			// LD A, 0x44
		0x1a,				// LD A, (DE)
	}));

	cpu.reset();
	step_n(cpu, 4, addr, cycle);

	cpu.step();

	EXPECT_EQ(cpu.get_register(lr35902::r8::A), 0x22);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 1);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 8);
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

TEST(instructions, op_swap_r8)
{
	// CB SWAP r8
	// 2 8
	// Z 0 0 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x0f,			// LD B, 0x0f
		0x0e, 0x0f,			// LD C, 0x0f
		0x16, 0x0f,			// LD D, 0x0f
		0x1e, 0x0f,			// LD E, 0x0f
		0x26, 0x0f,			// LD H, 0x0f
		0x2e, 0x0f,			// LD L, 0x0f
		0x3e, 0x0f,			// LD A, 0x0f
		0xcb, 0x30,			// CB SWAP B
		0xcb, 0x31,			// CB SWAP C
		0xcb, 0x32,			// CB SWAP D
		0xcb, 0x33,			// CB SWAP E
		0xcb, 0x34,			// CB SWAP H
		0xcb, 0x35,			// CB SWAP L
		0xcb, 0x37,			// CB SWAP A
		0x06, 0x00,			// LD B, 0x00
		0xcb, 0x30,			// CB SWAP B
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	for (auto const& reg : r8_registers)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(reg), 0xf0);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), 0x00);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}

	addr += 4;
	cycle += 16;
	cpu.step();
	cpu.step();
	EXPECT_EQ(cpu.get_register(lr35902::r8::B), 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle);
}

TEST(instructions, op_swap_hl)
{
	// CB SWAP (HL)
	// 2 16
	// Z 0 0 0

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x9a,			// LD A, 0x9a
		0x77,				// LD (HL), A
		0xcb, 0x36,			// CB SWAP (HL)
		0xaf,				// XOR A
		0x77,				// LD (HL), A
		0xcb, 0x36,			// CB SWAP (HL)
	}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0xa9);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}

TEST(instructions, op_bit_r8)
{
	// CB BIT r8
	// 2 8
	// Z 0 1 -

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x9d,			// LD B, 0x9d
		0x0e, 0x9d,			// LD C, 0x9d
		0x16, 0x9d,			// LD D, 0x9d
		0x1e, 0x9d,			// LD E, 0x9d
		0x26, 0x9d,			// LD H, 0x9d
		0x2e, 0x9d,			// LD L, 0x9d
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

	step_n(cpu, 7, addr, cycle);

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
		for (auto const& reg : r8_registers)
		{
			cpu.step();
			addr += 2;
			cycle += 8;

			EXPECT_EQ(cpu.get_register(reg), res.value);
			EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
			EXPECT_EQ(cpu.get_flags(), res.flags);
			EXPECT_EQ(cpu.get_cycle(), cycle);
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
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

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

	step_n(cpu, 3, addr, cycle);

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
		0x26, 0xff,			// LD H, 0xff
		0x2e, 0xff,			// LD L, 0xff
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
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

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

	step_n(cpu, 3, addr, cycle);

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
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
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

	step_n(cpu, 2, addr, cycle);

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

TEST(instructions, op_srl_r8)
{
	// CB SRL r8
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x80,			// LD B, 0x80
		0x0e, 0x40,			// LD C, 0x40
		0x16, 0x20,			// LD D, 0x20
		0x1e, 0x10,			// LD E, 0x10
		0x26, 0x08,			// LD H, 0x08
		0x2e, 0x04,			// LD L, 0x04
		0x3e, 0x01,			// LD A, 0x01
		0xcb, 0x38,			// CB SRL B
		0xcb, 0x39,			// CB SRL C
		0xcb, 0x3a,			// CB SRL D
		0xcb, 0x3b,			// CB SRL E
		0xcb, 0x3c,			// CB SRL H
		0xcb, 0x3d,			// CB SRL L
		0xcb, 0x3f,			// CB SRL A
		}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x40, 0x00, lr35902::r8::B },
		result{ 0x20, 0x00, lr35902::r8::C },
		result{ 0x10, 0x00, lr35902::r8::D },
		result{ 0x08, 0x00, lr35902::r8::E },
		result{ 0x04, 0x00, lr35902::r8::H },
		result{ 0x02, 0x00, lr35902::r8::L },
		result{ 0x00, 0x90, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_srl_hl)
{
	// CB SRL (HL)
	// 2 16
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xab,			// LD A, 0xab
		0x77,				// LD (HL), A
		0xcb, 0x3e,			// CB SRL (HL)
		0x3e, 0x01,			// LD A, 0x01
		0x77,				// LD (HL), A
		0xcb, 0x3e,			// CB SRL (HL)
	}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x55);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x90);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}

TEST(instructions, op_sra_r8)
{
	// CB SRA r8
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x80,			// LD B, 0x80
		0x0e, 0x40,			// LD C, 0x40
		0x16, 0x20,			// LD D, 0x20
		0x1e, 0x10,			// LD E, 0x10
		0x26, 0x08,			// LD H, 0x08
		0x2e, 0x04,			// LD L, 0x04
		0x3e, 0x01,			// LD A, 0x01
		0xcb, 0x28,			// CB SRA B
		0xcb, 0x29,			// CB SRA C
		0xcb, 0x2a,			// CB SRA D
		0xcb, 0x2b,			// CB SRA E
		0xcb, 0x2c,			// CB SRA H
		0xcb, 0x2d,			// CB SRA L
		0xcb, 0x2f,			// CB SRA A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0xc0, 0x00, lr35902::r8::B },
		result{ 0x20, 0x00, lr35902::r8::C },
		result{ 0x10, 0x00, lr35902::r8::D },
		result{ 0x08, 0x00, lr35902::r8::E },
		result{ 0x04, 0x00, lr35902::r8::H },
		result{ 0x02, 0x00, lr35902::r8::L },
		result{ 0x00, 0x90, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_sra_hl)
{
	// CB SRA (HL)
	// 2 16
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xab,			// LD A, 0xab
		0x77,				// LD (HL), A
		0xcb, 0x2e,			// CB SRA (HL)
		0x3e, 0x01,			// LD A, 0x01
		0x77,				// LD (HL), A
		0xcb, 0x2e,			// CB SRA (HL)
	}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0xd5);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x90);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}


TEST(instructions, op_sla_r8)
{
	// CB SLA r8
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0xff,			// LD B, 0xff
		0x0e, 0x7f,			// LD C, 0x7f
		0x16, 0x80,			// LD D, 0x80
		0x1e, 0xff,			// LD E, 0xff
		0x26, 0x7f,			// LD H, 0x7f
		0x2e, 0x80,			// LD L, 0x80
		0x3e, 0xff,			// LD A, 0xff
		0xcb, 0x20,			// CB SLA B
		0xcb, 0x21,			// CB SLA C
		0xcb, 0x22,			// CB SLA D
		0xcb, 0x23,			// CB SLA E
		0xcb, 0x24,			// CB SLA H
		0xcb, 0x25,			// CB SLA L
		0xcb, 0x27,			// CB SLA A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0xfe, 0x10, lr35902::r8::B },
		result{ 0xfe, 0x00, lr35902::r8::C },
		result{ 0x00, 0x90, lr35902::r8::D },
		result{ 0xfe, 0x10, lr35902::r8::E },
		result{ 0xfe, 0x00, lr35902::r8::H },
		result{ 0x00, 0x90, lr35902::r8::L },
		result{ 0xfe, 0x10, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_sla_hl)
{
	// CB SLA (HL)
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xff,			// LD A, 0xff
		0x77,				// LD (HL), A
		0xcb, 0x26,			// CB SLA (HL)
		0x3e, 0x7f,			// LD A, 0x7f
		0x77,				// LD (HL), A
		0xcb, 0x26,			// CB SLA (HL)
		0x3e, 0x80,			// LD A, 0x80
		0x77,				// LD (HL), A
		0xcb, 0x26,			// CB SLA (HL)
	}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0xfe);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0xfe);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x90);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}

TEST(instructions, op_rlc_r8)
{
	// CB RLC r8
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0xaa,			// LD B, 0xaa
		0x0e, 0x00,			// LD C, 0x00
		0x16, 0x55,			// LD D, 0x55
		0x1e, 0xaa,			// LD E, 0xaa
		0x26, 0x00,			// LD H, 0x00
		0x2e, 0x55,			// LD L, 0x55
		0x3e, 0xaa,			// LD A, 0xaa
		0xcb, 0x00,			// CB RLC B
		0xcb, 0x01,			// CB RLC C
		0xcb, 0x02,			// CB RLC D
		0xcb, 0x03,			// CB RLC E
		0xcb, 0x04,			// CB RLC H
		0xcb, 0x05,			// CB RLC L
		0xcb, 0x07,			// CB RLC A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x55, 0x10, lr35902::r8::B },
		result{ 0x00, 0x80, lr35902::r8::C },
		result{ 0xaa, 0x00, lr35902::r8::D },
		result{ 0x55, 0x10, lr35902::r8::E },
		result{ 0x00, 0x80, lr35902::r8::H },
		result{ 0xaa, 0x00, lr35902::r8::L },
		result{ 0x55, 0x10, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_rlc_hl)
{
	// CB RLC (HL)
	// 2 16
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xaa,			// LD A, 0xaa
		0x77,				// LD (HL), A
		0xcb, 0x06,			// CB RLC (HL)
		0x3e, 0x00,			// LD A, 0x00
		0x77,				// LD (HL), A
		0xcb, 0x06,			// CB RLC (HL)
		0x3e, 0x55,			// LD A, 0x55
		0x77,				// LD (HL), A
		0xcb, 0x06,			// CB RLC (HL)
	}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x55);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0xaa);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}

TEST(instructions, op_rrc_r8)
{
	// CB RRC r8
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0xaa,			// LD B, 0xaa
		0x0e, 0x00,			// LD C, 0x00
		0x16, 0x01,			// LD D, 0x01
		0x1e, 0xaa,			// LD E, 0xaa
		0x26, 0x00,			// LD H, 0x00
		0x2e, 0x01,			// LD L, 0x01
		0x3e, 0xaa,			// LD A, 0xaa
		0xcb, 0x08,			// CB RRC B
		0xcb, 0x09,			// CB RRC C
		0xcb, 0x0a,			// CB RRC D
		0xcb, 0x0b,			// CB RRC E
		0xcb, 0x0c,			// CB RRC H
		0xcb, 0x0d,			// CB RRC L
		0xcb, 0x0f,			// CB RRC A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x55, 0x00, lr35902::r8::B },
		result{ 0x00, 0x80, lr35902::r8::C },
		result{ 0x80, 0x10, lr35902::r8::D },
		result{ 0x55, 0x00, lr35902::r8::E },
		result{ 0x00, 0x80, lr35902::r8::H },
		result{ 0x80, 0x10, lr35902::r8::L },
		result{ 0x55, 0x00, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_rrc_hl)
{
	// CB RRC (HL)
	// 2 16
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0xaa,			// LD A, 0xaa
		0x77,				// LD (HL), A
		0xcb, 0x0e,			// CB RRC (HL)
		0x3e, 0x00,			// LD A, 0x00
		0x77,				// LD (HL), A
		0xcb, 0x0e,			// CB RRC (HL)
		0x3e, 0x01,			// LD A, 0x01
		0x77,				// LD (HL), A
		0xcb, 0x0e,			// CB RRC (HL)
	}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x55);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x80);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x10);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}

TEST(instructions, op_rl_r8)
{
	// CB RL r8
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x80,			// LD B, 0x80
		0x0e, 0x00,			// LD C, 0x00
		0x16, 0x00,			// LD D, 0x00
		0x1e, 0x80,			// LD E, 0x80
		0x26, 0x00,			// LD H, 0x00
		0x2e, 0x00,			// LD L, 0x00
		0x3e, 0x80,			// LD A, 0x80
		0xcb, 0x10,			// CB RL B
		0xcb, 0x11,			// CB RL C
		0xcb, 0x12,			// CB RL D
		0xcb, 0x13,			// CB RL E
		0xcb, 0x14,			// CB RL H
		0xcb, 0x15,			// CB RL L
		0xcb, 0x17,			// CB RL A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x00, 0x90, lr35902::r8::B },
		result{ 0x01, 0x00, lr35902::r8::C },
		result{ 0x00, 0x80, lr35902::r8::D },
		result{ 0x00, 0x90, lr35902::r8::E },
		result{ 0x01, 0x00, lr35902::r8::H },
		result{ 0x00, 0x80, lr35902::r8::L },
		result{ 0x00, 0x90, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_rl_hl)
{
	// CB RL (HL)
	// 2 16
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x80,			// LD A, 0x80
		0x77,				// LD (HL), A
		0xcb, 0x16,			// CB RL (HL)
		0x3e, 0x00,			// LD A, 0x00
		0x77,				// LD (HL), A
		0xcb, 0x16,			// CB RL (HL)
		0x3e, 0x00,			// LD A, 0x00
		0x77,				// LD (HL), A
		0xcb, 0x16,			// CB RL (HL)
	}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x90);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x01);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}

TEST(instructions, op_rr_r8)
{
	// CB RR r8
	// 2 8
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x01,			// LD B, 0x01
		0x0e, 0x00,			// LD C, 0x00
		0x16, 0x00,			// LD D, 0x00
		0x1e, 0x01,			// LD E, 0x01
		0x26, 0x00,			// LD H, 0x00
		0x2e, 0x00,			// LD L, 0x00
		0x3e, 0x01,			// LD A, 0x01
		0xcb, 0x18,			// CB RR B
		0xcb, 0x19,			// CB RR C
		0xcb, 0x1a,			// CB RR D
		0xcb, 0x1b,			// CB RR E
		0xcb, 0x1c,			// CB RR H
		0xcb, 0x1d,			// CB RR L
		0xcb, 0x1f,			// CB RR A
	}));

	cpu.reset();

	step_n(cpu, 7, addr, cycle);

	std::vector<result> results =
	{
		result{ 0x00, 0x90, lr35902::r8::B },
		result{ 0x80, 0x00, lr35902::r8::C },
		result{ 0x00, 0x80, lr35902::r8::D },
		result{ 0x00, 0x90, lr35902::r8::E },
		result{ 0x80, 0x00, lr35902::r8::H },
		result{ 0x00, 0x80, lr35902::r8::L },
		result{ 0x00, 0x90, lr35902::r8::A },
	};

	for (auto const& res : results)
	{
		addr += 2;
		cycle += 8;
		cpu.step();
		EXPECT_EQ(cpu.get_register(res.reg), res.value);
		EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr);
		EXPECT_EQ(cpu.get_flags(), res.flags);
		EXPECT_EQ(cpu.get_cycle(), cycle);
	}
}

TEST(instructions, op_rr_hl)
{
	// CB RR (HL)
	// 2 16
	// Z 0 0 C

	mmu mmu;
	lr35902 cpu{ mmu };
	std::uint16_t addr = 0;
	std::uint64_t cycle = 0;

	mmu.set_cartridge(cartridge_buf({
		0x21, 0x00, 0xc0,	// LD HL, 0xc000
		0x3e, 0x01,			// LD A, 0x01
		0x77,				// LD (HL), A
		0xcb, 0x1e,			// CB RR (HL)
		0x3e, 0x00,			// LD A, 0x00
		0x77,				// LD (HL), A
		0xcb, 0x1e,			// CB RR (HL)
		0x3e, 0x00,			// LD A, 0x00
		0x77,				// LD (HL), A
		0xcb, 0x1e,			// CB RR (HL)
		}));

	cpu.reset();

	step_n(cpu, 3, addr, cycle);

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x90);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x80);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x00);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);

	step_n(cpu, 2, addr, cycle);
	addr = cpu.get_register(lr35902::r16::PC);
	cycle = cpu.get_cycle();

	cpu.step();
	EXPECT_EQ(mmu[0xc000], 0x00);
	EXPECT_EQ(cpu.get_register(lr35902::r16::PC), addr + 2);
	EXPECT_EQ(cpu.get_flags(), 0x80);
	EXPECT_EQ(cpu.get_cycle(), cycle + 16);
}
