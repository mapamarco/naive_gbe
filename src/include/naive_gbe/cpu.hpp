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

#include <naive_gbe/mmu.hpp>

namespace naive_gbe
{
	class lr35902
	{
	public:

		enum flags : std::uint8_t
		{
			carry		= 1 << 4,
			half_carry	= 1 << 5,
			subtraction = 1 << 6,
			zero		= 1 << 7
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
			set_operation_table_cb();
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

		constexpr std::size_t get_cycle() const
		{
			return cycle_;
		}

		constexpr std::uint8_t get_flags() const
		{
			return registers_[(std::uint8_t)r8::F] & 0xf0;
		}

		constexpr bool get_flag(flags flag) const
		{
			return registers_[(std::uint8_t)r8::F] & flag;
		}

		constexpr std::uint8_t get_register(r8 index) const
		{
			return registers_[(std::uint8_t)index];
		}

		constexpr std::uint16_t get_register(r16 index) const
		{
			return registers_[(std::uint8_t)index] << 8 | registers_[(std::uint8_t)index + 1];
		}

	private:

		enum bits : std::uint8_t
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

		using operations	= std::vector<operation>;
		using registers		= std::array<std::uint8_t, 12>;

		void step(operations& ops, bool extended)
		{
			auto& op = ops[fetch_u8()];

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
			registers_[(std::uint8_t)r8::F] = flags;
		}

		void set_register(r8 index, std::uint8_t value)
		{
			registers_[(std::uint8_t)index] = value;
		}

		void set_register(r16 index, std::uint16_t value)
		{
			registers_[(std::uint8_t)index + 1] = value & 0x00ff;
			registers_[(std::uint8_t)index] = value >> 8;
		}

		auto get_ref(r8 reg)
		{
			return std::ref(registers_[(std::uint8_t)reg]);
		}

		std::uint8_t& get_hl_ref()
		{
			return mmu_[get_register(r16::HL)];
		}

		constexpr std::uint8_t swap_nibbles(std::uint8_t value) const
		{
			return (value & 0x0f) << 4 | (value & 0xf0) >> 4;
		}

		void set_zero_flag(std::uint8_t value, std::uint8_t& flags)
		{
			if (!value)
				flags |= flags::zero;
		}

		void set_half_carry_and_zero_flags(std::uint8_t value, std::uint8_t& flags)
		{
			if (value & bits::b4)
				flags |= flags::half_carry;
			else if (!value)
				flags |= flags::zero;
		}

		void test_bit(std::uint8_t bit, std::uint8_t value, std::uint8_t& flags)
		{
			flags = flags::half_carry | (flags & flags::carry);

			if (value & bit)
				flags |= flags::zero;
		}

		void compare(std::uint8_t rhs, std::uint8_t lhs, std::uint8_t& flags)
		{
			flags = flags::subtraction;

			if (rhs - lhs)
			{
				if (lhs > rhs)
					flags |= flags::carry;

				if ((lhs & 0x0f) > (rhs & 0x0f))
					flags |= flags::half_carry;
			}
			else
			{
				flags |= flags::zero;
			}
		}

		void left_rotate(std::uint8_t& value, std::uint8_t& flags)
		{
			flags = 0;

			if (value & bits::b7)
				flags |= flags::carry;

			value = (value >> 1) | (value << 7);

			set_zero_flag(value, flags);
		}

		void left_rotate_carry(std::uint8_t& value, std::uint8_t& flags)
		{
			std::uint8_t bit0 = (flags & flags::carry) >> 4;

			flags = 0;

			if (value & bits::b7)
				flags = flags::carry;

			value = (value << 1) | bit0;

			set_zero_flag(value, flags);
		}

		void increment(std::uint8_t& value, std::uint8_t& flags)
		{
			flags &= flags::carry;

			++value;

			set_half_carry_and_zero_flags(value, flags);
		}

		void decrement(std::uint8_t& value, std::uint8_t& flags)
		{
			flags = (flags & flags::carry) | flags::subtraction;

			--value;

			set_half_carry_and_zero_flags(value, flags);
		}

		void add(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t carry, std::uint8_t& flags)
		{
			flags = 0;

			if ((lhs + rhs + carry) & 0x0100)
				flags |= flags::carry;

			if (((lhs & 0x0f) + (rhs & 0x0f) + carry) & 0x0010)
				flags |= flags::half_carry;

			lhs += rhs + carry;

			set_zero_flag(lhs, flags);
		}

		void sub(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t carry, std::uint8_t& flags)
		{
			flags = flags::subtraction;

			if (lhs > (rhs + carry))
				flags |= flags::carry;

			if ((lhs & 0x0f) > ((rhs & 0x0f) + carry))
				flags |= flags::half_carry;

			lhs -= rhs + carry;

			set_zero_flag(lhs, flags);
		}

		void right_rotate_u8(std::uint8_t& value, std::uint8_t& flags)
		{
			flags = 0;

			if (value & bits::b0)
				flags |= flags::carry;

			value = (value << 7) | (value >> 1);

			set_zero_flag(value, flags);
		}

		void right_rotate_carry_u8(std::uint8_t& value, std::uint8_t& flags)
		{
			std::uint8_t bit7 = (flags & flags::carry) << 3;

			flags = 0;

			if (value & bits::b0)
				flags = flags::carry;

			value = (value >> 1) | bit7;

			set_zero_flag(value, flags);
		}

		void left_shift_u8(std::uint8_t& value, std::uint8_t& flags)
		{
			flags = 0;

			if (value & bits::b7)
				flags |= flags::carry;

			value <<= 1;

			set_zero_flag(value, flags);
		}

		void right_shift_u8(std::uint8_t& value, std::uint8_t& flags)
		{
			flags = 0;

			if (value & bits::b0)
				flags |= flags::carry;

			value >>= 1;

			set_zero_flag(value, flags);
		}

		// OPERATION TABLES
		// ~~~~~~~~~~~~~~~~
		void set_operation_table()
		{
			auto& ops = ops_;
			ops.assign(0x100, operation{ 1, 4, std::bind(&lr35902::op_undefined, this) });

			ops[0x00] = operation{ 1,  4, std::bind(&lr35902::op_nop, this) };
			ops[0x01] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::BC) };
			ops[0x02] = operation{ 1,  8, std::bind(&lr35902::op_ld_bc_r8, this, get_ref(r8::A)) };
			ops[0x03] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::BC) };
			ops[0x04] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x05] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x06] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::B)) };
			ops[0x0a] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_bc, this, get_ref(r8::A)) };
			ops[0x0b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::BC) };
			ops[0x0c] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x0d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x0e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::C)) };

			ops[0x10] = operation{ 2,  4, std::bind(&lr35902::op_stop, this) };
			ops[0x11] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::DE) };
			ops[0x12] = operation{ 1,  8, std::bind(&lr35902::op_ld_de_r8, this, get_ref(r8::A)) };
			ops[0x13] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::DE) };
			ops[0x14] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x15] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x16] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::D)) };
			ops[0x1a] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_de, this, get_ref(r8::A)) };
			ops[0x1b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::DE) };
			ops[0x1c] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x1d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x1e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::E)) };

			ops[0x21] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::HL) };
			ops[0x22] = operation{ 1,  8, std::bind(&lr35902::op_ldi_hl, this, get_ref(r8::A)) };
			ops[0x23] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::HL) };
			ops[0x24] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x25] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x26] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::H)) };
			ops[0x2a] = operation{ 1,  8, std::bind(&lr35902::op_ldi_r8, this, get_ref(r8::A)) };
			ops[0x2b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::HL) };
			ops[0x2c] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x2d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x2e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::L)) };

			ops[0x31] = operation{ 3, 12, std::bind(&lr35902::op_ld_r16, this, r16::SP) };
			ops[0x32] = operation{ 1,  8, std::bind(&lr35902::op_ldd_hl, this) };
			ops[0x33] = operation{ 1,  8, std::bind(&lr35902::op_inc_r16, this, r16::SP) };
			ops[0x34] = operation{ 1, 12, std::bind(&lr35902::op_inc_hl, this, get_ref(r8::F)) };
			ops[0x35] = operation{ 1, 12, std::bind(&lr35902::op_dec_hl, this, get_ref(r8::F)) };
			ops[0x36] = operation{ 2, 12, std::bind(&lr35902::op_ld_hl, this) };
			ops[0x37] = operation{ 1,  4, std::bind(&lr35902::op_scf, this, get_ref(r8::F)) };
			ops[0x3a] = operation{ 1,  8, std::bind(&lr35902::op_ldd_a, this) };
			ops[0x3b] = operation{ 1,  8, std::bind(&lr35902::op_dec_r16, this, r16::SP) };
			ops[0x3c] = operation{ 1,  4, std::bind(&lr35902::op_inc_r8, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x3d] = operation{ 1,  4, std::bind(&lr35902::op_dec_r8, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x3e] = operation{ 2,  8, std::bind(&lr35902::op_ld_r8, this, get_ref(r8::A)) };
			ops[0x3f] = operation{ 1,  4, std::bind(&lr35902::op_ccf, this, get_ref(r8::F)) };

			ops[0x40] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::B)) };
			ops[0x41] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::C)) };
			ops[0x42] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::D)) };
			ops[0x43] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::E)) };
			ops[0x44] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::H)) };
			ops[0x45] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::L)) };
			ops[0x46] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::B)) };
			ops[0x47] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::B), get_ref(r8::A)) };
			ops[0x48] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::B)) };
			ops[0x49] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::C)) };
			ops[0x4a] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::D)) };
			ops[0x4b] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::E)) };
			ops[0x4c] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::H)) };
			ops[0x4d] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::L)) };
			ops[0x4e] = operation{ 1,  8, std::bind(&lr35902::op_ld_r8_hl, this, get_ref(r8::C)) };
			ops[0x4f] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::C), get_ref(r8::A)) };

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
			ops[0x5f] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::E), get_ref(r8::A)) };

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
			ops[0x6f] = operation{ 1,  4, std::bind(&lr35902::op_ld_r8_r8, this, get_ref(r8::L), get_ref(r8::A)) };

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

			ops[0x80] = operation{ 1,  4, std::bind(&lr35902::op_add_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0x81] = operation{ 1,  4, std::bind(&lr35902::op_add_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0x82] = operation{ 1,  4, std::bind(&lr35902::op_add_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0x83] = operation{ 1,  4, std::bind(&lr35902::op_add_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0x84] = operation{ 1,  4, std::bind(&lr35902::op_add_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0x85] = operation{ 1,  4, std::bind(&lr35902::op_add_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0x86] = operation{ 1,  8, std::bind(&lr35902::op_add_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x87] = operation{ 1,  4, std::bind(&lr35902::op_add_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };
			ops[0x88] = operation{ 1,  4, std::bind(&lr35902::op_adc_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0x89] = operation{ 1,  4, std::bind(&lr35902::op_adc_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0x8a] = operation{ 1,  4, std::bind(&lr35902::op_adc_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0x8b] = operation{ 1,  4, std::bind(&lr35902::op_adc_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0x8c] = operation{ 1,  4, std::bind(&lr35902::op_adc_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0x8d] = operation{ 1,  4, std::bind(&lr35902::op_adc_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0x8e] = operation{ 1,  8, std::bind(&lr35902::op_adc_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x8f] = operation{ 1,  4, std::bind(&lr35902::op_adc_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };

			ops[0x90] = operation{ 1,  4, std::bind(&lr35902::op_sub_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0x91] = operation{ 1,  4, std::bind(&lr35902::op_sub_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0x92] = operation{ 1,  4, std::bind(&lr35902::op_sub_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0x93] = operation{ 1,  4, std::bind(&lr35902::op_sub_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0x94] = operation{ 1,  4, std::bind(&lr35902::op_sub_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0x95] = operation{ 1,  4, std::bind(&lr35902::op_sub_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0x96] = operation{ 1,  8, std::bind(&lr35902::op_sub_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x97] = operation{ 1,  4, std::bind(&lr35902::op_sub_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };
			ops[0x98] = operation{ 1,  4, std::bind(&lr35902::op_sbc_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0x99] = operation{ 1,  4, std::bind(&lr35902::op_sbc_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0x9a] = operation{ 1,  4, std::bind(&lr35902::op_sbc_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0x9b] = operation{ 1,  4, std::bind(&lr35902::op_sbc_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0x9c] = operation{ 1,  4, std::bind(&lr35902::op_sbc_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0x9d] = operation{ 1,  4, std::bind(&lr35902::op_sbc_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0x9e] = operation{ 1,  8, std::bind(&lr35902::op_sbc_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x9f] = operation{ 1,  4, std::bind(&lr35902::op_sbc_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };

			ops[0xcb] = operation{ 0,  0, std::bind(&lr35902::op_cb, this) };

			ops[0xa0] = operation{ 1,  4, std::bind(&lr35902::op_and_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0xa1] = operation{ 1,  4, std::bind(&lr35902::op_and_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0xa2] = operation{ 1,  4, std::bind(&lr35902::op_and_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0xa3] = operation{ 1,  4, std::bind(&lr35902::op_and_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0xa4] = operation{ 1,  4, std::bind(&lr35902::op_and_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0xa5] = operation{ 1,  4, std::bind(&lr35902::op_and_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0xa6] = operation{ 1,  8, std::bind(&lr35902::op_and_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0xa7] = operation{ 1,  4, std::bind(&lr35902::op_and_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };
			ops[0xa8] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0xa9] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0xaa] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0xab] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0xac] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0xad] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0xae] = operation{ 1,  8, std::bind(&lr35902::op_xor_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0xaf] = operation{ 1,  4, std::bind(&lr35902::op_xor_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };

			ops[0xb0] = operation{ 1,  4, std::bind(&lr35902::op_or_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0xb1] = operation{ 1,  4, std::bind(&lr35902::op_or_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0xb2] = operation{ 1,  4, std::bind(&lr35902::op_or_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0xb3] = operation{ 1,  4, std::bind(&lr35902::op_or_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0xb4] = operation{ 1,  4, std::bind(&lr35902::op_or_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0xb5] = operation{ 1,  4, std::bind(&lr35902::op_or_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0xb6] = operation{ 1,  8, std::bind(&lr35902::op_or_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0xb7] = operation{ 1,  4, std::bind(&lr35902::op_or_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };
			ops[0xb8] = operation{ 1,  4, std::bind(&lr35902::op_cp_r8, this, get_ref(r8::A), get_ref(r8::B), get_ref(r8::F)) };
			ops[0xb9] = operation{ 1,  4, std::bind(&lr35902::op_cp_r8, this, get_ref(r8::A), get_ref(r8::C), get_ref(r8::F)) };
			ops[0xba] = operation{ 1,  4, std::bind(&lr35902::op_cp_r8, this, get_ref(r8::A), get_ref(r8::D), get_ref(r8::F)) };
			ops[0xbb] = operation{ 1,  4, std::bind(&lr35902::op_cp_r8, this, get_ref(r8::A), get_ref(r8::E), get_ref(r8::F)) };
			ops[0xbc] = operation{ 1,  4, std::bind(&lr35902::op_cp_r8, this, get_ref(r8::A), get_ref(r8::H), get_ref(r8::F)) };
			ops[0xbd] = operation{ 1,  4, std::bind(&lr35902::op_cp_r8, this, get_ref(r8::A), get_ref(r8::L), get_ref(r8::F)) };
			ops[0xbe] = operation{ 1,  8, std::bind(&lr35902::op_cp_hl, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0xbf] = operation{ 1,  4, std::bind(&lr35902::op_cp_r8, this, get_ref(r8::A), get_ref(r8::A), get_ref(r8::F)) };

			ops[0xd3] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xdb] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xdd] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };

			ops[0xe3] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xe4] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xeb] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xec] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xed] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };

			ops[0xf4] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xfc] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
			ops[0xfd] = operation{ 1, 4, std::bind(&lr35902::op_undefined, this) };
		}

		void set_operation_table_cb()
		{
			auto& ops = ops_cb_;
			ops.assign(0x100, operation{ 1, 4, std::bind(&lr35902::op_undefined, this) });

			ops[0x00] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x01] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x02] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x03] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x04] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x05] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x06] = operation{ 2, 16, std::bind(&lr35902::op_rlc_hl, this, get_ref(r8::F)) };
			ops[0x07] = operation{ 2,  8, std::bind(&lr35902::op_rlc_r8, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x08] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x09] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x0a] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x0b] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x0c] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x0d] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x0e] = operation{ 2, 16, std::bind(&lr35902::op_rrc_hl, this, get_ref(r8::F)) };
			ops[0x0f] = operation{ 2,  8, std::bind(&lr35902::op_rrc_r8, this, get_ref(r8::A), get_ref(r8::F)) };

			ops[0x10] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x11] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x12] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x13] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x14] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x15] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x16] = operation{ 2, 16, std::bind(&lr35902::op_rl_hl, this, get_ref(r8::F)) };
			ops[0x17] = operation{ 2,  8, std::bind(&lr35902::op_rl_r8, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x18] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x19] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x1a] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x1b] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x1c] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x1d] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x1e] = operation{ 2, 16, std::bind(&lr35902::op_rr_hl, this, get_ref(r8::F)) };
			ops[0x1f] = operation{ 2,  8, std::bind(&lr35902::op_rr_r8, this, get_ref(r8::A), get_ref(r8::F)) };

			ops[0x20] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x21] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x22] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x23] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x24] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x25] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x26] = operation{ 2, 16, std::bind(&lr35902::op_sla_hl, this, get_ref(r8::F)) };
			ops[0x27] = operation{ 2,  8, std::bind(&lr35902::op_sla_r8, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x28] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x29] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x2a] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x2b] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x2c] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x2d] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x2e] = operation{ 2, 16, std::bind(&lr35902::op_sra_hl, this, get_ref(r8::F)) };
			ops[0x2f] = operation{ 2,  8, std::bind(&lr35902::op_sra_r8, this, get_ref(r8::A), get_ref(r8::F)) };

			ops[0x30] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x31] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x32] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x33] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x34] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x35] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x36] = operation{ 2, 16, std::bind(&lr35902::op_swap_hl, this, get_ref(r8::F)) };
			ops[0x37] = operation{ 2,  8, std::bind(&lr35902::op_swap_r8, this, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x38] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x39] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x3a] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x3b] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x3c] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x3d] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x3e] = operation{ 2, 16, std::bind(&lr35902::op_srl_hl, this, get_ref(r8::F)) };
			ops[0x3f] = operation{ 2,  8, std::bind(&lr35902::op_srl_r8, this, get_ref(r8::A), get_ref(r8::F)) };

			ops[0x40] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x41] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x42] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x43] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x44] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x45] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x46] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b0, get_ref(r8::F)) };
			ops[0x47] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b0, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x48] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x49] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x4a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x4b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x4c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x4d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x4e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b1, get_ref(r8::F)) };
			ops[0x4f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b1, get_ref(r8::A), get_ref(r8::F)) };

			ops[0x50] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x51] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x52] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x53] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x54] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x55] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x56] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b2, get_ref(r8::F)) };
			ops[0x57] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b2, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x58] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x59] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x5a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x5b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x5c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x5d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x5e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b3, get_ref(r8::F)) };
			ops[0x5f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b3, get_ref(r8::A), get_ref(r8::F)) };

			ops[0x60] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x61] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x62] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x63] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x64] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x65] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x66] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b4, get_ref(r8::F)) };
			ops[0x67] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b4, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x68] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x69] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x6a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x6b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x6c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x6d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x6e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b5, get_ref(r8::F)) };
			ops[0x6f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b5, get_ref(r8::A), get_ref(r8::F)) };

			ops[0x70] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x71] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x72] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x73] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x74] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x75] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x76] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b6, get_ref(r8::F)) };
			ops[0x77] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b6, get_ref(r8::A), get_ref(r8::F)) };
			ops[0x78] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::B), get_ref(r8::F)) };
			ops[0x79] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::C), get_ref(r8::F)) };
			ops[0x7a] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::D), get_ref(r8::F)) };
			ops[0x7b] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::E), get_ref(r8::F)) };
			ops[0x7c] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::H), get_ref(r8::F)) };
			ops[0x7d] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::L), get_ref(r8::F)) };
			ops[0x7e] = operation{ 2, 16, std::bind(&lr35902::op_bit_hl, this, bits::b7, get_ref(r8::F)) };
			ops[0x7f] = operation{ 2,  8, std::bind(&lr35902::op_bit_r8, this, bits::b7, get_ref(r8::A), get_ref(r8::F)) };

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
			//std::abort();
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

		// SCF
		// 1 4
		// - 0 0 1
		void op_scf(std::uint8_t& flags)
		{
			flags = (flags & flags::zero) | flags::carry;
		}

		// CCF
		// 1 4
		// - 0 0 1
		void op_ccf(std::uint8_t& flags)
		{
			flags = (flags & flags::zero) | (~(flags & flags::carry) & flags::carry);
		}

		// ADD A, r8
		// 1 4
		// Z 0 H C
		void op_add_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags)
		{
			add(lhs, rhs, 0, flags);
		}

		// ADD A, (HL)
		// 2 8
		// Z 0 H C
		void op_add_hl(std::uint8_t& lhs, std::uint8_t& flags)
		{
			add(get_hl_ref(), lhs, 0, flags);
		}

		// ADC A, r8
		// 1 4
		// Z 0 H C
		void op_adc_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags)
		{
			std::uint8_t carry = flags & flags::carry ? 1 : 0;

			add(lhs, rhs, carry, flags);
		}

		// ADC A, (HL)
		// 2 8
		// Z 0 H C
		void op_adc_hl(std::uint8_t& reg, std::uint8_t& flags)
		{
			std::uint8_t carry = flags & flags::carry ? 1 : 0;

			add(get_hl_ref(), reg, carry, flags);
		}

		// SUB A, r8
		// 1 4
		// Z 0 H C
		void op_sub_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags)
		{
			sub(lhs, rhs, 0, flags);
		}

		// SUB A, (HL)
		// 2 8
		// Z 0 H C
		void op_sub_hl(std::uint8_t& reg, std::uint8_t& flags)
		{
			sub(get_hl_ref(), reg, 0, flags);
		}

		// SBC A, r8
		// 1 4
		// Z 1 H C
		void op_sbc_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags)
		{
			std::uint8_t carry = flags & flags::carry ? 1 : 0;

			sub(lhs, rhs, carry, flags);
		}

		// SBC A, (HL)
		// 1 8
		// Z 1 H C
		void op_sbc_hl(std::uint8_t& lhs, std::uint8_t& flags)
		{
			std::uint8_t carry = flags & flags::carry ? 1 : 0;

			sub(lhs, get_hl_ref(), carry, flags);
		}

		// INC r8
		// 1 4
		// Z 0 H -
		void op_inc_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			increment(reg, flags);
		}

		// INC (HL)
		// 1 12
		// Z 0 H -
		void op_inc_hl(std::uint8_t& flags)
		{
			increment(get_hl_ref(), flags);
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
		void op_dec_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			decrement(reg, flags);
		}

		// DEC (HL)
		// 1 12
		// Z 1 H -
		void op_dec_hl(std::uint8_t& flags)
		{
			decrement(get_hl_ref(), flags);
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
		void op_xor_r8(std::uint8_t& rhs, std::uint8_t lhs, std::uint8_t& flags)
		{
			rhs ^= lhs;
			flags = 0;
			set_zero_flag(rhs, flags);
		}

		// XOR (HL)
		// 1 8
		// Z 0 0 0
		void op_xor_hl(std::uint8_t& reg, std::uint8_t& flags)
		{
			reg ^= get_hl_ref();
			flags = 0;
			set_zero_flag(reg, flags);
		}

		// AND r8
		// 1 4
		// Z 0 1 0
		void op_and_r8(std::uint8_t& rhs, std::uint8_t lhs, std::uint8_t& flags)
		{
			rhs &= lhs;
			flags = flags::half_carry;
			set_zero_flag(rhs, flags);
		}

		// AND (HL)
		// 1 8
		// Z 0 1 0
		void op_and_hl(std::uint8_t& reg, std::uint8_t& flags)
		{
			reg &= get_hl_ref();
			flags = flags::half_carry;
			set_zero_flag(reg, flags);
		}

		// OR r8
		// 1 4
		// Z 0 0 0
		void op_or_r8(std::uint8_t& rhs, std::uint8_t lhs, std::uint8_t& flags)
		{
			rhs |= lhs;
			flags = 0;
			set_zero_flag(rhs, flags);
		}

		// OR (HL)
		// 1 8
		// Z 0 0 0
		void op_or_hl(std::uint8_t& reg, std::uint8_t& flags)
		{
			reg |= get_hl_ref();
			flags = 0;
			set_zero_flag(reg, flags);
		}

		// CP r8
		// 1 4
		// Z 1 H C
		void op_cp_r8(std::uint8_t& rhs, std::uint8_t lhs, std::uint8_t& flags)
		{
			compare(rhs, lhs, flags);
		}

		// CP (HL)
		// 1 8
		// Z 1 H C
		void op_cp_hl(std::uint8_t& reg, std::uint8_t& flags)
		{
			compare(reg, get_hl_ref(), flags);
		}

		// LD (BC), A
		// 1 8
		// - - - -
		void op_ld_bc_r8(std::uint8_t& reg)
		{
			mmu_[get_register(r16::BC)] = reg;
		}

		// LD (DE), A
		// 1 8
		// - - - -
		void op_ld_de_r8(std::uint8_t& reg)
		{
			mmu_[get_register(r16::DE)] = reg;
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
		void op_ld_r8_bc(std::uint8_t& reg)
		{
			reg = mmu_[get_register(r16::BC)];
			set_register(r8::A, mmu_[get_register(r16::BC)]);
		}

		// LD A, (DE)
		// 1 8
		// - - - -
		void op_ld_r8_de(std::uint8_t& reg)
		{
			reg = mmu_[get_register(r16::DE)];
		}

		// LD r8, r8
		// 1 4
		// - - - -
		void op_ld_r8_r8(std::uint8_t& rhs, std::uint8_t& lhs)
		{
			rhs = lhs;
		}

		// LD r8, (HL)
		// 1 8
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
		void op_ldi_hl(std::uint8_t& reg)
		{
			std::uint16_t addr = get_register(r16::HL);

			mmu_[addr] = reg;
			set_register(r16::HL, addr + 1);
		}

		// LDI A, (HL)
		// 1 8
		// - - - -
		void op_ldi_r8(std::uint8_t& reg)
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
		// 0 0
		// - - - -
		void op_cb()
		{
			step(ops_cb_, true);
		}

		// CB RLC r8
		// 2 8
		// Z 0 0 C
		void op_rlc_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			left_rotate(reg, flags);
		}

		// CB RLC (HL)
		// 2 16
		// Z 0 0 C
		void op_rlc_hl(std::uint8_t& flags)
		{
			left_rotate(get_hl_ref(), flags);
		}

		// CB RRC r8
		// 2 8
		// Z 0 0 C
		void op_rrc_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			right_rotate_u8(reg, flags);
		}

		// CB RRC (HL)
		// 2 16
		// Z 0 0 C
		void op_rrc_hl(std::uint8_t& flags)
		{
			right_rotate_u8(get_hl_ref(), flags);
		}

		// CB RL r8
		// 2 8
		// Z 0 0 C
		void op_rl_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			left_rotate_carry(reg, flags);
		}

		// CB RL (HL)
		// 2 16
		// Z 0 0 C
		void op_rl_hl(std::uint8_t& flags)
		{
			left_rotate_carry(get_hl_ref(), flags);
		}

		// CB RR r8
		// 2 8
		// Z 0 0 C
		void op_rr_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			right_rotate_carry_u8(reg, flags);
		}

		// CB RR (HL)
		// 2 16
		// Z 0 0 C
		void op_rr_hl(std::uint8_t& flags)
		{
			right_rotate_carry_u8(get_hl_ref(), flags);
		}

		// CB SLA r8
		// 2 8
		// Z 0 0 C
		void op_sla_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			left_shift_u8(reg, flags);
		}

		// CB SLA (HL)
		// 2 8
		// Z 0 0 C
		void op_sla_hl(std::uint8_t& flags)
		{
			left_shift_u8(get_hl_ref(), flags);
		}

		// CB SRA r8
		// 2 8
		// Z 0 0 C
		void op_sra_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			std::uint8_t bit7 = reg & bits::b7;

			right_shift_u8(reg, flags);
			reg |= bit7;
		}

		// CB SRA (HL)
		// 2 16
		// Z 0 0 C
		void op_sra_hl(std::uint8_t& flags)
		{
			std::uint8_t& value = get_hl_ref();
			std::uint8_t bit7 = value & bits::b7;

			right_shift_u8(value, flags);
			value |= bit7;
		}

		// CB SWAP r8
		// 2 8
		// Z 0 0 0
		void op_swap_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			reg = swap_nibbles(reg);
			flags = reg ? 0 : flags::zero;
		}

		// CB SWAP (HL)
		// 2 16
		// Z 0 0 0
		void op_swap_hl(std::uint8_t& flags)
		{
			std::uint16_t addr = get_register(r16::HL);

			mmu_[addr] = swap_nibbles(mmu_[addr]);
			flags = mmu_[addr] ? 0 : flags::zero;
		}

		// CB SRL r8
		// 2 8
		// Z 0 0 C
		void op_srl_r8(std::uint8_t& reg, std::uint8_t& flags)
		{
			right_shift_u8(reg, flags);
		}

		// CB SRL (HL)
		// 2 16
		// Z 0 0 C
		void op_srl_hl(std::uint8_t& flags)
		{
			right_shift_u8(get_hl_ref(), flags);
		}

		// CB BIT n, r8
		// 2 8
		// Z 0 1 -
		void op_bit_r8(std::uint8_t bit, std::uint8_t& reg, std::uint8_t& flags)
		{
			test_bit(bit, reg, flags);
		}

		// CB BIT n, (HL)
		// 2 16
		// Z 0 1 -
		void op_bit_hl(std::uint8_t bit, std::uint8_t& flags)
		{
			test_bit(bit, get_hl_ref(), flags);
		}

		// CB RES n, r8
		// 2 8
		// - - - -
		void op_res_r8(std::uint8_t bit, std::uint8_t& reg)
		{
			reg &= ~bit;
		}

		// CB RES n, (HL)
		// 2 16
		// - - - -
		void op_res_hl(std::uint8_t bit)
		{
			get_hl_ref() &= ~bit;
		}

		// CB SET n, r8
		// 2 8
		// - - - -
		void op_set_r8(std::uint8_t bit, std::uint8_t& reg)
		{
			reg |= bit;
		}

		// CB SET n, (HL)
		// 2 16
		// - - - -
		void op_set_hl(std::uint8_t bit)
		{
			get_hl_ref() |= bit;
		}

	private:

		registers		registers_;
		std::uint64_t	cycle_;
		mmu&			mmu_;
		operations		ops_;
		operations		ops_cb_;
	};

}
