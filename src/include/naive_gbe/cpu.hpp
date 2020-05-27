//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>

#include <naive_gbe/mmu.hpp>
#include <naive_gbe/disassembler.hpp>

namespace naive_gbe
{
	class lr35902
	{
	public:

		enum class flags : std::uint8_t
		{
			carry		= 1 << 4,
			half_carry	= 1 << 5,
			subtraction = 1 << 6,
			zero		= 1 << 7
		};

		enum class bits : std::uint8_t
		{
			b0			= 1 << 0,
			b1			= 1 << 1,
			b2			= 1 << 2,
			b3			= 1 << 3,
			b4			= 1 << 4,
			b5			= 1 << 5,
			b6			= 1 << 6,
			b7			= 1 << 7,
		};

		enum class r8 : std::uint8_t
		{
			A			= 0,
			F			= 1,
			B			= 2,
			C			= 3,
			D			= 4,
			E			= 5,
			H			= 6,
			L			= 7
		};

		enum class r16 : std::uint8_t
		{
			AF			= 0,
			BC			= 2,
			DE			= 4,
			HL			= 6,
			SP			= 8,
			PC			= 10
		};

		struct operation
		{
			using handler = std::function<void()>;

			std::uint8_t	size_	= 1;
			std::uint8_t	cycles_ = 4;
			handler			func_	= nullptr;
		};

		lr35902(mmu& mmu) :
			mmu_(mmu)
		{
			reset();
			set_operation_table();
			set_operation_table_ex();
		}

		void reset()
		{
			registers_.fill(0);
			cycle_ = 0;
		}

		void step()
		{
			step(ops_, false);
		}

		std::size_t get_cycle() const
		{
			return cycle_;
		}

		std::uint8_t get_flags()
		{
			return registers_[static_cast<std::size_t>(r8::F)];
		}

		bool get_flag(flags flag)
		{
			return registers_[static_cast<std::size_t>(r8::F)] & (std::uint8_t)flag;
		}

		constexpr std::uint8_t get_register(r8 index) const
		{
			return registers_[static_cast<std::size_t>(index)];
		}

		constexpr std::uint16_t get_register(r16 index) const
		{
			return registers_[static_cast<std::size_t>(index)] << 8 | registers_[static_cast<std::size_t>(index) + 1];
		}

	private:

		using operations	= std::vector<operation>;
		using registers		= std::array<std::uint8_t, 12>;

		void disassembly(std::uint8_t opcode, operation const& op, bool extended)
		{
			if (opcode == 0xcb)
				return;

			std::uint8_t params[4] = { 0, };
			std::uint16_t addr = get_register(r16::PC);

			std::copy(&mmu_[addr], &mmu_[addr + op.size_ - 1], &params[0]);
			auto decoded = disasm_.disassembly(opcode, extended, op.size_, params);

			std::cout << decoded << "\n";
		}

		void step(operations& ops, bool extended)
		{
			std::uint8_t opcode = fetch_u8();

			auto& op = ops[opcode];

			//disassembly(opcode, op, extended);

			op.func_();
			cycle_ += op.cycles_;
		}

		std::uint8_t fetch_u8()
		{
			std::uint16_t addr = get_register(r16::PC);

			set_register(r16::PC, addr + 1);

			return mmu_[addr];
		}

		std::uint16_t fetch_u16()
		{
			std::uint16_t addr = get_register(r16::PC);

			set_register(r16::PC, addr + 2);

			return reinterpret_cast<std::uint16_t&>(mmu_[addr]);
		}

		void set_flags(uint8_t flags)
		{
			registers_[static_cast<std::size_t>(r8::F)] = flags & 0xf0;
		}

		void set_register(r8 index, std::uint8_t value)
		{
			registers_[static_cast<std::size_t>(index)] = value;
		}

		void set_register(r16 index, std::uint16_t value)
		{
			registers_[static_cast<std::size_t>(index) + 1] = value & 0x00ff;
			registers_[static_cast<std::size_t>(index)] = value >> 8;
		}

		void test_bit_u8(bits bit, std::uint8_t value)
		{
			std::uint8_t flags = (std::uint8_t)flags::half_carry | (get_flags() & (std::uint8_t)flags::carry);

			if (value & (std::uint8_t)bit)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);
		}

		constexpr std::uint8_t swap_nibbles(std::uint8_t value)
		{
			return (value & 0x0f) << 4 | (value & 0xf0) >> 4;
		}

		auto get_ref(r8 reg)
		{
			return std::ref(registers_[static_cast<std::size_t>(reg)]);
		}

		// OPERATION TABLES
		// ~~~~~~~~~~~~~~~~
		void set_operation_table()
		{
			auto& ops = ops_;
			ops.assign(0x100, operation{ 1, 4, std::bind(&lr35902::op_undefined, this) });

			ops[0x00] = operation{ 1,  4, std::bind(&lr35902::op_nop, this) };
			ops[0x01] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::BC) };
			ops[0x02] = operation{ 1,  8, std::bind(&lr35902::op_ld_bc, this) };
			ops[0x03] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::BC) };
			ops[0x04] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::B)) };
			ops[0x05] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::B)) };
			ops[0x06] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::B)) };
			ops[0x0a] = operation{ 1,  8, std::bind(&lr35902::op_ld_a_bc, this) };
			ops[0x0b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::BC) };
			ops[0x0c] = operation{ 3, 12, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::C)) };
			ops[0x0d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::C)) };
			ops[0x0e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::C)) };

			ops[0x10] = operation{ 2,  4, std::bind(&lr35902::op_stop, this) };
			ops[0x11] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::DE) };
			ops[0x12] = operation{ 1,  8, std::bind(&lr35902::op_ld_de, this) };
			ops[0x13] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::DE) };
			ops[0x14] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::D)) };
			ops[0x15] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::D)) };
			ops[0x16] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::D)) };
			ops[0x1a] = operation{ 1,  8, std::bind(&lr35902::op_ld_a_de, this) };
			ops[0x1b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::DE) };
			ops[0x1c] = operation{ 3, 12, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::E)) };
			ops[0x1d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::E)) };
			ops[0x1e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::E)) };

			ops[0x21] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::HL) };
			ops[0x22] = operation{ 1,  8, std::bind(&lr35902::op_ldi_hl, this) };
			ops[0x23] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::HL) };
			ops[0x24] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::H)) };
			ops[0x25] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::H)) };
			ops[0x26] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::H)) };
			ops[0x2a] = operation{ 1,  8, std::bind(&lr35902::op_ldi_a, this) };
			ops[0x2b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::HL) };
			ops[0x2c] = operation{ 3, 12, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::L)) };
			ops[0x2d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::L)) };
			ops[0x2e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::L)) };

			ops[0x31] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::SP) };
			ops[0x32] = operation{ 1,  8, std::bind(&lr35902::op_ldd_hl, this) };
			ops[0x33] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::SP) };
			ops[0x34] = operation{ 1, 12, std::bind(&lr35902::op_inc_hl, this) };
			ops[0x35] = operation{ 1, 12, std::bind(&lr35902::op_dec_hl, this) };
			ops[0x36] = operation{ 2, 12, std::bind(&lr35902::op_ld_hl, this) };
			ops[0x3a] = operation{ 1,  8, std::bind(&lr35902::op_ldd_a, this) };
			ops[0x3b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::SP) };
			ops[0x3c] = operation{ 3, 12, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::H)) };
			ops[0x3d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::A)) };
			ops[0x3e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::A)) };

			ops[0xcb] = operation{ 0,  0, std::bind(&lr35902::op_cb, this) };

			ops[0xa8] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::B)) };
			ops[0xa9] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::C)) };
			ops[0xaa] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::D)) };
			ops[0xab] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::E)) };
			ops[0xac] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::H)) };
			ops[0xad] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::L)) };
			ops[0xae] = operation{ 1,  8, std::bind(&lr35902::op_xor_hl, this, get_ref(r8::A)) };
			ops[0xaf] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::A)) };

			ops[0x40] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::B)) };
			ops[0x41] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::C)) };
			ops[0x42] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::D)) };
			ops[0x43] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::E)) };
			ops[0x44] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::H)) };
			ops[0x45] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::L)) };
			ops[0x46] =	operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::B)) };
			ops[0x47] =	operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::A)) };
			ops[0x48] =	operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::B)) };
			ops[0x49] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::C)) };
			ops[0x4a] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::D)) };
			ops[0x4b] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::E)) };
			ops[0x4c] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::H)) };
			ops[0x4d] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::L)) };
			ops[0x4e] =	operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::C)) };
			ops[0x4f] =	operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::B)) };

			ops[0x50] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::D), get_ref(r8::B)) };
			ops[0x51] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::D), get_ref(r8::C)) };
			ops[0x52] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::D), get_ref(r8::D)) };
			ops[0x53] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::D), get_ref(r8::E)) };
			ops[0x54] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::D), get_ref(r8::H)) };
			ops[0x55] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::D), get_ref(r8::L)) };
			ops[0x56] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::D)) };
			ops[0x57] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::D), get_ref(r8::A)) };
			ops[0x58] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::B)) };
			ops[0x59] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::C)) };
			ops[0x5a] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::D)) };
			ops[0x5b] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::E)) };
			ops[0x5c] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::H)) };
			ops[0x5d] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::L)) };
			ops[0x5e] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::E)) };
			ops[0x5f] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::B)) };

			ops[0x60] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::H), get_ref(r8::B)) };
			ops[0x61] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::H), get_ref(r8::C)) };
			ops[0x62] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::H), get_ref(r8::D)) };
			ops[0x63] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::H), get_ref(r8::E)) };
			ops[0x64] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::H), get_ref(r8::H)) };
			ops[0x65] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::H), get_ref(r8::L)) };
			ops[0x66] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::H)) };
			ops[0x67] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::H), get_ref(r8::A)) };
			ops[0x68] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::B)) };
			ops[0x69] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::C)) };
			ops[0x6a] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::D)) };
			ops[0x6b] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::E)) };
			ops[0x6c] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::H)) };
			ops[0x6d] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::L)) };
			ops[0x6e] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::L)) };
			ops[0x6f] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::B)) };

			ops[0x70] = operation{ 1,  8, std::bind(&lr35902::op_ld_hl_r8, this, get_ref(r8::B)) };
			ops[0x71] = operation{ 1,  8, std::bind(&lr35902::op_ld_hl_r8, this, get_ref(r8::C)) };
			ops[0x72] = operation{ 1,  8, std::bind(&lr35902::op_ld_hl_r8, this, get_ref(r8::D)) };
			ops[0x73] = operation{ 1,  8, std::bind(&lr35902::op_ld_hl_r8, this, get_ref(r8::E)) };
			ops[0x74] = operation{ 1,  8, std::bind(&lr35902::op_ld_hl_r8, this, get_ref(r8::H)) };
			ops[0x75] = operation{ 1,  8, std::bind(&lr35902::op_ld_hl_r8, this, get_ref(r8::L)) };
			ops[0x76] = operation{ 1,  4, std::bind(&lr35902::op_halt, this) };
			ops[0x77] = operation{ 1,  8, std::bind(&lr35902::op_ld_hl_r8, this, get_ref(r8::A)) };
			ops[0x78] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::A), get_ref(r8::B)) };
			ops[0x79] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::A), get_ref(r8::C)) };
			ops[0x7a] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::A), get_ref(r8::D)) };
			ops[0x7b] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::A), get_ref(r8::E)) };
			ops[0x7c] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::A), get_ref(r8::H)) };
			ops[0x7d] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::A), get_ref(r8::L)) };
			ops[0x7e] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::A)) };
			ops[0x7f] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::A), get_ref(r8::B)) };
		}

		void set_operation_table_ex()
		{
			auto& ops = ops_ex_;
			ops.assign(0x100, operation{ 1, 4, std::bind(&lr35902::op_undefined, this) });

			ops[0x00] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::B)) };
			ops[0x01] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::C)) };
			ops[0x02] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::D)) };
			ops[0x03] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::E)) };
			ops[0x04] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::H)) };
			ops[0x05] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::L)) };
			ops[0x06] = operation{ 2, 16, std::bind(&lr35902::op_rlc_hl, this) };
			ops[0x07] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::A)) };
			ops[0x08] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::B)) };
			ops[0x09] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::C)) };
			ops[0x0a] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::D)) };
			ops[0x0b] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::E)) };
			ops[0x0c] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::H)) };
			ops[0x0d] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::L)) };
			ops[0x0e] = operation{ 2, 16, std::bind(&lr35902::op_rrc_hl, this) };
			ops[0x0f] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::A)) };

			ops[0x10] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::B)) };
			ops[0x11] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::C)) };
			ops[0x12] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::D)) };
			ops[0x13] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::E)) };
			ops[0x14] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::H)) };
			ops[0x15] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::L)) };
			ops[0x16] = operation{ 2, 16, std::bind(&lr35902::op_rl_hl, this) };
			ops[0x17] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::A)) };
			ops[0x18] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::B)) };
			ops[0x19] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::C)) };
			ops[0x1a] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::D)) };
			ops[0x1b] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::E)) };
			ops[0x1c] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::H)) };
			ops[0x1d] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::L)) };
			ops[0x1e] = operation{ 2, 16, std::bind(&lr35902::op_rr_hl, this) };
			ops[0x1f] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::A)) };

			ops[0x20] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::B)) };
			ops[0x21] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::C)) };
			ops[0x22] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::D)) };
			ops[0x23] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::E)) };
			ops[0x24] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::H)) };
			ops[0x25] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::L)) };
			ops[0x26] = operation{ 2, 16, std::bind(&lr35902::op_sla_hl, this) };
			ops[0x27] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::A)) };
			ops[0x28] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::B)) };
			ops[0x29] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::C)) };
			ops[0x2a] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::D)) };
			ops[0x2b] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::E)) };
			ops[0x2c] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::H)) };
			ops[0x2d] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::L)) };
			ops[0x2e] = operation{ 2, 16, std::bind(&lr35902::op_sra_hl, this) };
			ops[0x2f] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::A)) };

			ops[0x30] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::B)) };
			ops[0x31] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::C)) };
			ops[0x32] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::D)) };
			ops[0x33] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::E)) };
			ops[0x34] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::H)) };
			ops[0x35] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::L)) };
			ops[0x36] = operation{ 2, 16, std::bind(&lr35902::op_swap_hl, this) };
			ops[0x37] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::A)) };
			ops[0x38] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::B)) };
			ops[0x39] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::C)) };
			ops[0x3a] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::D)) };
			ops[0x3b] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::E)) };
			ops[0x3c] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::H)) };
			ops[0x3d] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::L)) };
			ops[0x3e] = operation{ 2, 16, std::bind(&lr35902::op_srl_hl, this) };
			ops[0x3f] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::A)) };

			ops[0x40] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::B)) };
			ops[0x41] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::C)) };
			ops[0x42] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::D)) };
			ops[0x43] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::E)) };
			ops[0x44] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::H)) };
			ops[0x45] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::L)) };
			ops[0x46] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b0) };
			ops[0x47] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::A)) };
			ops[0x48] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::B)) };
			ops[0x49] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::C)) };
			ops[0x4a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::D)) };
			ops[0x4b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::E)) };
			ops[0x4c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::H)) };
			ops[0x4d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::L)) };
			ops[0x4e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b1) };
			ops[0x4f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::A)) };

			ops[0x50] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::B)) };
			ops[0x51] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::C)) };
			ops[0x52] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::D)) };
			ops[0x53] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::E)) };
			ops[0x54] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::H)) };
			ops[0x55] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::L)) };
			ops[0x56] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b2) };
			ops[0x57] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::A)) };
			ops[0x58] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::B)) };
			ops[0x59] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::C)) };
			ops[0x5a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::D)) };
			ops[0x5b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::E)) };
			ops[0x5c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::H)) };
			ops[0x5d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::L)) };
			ops[0x5e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b3) };
			ops[0x5f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::A)) };

			ops[0x60] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::B)) };
			ops[0x61] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::C)) };
			ops[0x62] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::D)) };
			ops[0x63] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::E)) };
			ops[0x64] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::H)) };
			ops[0x65] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::L)) };
			ops[0x66] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b4) };
			ops[0x67] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::A)) };
			ops[0x68] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::B)) };
			ops[0x69] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::C)) };
			ops[0x6a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::D)) };
			ops[0x6b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::E)) };
			ops[0x6c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::H)) };
			ops[0x6d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::L)) };
			ops[0x6e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b5) };
			ops[0x6f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::A)) };

			ops[0x70] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::B)) };
			ops[0x71] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::C)) };
			ops[0x72] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::D)) };
			ops[0x73] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::E)) };
			ops[0x74] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::H)) };
			ops[0x75] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::L)) };
			ops[0x76] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b6) };
			ops[0x77] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::A)) };
			ops[0x78] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::B)) };
			ops[0x79] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::C)) };
			ops[0x7a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::D)) };
			ops[0x7b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::E)) };
			ops[0x7c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::H)) };
			ops[0x7d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::L)) };
			ops[0x7e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b7) };
			ops[0x7f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::A)) };

			ops[0x80] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b0, get_ref(r8::B)) };
			ops[0x81] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b0, get_ref(r8::C)) };
			ops[0x82] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b0, get_ref(r8::D)) };
			ops[0x83] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b0, get_ref(r8::E)) };
			ops[0x84] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b0, get_ref(r8::H)) };
			ops[0x85] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b0, get_ref(r8::L)) };
			ops[0x86] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b0) };
			ops[0x87] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b0, get_ref(r8::A)) };
			ops[0x88] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b1, get_ref(r8::B)) };
			ops[0x89] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b1, get_ref(r8::C)) };
			ops[0x8a] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b1, get_ref(r8::D)) };
			ops[0x8b] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b1, get_ref(r8::E)) };
			ops[0x8c] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b1, get_ref(r8::H)) };
			ops[0x8d] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b1, get_ref(r8::L)) };
			ops[0x8e] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b1) };
			ops[0x8f] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b1, get_ref(r8::A)) };

			ops[0x90] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b2, get_ref(r8::B)) };
			ops[0x91] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b2, get_ref(r8::C)) };
			ops[0x92] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b2, get_ref(r8::D)) };
			ops[0x93] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b2, get_ref(r8::E)) };
			ops[0x94] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b2, get_ref(r8::H)) };
			ops[0x95] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b2, get_ref(r8::L)) };
			ops[0x96] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b2) };
			ops[0x97] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b2, get_ref(r8::A)) };
			ops[0x98] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b3, get_ref(r8::B)) };
			ops[0x99] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b3, get_ref(r8::C)) };
			ops[0x9a] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b3, get_ref(r8::D)) };
			ops[0x9b] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b3, get_ref(r8::E)) };
			ops[0x9c] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b3, get_ref(r8::H)) };
			ops[0x9d] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b3, get_ref(r8::L)) };
			ops[0x9e] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b3) };
			ops[0x9f] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b3, get_ref(r8::A)) };

			ops[0xa0] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b4, get_ref(r8::B)) };
			ops[0xa1] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b4, get_ref(r8::C)) };
			ops[0xa2] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b4, get_ref(r8::D)) };
			ops[0xa3] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b4, get_ref(r8::E)) };
			ops[0xa4] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b4, get_ref(r8::H)) };
			ops[0xa5] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b4, get_ref(r8::L)) };
			ops[0xa6] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b4) };
			ops[0xa7] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b4, get_ref(r8::A)) };
			ops[0xa8] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b5, get_ref(r8::B)) };
			ops[0xa9] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b5, get_ref(r8::C)) };
			ops[0xaa] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b5, get_ref(r8::D)) };
			ops[0xab] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b5, get_ref(r8::E)) };
			ops[0xac] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b5, get_ref(r8::H)) };
			ops[0xad] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b5, get_ref(r8::L)) };
			ops[0xae] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b5) };
			ops[0xaf] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b5, get_ref(r8::A)) };

			ops[0xb0] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b6, get_ref(r8::B)) };
			ops[0xb1] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b6, get_ref(r8::C)) };
			ops[0xb2] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b6, get_ref(r8::D)) };
			ops[0xb3] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b6, get_ref(r8::E)) };
			ops[0xb4] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b6, get_ref(r8::H)) };
			ops[0xb5] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b6, get_ref(r8::L)) };
			ops[0xb6] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b6) };
			ops[0xb7] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b6, get_ref(r8::A)) };
			ops[0xb8] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b7, get_ref(r8::B)) };
			ops[0xb9] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b7, get_ref(r8::C)) };
			ops[0xba] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b7, get_ref(r8::D)) };
			ops[0xbb] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b7, get_ref(r8::E)) };
			ops[0xbc] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b7, get_ref(r8::H)) };
			ops[0xbd] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b7, get_ref(r8::L)) };
			ops[0xbe] = operation{ 2, 16, std::bind(&lr35902::op_res_hl, this, bits::b7) };
			ops[0xbf] = operation{ 2,  8, std::bind(&lr35902::op_res_r8, this, bits::b7, get_ref(r8::A)) };

			ops[0xc0] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b0, get_ref(r8::B)) };
			ops[0xc1] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b0, get_ref(r8::C)) };
			ops[0xc2] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b0, get_ref(r8::D)) };
			ops[0xc3] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b0, get_ref(r8::E)) };
			ops[0xc4] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b0, get_ref(r8::H)) };
			ops[0xc5] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b0, get_ref(r8::L)) };
			ops[0xc6] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b0) };
			ops[0xc7] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b0, get_ref(r8::A)) };
			ops[0xc8] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b1, get_ref(r8::B)) };
			ops[0xc9] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b1, get_ref(r8::C)) };
			ops[0xca] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b1, get_ref(r8::D)) };
			ops[0xcb] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b1, get_ref(r8::E)) };
			ops[0xcc] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b1, get_ref(r8::H)) };
			ops[0xcd] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b1, get_ref(r8::L)) };
			ops[0xce] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b1) };
			ops[0xcf] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b1, get_ref(r8::A)) };

			ops[0xd0] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b2, get_ref(r8::B)) };
			ops[0xd1] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b2, get_ref(r8::C)) };
			ops[0xd2] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b2, get_ref(r8::D)) };
			ops[0xd3] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b2, get_ref(r8::E)) };
			ops[0xd4] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b2, get_ref(r8::H)) };
			ops[0xd5] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b2, get_ref(r8::L)) };
			ops[0xd6] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b2) };
			ops[0xd7] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b2, get_ref(r8::A)) };
			ops[0xd8] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b3, get_ref(r8::B)) };
			ops[0xd9] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b3, get_ref(r8::C)) };
			ops[0xda] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b3, get_ref(r8::D)) };
			ops[0xdb] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b3, get_ref(r8::E)) };
			ops[0xdc] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b3, get_ref(r8::H)) };
			ops[0xdd] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b3, get_ref(r8::L)) };
			ops[0xde] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b3) };
			ops[0xdf] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b3, get_ref(r8::A)) };

			ops[0xe0] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b4, get_ref(r8::B)) };
			ops[0xe1] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b4, get_ref(r8::C)) };
			ops[0xe2] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b4, get_ref(r8::D)) };
			ops[0xe3] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b4, get_ref(r8::E)) };
			ops[0xe4] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b4, get_ref(r8::H)) };
			ops[0xe5] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b4, get_ref(r8::L)) };
			ops[0xe6] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b4) };
			ops[0xe7] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b4, get_ref(r8::A)) };
			ops[0xe8] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b5, get_ref(r8::B)) };
			ops[0xe9] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b5, get_ref(r8::C)) };
			ops[0xea] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b5, get_ref(r8::D)) };
			ops[0xeb] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b5, get_ref(r8::E)) };
			ops[0xec] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b5, get_ref(r8::H)) };
			ops[0xed] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b5, get_ref(r8::L)) };
			ops[0xee] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b5) };
			ops[0xef] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b5, get_ref(r8::A)) };

			ops[0xf0] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b6, get_ref(r8::B)) };
			ops[0xf1] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b6, get_ref(r8::C)) };
			ops[0xf2] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b6, get_ref(r8::D)) };
			ops[0xf3] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b6, get_ref(r8::E)) };
			ops[0xf4] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b6, get_ref(r8::H)) };
			ops[0xf5] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b6, get_ref(r8::L)) };
			ops[0xf6] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b6) };
			ops[0xf7] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b6, get_ref(r8::A)) };
			ops[0xf8] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b7, get_ref(r8::B)) };
			ops[0xf9] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b7, get_ref(r8::C)) };
			ops[0xfa] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b7, get_ref(r8::D)) };
			ops[0xfb] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b7, get_ref(r8::E)) };
			ops[0xfc] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b7, get_ref(r8::H)) };
			ops[0xfd] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b7, get_ref(r8::L)) };
			ops[0xfe] = operation{ 2, 16, std::bind(&lr35902::op_set_hl, this, bits::b7) };
			ops[0xff] = operation{ 2,  8, std::bind(&lr35902::op_set_r8, this, bits::b7, get_ref(r8::A)) };
		}

		// INSTRUCTIONS
		// ~~~~~~~~~~~~
		// opcode
		// size cycles
		// Z N H C
		// https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html

		// UNDEF
		// 1 4
		// - - - -
		void op_undefined()
		{
			std::cerr << "unexpected instruction!\n";
			std::abort();
		}

		// NOP
		// 1 4
		// - - - -
		void op_nop()
		{
		}

		// STOP
		// 2 4
		// - - - -
		void op_stop()
		{
			// TODO
		}

		// HALT
		// 1 4
		// - - - -
		void op_halt()
		{
			// TODO
		}

		// INC r8
		// 1 4
		// Z 0 H -
		void op_inc_r8(std::uint8_t& reg)
		{
			std::uint8_t value = reg + 1;
			std::uint8_t flags = get_flags() & (std::uint8_t)flags::carry;

			if (value & 0x08)
				flags |= (std::uint8_t)flags::half_carry;
			else if (!value)
				flags |= (std::uint8_t)flags::zero;

			reg = value;
			set_flags(flags);
		}

		// INC (HL)
		// 1 12
		// Z 0 H -
		void op_inc_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t flags = get_flags() & (std::uint8_t)flags::carry;

			value += 1;

			if (value & 0x08)
				flags |= (std::uint8_t)flags::half_carry;
			else if (!value)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);
		}

		// INC r16
		// 1 8
		// - - - -
		void op_inc_r16(r16 reg)
		{
			set_register(reg, get_register(reg) + 1);
		}

		// DEC r8
		// 1 4
		// Z 1 H -
		void op_dec_r8(std::uint8_t& reg)
		{
			std::uint8_t value = reg - 1;
			std::uint8_t flags = (get_flags() & (std::uint8_t)flags::carry) | (std::uint8_t)flags::subtraction;

			if (value & 0x08)
				flags |= (std::uint8_t)flags::half_carry;
			else if (!value)
				flags |= (std::uint8_t)flags::zero;

			reg = value;
			set_flags(flags);
		}

		// DEC (HL)
		// 1 12
		// Z 1 H -
		void op_dec_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t flags = (get_flags() & (std::uint8_t)flags::carry) | (std::uint8_t)flags::subtraction;

			value -= 1;

			if (value & 0x08)
				flags |= (std::uint8_t)flags::half_carry;
			else if (!value)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);
		}

		// DEC r16
		// 1 8
		// - - - -
		void op_dec_r16(r16 reg)
		{
			set_register(reg, get_register(reg) - 1);
		}

		// XOR r8
		// 1 4
		// Z 0 0 0
		void op_xor_r8(std::uint8_t& rhs, std::uint8_t& lhs)
		{
			rhs ^= lhs;
			set_flags((std::uint8_t)flags::zero);
		}

		// XOR (HL)
		// 1 4
		// Z 0 0 0
		void op_xor_hl(std::uint8_t& reg)
		{
			reg ^= mmu_[get_register(r16::HL)];
			set_flags((std::uint8_t)flags::zero);
		}

		// LD (BC), A
		// 1 8
		// - - - -
		void op_ld_bc()
		{
			mmu_[get_register(r16::BC)] = get_register(r8::A);
		}

		// LD (DE), A
		// 1 8
		// - - - -
		void op_ld_de()
		{
			mmu_[get_register(r16::DE)] = get_register(r8::A);
		}

		// LD r8, u8
		// 1 4
		// - - - -
		void op_ld_r8(std::uint8_t& reg)
		{
			reg = fetch_u8();
		}

		// LD (HL), u8
		// 2 12
		// - - - -
		void op_ld_hl()
		{
			get_hl_ref() = fetch_u8();
		}

		// LD A, (BC)
		// 1 8
		// - - - -
		void op_ld_a_bc()
		{
			set_register(r8::A, mmu_[get_register(r16::BC)]);
		}

		// LD A, (DE)
		// 1 8
		// - - - -
		void op_ld_a_de()
		{
			set_register(r8::A, mmu_[get_register(r16::DE)]);
		}

		// LD r8, r8
		// 1 4
		// - - - -
		void op_ld_r8_r8(std::uint8_t& rhs, std::uint8_t& lhs)
		{
			rhs = lhs;
		}

		// LD r8, (HL)
		// 1 4
		// - - - -
		void op_ld_r8_hl(std::uint8_t& reg)
		{
			reg = get_hl_ref();
		}

		// LD r16, d16
		// 3 12
		// - - - -
		void op_ld_r16(r16 reg)
		{
			set_register(reg, fetch_u16());
		}

		// LD (HL), r8
		// 1 8
		// - - - -
		void op_ld_hl_r8(std::uint8_t& reg)
		{
			get_hl_ref() = reg;
		}

		// LDI (HL), A
		// 1 8
		// - - - -
		void op_ldi_hl()
		{
			std::uint16_t addr = get_register(r16::HL);

			mmu_[addr] = get_register(r8::A);
			set_register(r16::HL, addr + 1);
		}

		// LDI A, (HL)
		// 1 8
		// - - - -
		void op_ldi_a()
		{
			std::uint16_t addr = get_register(r16::HL);

			set_register(r8::A, mmu_[addr]);
			set_register(r16::HL, addr + 1);
		}

		// LDD (HL), A
		// 1 8
		// - - - -
		void op_ldd_hl()
		{
			std::uint16_t addr = get_register(r16::HL);

			mmu_[addr] = get_register(r8::A);
			set_register(r16::HL, addr - 1);
		}

		// LDD A, (HL)
		// 1 8
		// - - - -
		void op_ldd_a()
		{
			std::uint16_t addr = get_register(r16::HL);

			set_register(r8::A, mmu_[addr]);
			set_register(r16::HL, addr - 1);
		}

		// CB
		// 1 4
		// - - - -
		void op_cb()
		{
			step(ops_ex_, true);
		}

		std::uint8_t& get_hl_ref()
		{
			std::uint16_t addr = get_register(r16::HL);
			return mmu_[addr];
		}

		std::uint8_t rlc_r8(std::uint8_t value)
		{
			std::uint8_t flags = 0;

			if (value & (std::uint8_t)bits::b7)
				flags |= (std::uint8_t)flags::carry;

			value = (value << 1) | (value >> 7);

			if (!value)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);

			return value;
		}

		std::uint8_t rl_r8(std::uint8_t value)
		{
			std::uint8_t carry = (std::uint8_t)get_flag(flags::carry);
			std::uint8_t res = rlc_r8(value);

			return res | carry;
		}

		std::uint8_t rrc_r8(std::uint8_t value)
		{
			std::uint8_t flags = 0;

			if (value & (std::uint8_t)bits::b0)
				flags |= (std::uint8_t)flags::carry;

			value = (value >> 1) | (value << 7);

			if (!value)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);

			return value;
		}

		std::uint8_t rr_r8(std::uint8_t value)
		{
			std::uint8_t carry = (std::uint8_t)get_flag(flags::carry);
			std::uint8_t res = rrc_r8(value);

			return res | carry;
		}

		std::uint8_t left_shift_u8(std::uint8_t value)
		{
			std::uint8_t flags = 0;

			if (value & (std::uint8_t)bits::b7)
				flags |= (std::uint8_t)flags::carry;

			value = value << 1;

			if (!value)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);

			return value;
		}

		std::uint8_t sra_r8(std::uint8_t value)
		{
			std::uint8_t flags = 0;

			if (value & (std::uint8_t)bits::b0)
				flags |= (std::uint8_t)flags::carry;

			value = value >> 1;

			if (!value)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);

			return value;
		}

		std::uint8_t right_shift_u8(std::uint8_t value)
		{
			std::uint8_t flags = 0;

			if (value & (std::uint8_t)bits::b0)
				flags |= (std::uint8_t)flags::carry;

			value >>= 1;

			if (!value)
				flags |= (std::uint8_t)flags::zero;

			set_flags(flags);

			return value;
		}

		// CB RLC r8
		// 2 8
		// Z 0 0 C
		void op_rlc_r8(std::uint8_t& reg)
		{
			reg = rlc_r8(reg);
		}

		// CB RLC (HL)
		// 2 16
		// Z 0 0 C
		void op_rlc_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t res = rlc_r8(value);

			value = res;
		}

		// CB RRC r8
		// 2 8
		// Z 0 0 C
		void op_rrc_r8(std::uint8_t& reg)
		{
			reg = rrc_r8(reg);
		}

		// CB RRC (HL)
		// 2 16
		// Z 0 0 C
		void op_rrc_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t res = rrc_r8(value);

			value = res;
		}

		// CB RL r8
		// 2 8
		// Z 0 0 C
		void op_rl_r8(std::uint8_t& reg)
		{
			reg = rl_r8(reg);
		}

		// CB RL (HL)
		// 2 16
		// Z 0 0 C
		void op_rl_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t res = rl_r8(value);

			value = res;
		}

		// CB RR r8
		// 2 8
		// Z 0 0 C
		void op_rr_r8(std::uint8_t& reg)
		{
			reg = rr_r8(reg);
		}

		// CB RR (HL)
		// 2 16
		// Z 0 0 C
		void op_rr_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t res = rr_r8(value);

			value = res;
		}

		// CB SLA r8
		// 2 8
		// Z 0 0 C
		void op_sla_r8(std::uint8_t& reg)
		{
			reg = left_shift_u8(reg);
		}

		// CB SLA (HL)
		// 2 8
		// Z 0 0 C
		void op_sla_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t res = left_shift_u8(value);

			value = res;
		}

		// CB SRA r8
		// 2 8
		// Z 0 0 C
		void op_sra_r8(std::uint8_t& reg)
		{
			std::uint8_t value = reg;
			reg = (value & (std::uint8_t)bits::b7) | right_shift_u8(reg);
		}

		// CB SRA (HL)
		// 2 16
		// Z 0 0 C
		void op_sra_hl()
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t res = right_shift_u8(value);

			value = (value & (std::uint8_t)bits::b7) | res;
		}

		// CB SWAP r8
		// 2 8
		// Z 0 0 0
		void op_swap_r8(std::uint8_t& reg)
		{
			reg = swap_nibbles(reg);
			set_flags(reg ? 0 : (std::uint8_t)flags::zero);
		}

		// CB SWAP (HL)
		// 2 16
		// Z 0 0 0
		void op_swap_hl()
		{
			std::uint16_t addr = get_register(r16::HL);
			std::uint8_t res = swap_nibbles(mmu_[addr]);

			mmu_[addr] = res;

			set_flags(res ? 0 : (std::uint8_t)flags::zero);
		}

		// CB SRL r8
		// 2 8
		// Z 0 0 C
		void op_srl_r8(std::uint8_t& reg)
		{
			reg = right_shift_u8(reg);
		}

		// CB SRL (HL)
		// 2 16
		// Z 0 0 C
		void op_srl_hl()
		{
			std::uint8_t& value = get_hl_ref();

			value = right_shift_u8(value);
		}

		// CB BIT n, r8
		// 2 8
		// Z 0 1 -
		void op_bit_r8(bits bit, std::uint8_t& reg)
		{
			test_bit_u8(bit, reg);
		}

		// CB BIT n, (HL)
		// 2 16
		// Z 0 1 -
		void op_bit_hl(bits bit)
		{
			test_bit_u8(bit, get_hl_ref());
		}

		// CB RES n, r8
		// 2 8
		// - - - -
		void op_res_r8(bits bit, std::uint8_t& reg)
		{
			reg &= ~(static_cast<std::uint8_t>(bit));
		}

		// CB RES n, (HL)
		// 2 16
		// - - - -
		void op_res_hl(bits bit)
		{
			std::uint8_t& value = get_hl_ref();

			value &= ~(static_cast<std::uint8_t>(bit));
		}

		// CB SET n, r8
		// 2 8
		// - - - -
		void op_set_r8(bits bit, std::uint8_t& reg)
		{
			reg |= static_cast<std::uint8_t>(bit);
		}

		// CB SET n, (HL)
		// 2 16
		// - - - -
		void op_set_hl(bits bit)
		{
			std::uint8_t& value = get_hl_ref();

			value |= static_cast<std::uint8_t>(bit);
		}

	private:

		registers		registers_;
		std::uint64_t	cycle_;
		mmu&			mmu_;
		operations		ops_;
		operations		ops_ex_;
		disassembler	disasm_;
	};

}
