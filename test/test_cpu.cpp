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
TEST(instructions, op_ld_r8)
{
	// LD r8, u8
	// 1 4
	// - - - -

	mmu mmu;
	lr35902 cpu{ mmu };

	mmu.set_cartridge(cartridge_buf({
		0x06, 0x12,		// LD B, 0x12
		0x0e, 0x23,		// LD C, 0x23
		0x16, 0x34,		// LD D, 0x34
		0x1e, 0x45,		// LD E, 0x45
		0x26, 0x56,		// LD L, 0x56
		0x2e, 0x67,		// LD H, 0x67
		0x3e, 0x78,		// LD A, 0x78
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
