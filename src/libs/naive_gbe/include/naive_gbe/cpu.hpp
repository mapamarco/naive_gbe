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
#include <cassert>
#include <functional>

#include <naive_gbe/mmu.hpp>
#include <naive_gbe/ppu.hpp>

namespace naive_gbe
{
	class lr35902
	{
	public:

		enum frequencies : std::size_t
		{
			NOMINAL		= 4194304
		};

		enum class state : std::uint8_t
		{
			STOPPED,
			READY,
			SUSPENDED
		};

		enum flags : std::uint8_t
		{
			CARRY		= 1 << 4,
			HALF_CARRY	= 1 << 5,
			SUBTRACTION = 1 << 6,
			ZERO		= 1 << 7
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

		lr35902(mmu& mmu, ppu& ppu);

		state get_state() const;

		void reset();

		void step();

		std::size_t get_cycle() const;

		std::uint8_t get_ime() const;

		std::uint8_t get_flags() const;

		bool get_flag(flags flag) const;

		std::uint8_t get_register(r8 index) const;

		std::uint16_t get_register(r16 index) const;

	private:

		struct daa
		{
			std::uint8_t	value_	= 0;
			bool			carry_	= false;
		};

		using registers		= std::array<std::uint8_t, 12>;
		using daas			= std::vector<daa>;
		using operations	= std::vector<operation>;

		void step(operations& ops, bool extended);

		std::uint8_t fetch_u8();

		std::int8_t fetch_i8();

		std::uint16_t fetch_u16();

		void set_flags(uint8_t flags);

		void set_register(r8 index, std::uint8_t value);

		void set_register(r16 index, std::uint16_t value);

		auto get_ref(r8 reg);

		address& get_hl_ref();

		std::uint8_t swap_nibbles(std::uint8_t value) const;

		void set_zero_flag(std::uint8_t value, std::uint8_t& flags);

		void set_half_carry_and_zero_flags(std::uint8_t value, std::uint8_t& flags);

		void call_addr(std::uint16_t addr);

		void logical_and(std::uint8_t& rhs, std::uint8_t lhs, std::uint8_t& flags);

		void logical_or(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t& flags);

		void logical_xor(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t& flags);

		void test_bit(std::uint8_t bit, std::uint8_t value, std::uint8_t& flags);

		void set_carry_flags_sub(std::uint8_t lhs, std::uint8_t rhs, std::uint8_t carry, std::uint8_t& flags);

		void set_carry_flags_add(std::uint16_t lhs, std::uint16_t rhs, std::uint8_t& flags);

		void compare(std::uint8_t lhs, std::uint8_t rhs, std::uint8_t& flags);

		void left_rotate(address& addr, std::uint8_t& flags);

		void left_rotate(std::uint8_t& value, std::uint8_t& flags);

		void left_rotate_carry(address& addr, std::uint8_t& flags);

		void left_rotate_carry(std::uint8_t& value, std::uint8_t& flags);

		void increment(address& addr, std::uint8_t& flags);

		void increment(std::uint8_t& value, std::uint8_t& flags);

		void decrement(address& addr, std::uint8_t& flags);

		void decrement(std::uint8_t& value, std::uint8_t& flags);

		void add(address& lhs, std::uint8_t rhs, std::uint8_t carry, std::uint8_t& flags);

		void add(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t carry, std::uint8_t& flags);

		void sub(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t carry, std::uint8_t& flags);

		void right_rotate(address& addr, std::uint8_t& flags);

		void right_rotate(std::uint8_t& value, std::uint8_t& flags);

		void right_rotate_carry(address& addr, std::uint8_t& flags);

		void right_rotate_carry(std::uint8_t& value, std::uint8_t& flags);

		void left_shift_u8(address& addr, std::uint8_t& flags);

		void left_shift_u8(std::uint8_t& value, std::uint8_t& flags);

		void right_shift_u8(address& value, std::uint8_t& flags);

		void right_shift_u8(std::uint8_t& value, std::uint8_t& flags);

		void set_daa_table();

		void set_operation_table();

		void set_operation_table_cb();

		void op_undefined();

		void op_nop();

		void op_stop();

		void op_halt();

		void op_di();

		void op_ei();

		void op_daa(std::uint8_t& lhs, std::uint8_t& flags);

		void op_rla(std::uint8_t& lhs, std::uint8_t& flags);

		void op_rlca(std::uint8_t& lhs, std::uint8_t& flags);

		void op_rra(std::uint8_t& lhs, std::uint8_t& flags);

		void op_rrca(std::uint8_t& lhs, std::uint8_t& flags);

		void op_pop(std::uint8_t& high, std::uint8_t& low);

		void op_push(std::uint8_t high, std::uint8_t low);

		void op_rst(std::uint16_t addr);

		void op_ret();

		void op_reti();

		void op_ret_cond(std::uint8_t flags, std::uint8_t bit, bool state);

		void op_call();

		void op_call_cond(std::uint8_t flags, std::uint8_t bit, bool state);

		void op_jp();

		void op_jp_hl();

		void op_jp_cond(std::uint8_t flags, std::uint8_t bit, bool state);

		void op_jr();

		void op_jr_cond(std::uint8_t flags, std::uint8_t bit, bool state);

		void op_cpl(std::uint8_t& reg, std::uint8_t& flags);

		void op_scf(std::uint8_t& flags);

		void op_ccf(std::uint8_t& flags);

		void op_add_sp_u8(std::uint8_t& flags);

		void op_add_d8(std::uint8_t& reg, std::uint8_t& flags);

		void op_add_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags);

		void op_add_r8_hl(std::uint8_t& lhs, std::uint8_t& flags);

		void op_add_hl_r16(r16 reg, std::uint8_t& flags);

		void op_adc_d8(std::uint8_t& lhs, std::uint8_t& flags);

		void op_adc_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags);

		void op_adc_hl(std::uint8_t& reg, std::uint8_t& flags);

		void op_sub_d8(std::uint8_t& reg, std::uint8_t& flags);

		void op_sub_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags);

		void op_sub_hl(std::uint8_t& reg, std::uint8_t& flags);

		void op_sbc_d8(std::uint8_t& lhs, std::uint8_t& flags);

		void op_sbc_r8(std::uint8_t& lhs, std::uint8_t& rhs, std::uint8_t& flags);

		void op_sbc_hl(std::uint8_t& lhs, std::uint8_t& flags);

		void op_inc_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_inc_hl(std::uint8_t& flags);

		void op_inc_r16(r16 reg);

		void op_dec_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_dec_hl(std::uint8_t& flags);

		void op_dec_r16(r16 reg);

		void op_xor_d8(std::uint8_t& lhs, std::uint8_t& flags);

		void op_xor_r8(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t& flags);

		void op_xor_hl(std::uint8_t& reg, std::uint8_t& flags);

		void op_and_d8(std::uint8_t& rhs, std::uint8_t& flags);

		void op_and_r8(std::uint8_t& rhs, std::uint8_t lhs, std::uint8_t& flags);

		void op_and_hl(std::uint8_t& reg, std::uint8_t& flags);

		void op_or_d8(std::uint8_t& lhs, std::uint8_t& flags);

		void op_or_r8(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t& flags);

		void op_or_hl(std::uint8_t& reg, std::uint8_t& flags);

		void op_cp_d8(std::uint8_t& lhs, std::uint8_t& flags);

		void op_cp_r8(std::uint8_t& lhs, std::uint8_t rhs, std::uint8_t& flags);

		void op_cp_hl(std::uint8_t& reg, std::uint8_t& flags);

		void op_ld_bc_r8(std::uint8_t& reg);

		void op_ld_de_r8(std::uint8_t& reg);

		void op_ld_r8(std::uint8_t& reg);

		void op_ld_hl();

		void op_ld_r8_bc(std::uint8_t& reg);

		void op_ld_c_r8(std::uint8_t lhs, std::uint8_t rhs);

		void op_ld_r8_c(std::uint8_t& lhs, std::uint8_t rhs);

		void op_ldh_a8_r8(std::uint8_t& rhs);

		void op_ldh_r8_a8(std::uint8_t rhs);

		void op_ld_r8_de(std::uint8_t& reg);

		void op_ld_r8_r8(std::uint8_t& rhs, std::uint8_t& lhs);

		void op_ld_r8_hl(std::uint8_t& reg);

		void op_ld_a16_r8(std::uint8_t& reg);

		void op_ld_r8_a16(std::uint8_t& reg);

		void op_ld_a16_sp();

		void op_ld_sp_hl();

		void op_ld_r16(r16 reg);

		void op_ldhl_sp();

		void op_ld_hl_r8(std::uint8_t& reg);

		void op_ldi_hl(std::uint8_t& reg);

		void op_ldi_r8(std::uint8_t& reg);

		void op_ldd_hl();

		void op_ldd_a();

		void op_cb();

		void op_rlc_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_rlc_hl(std::uint8_t& flags);

		void op_rrc_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_rrc_hl(std::uint8_t& flags);

		void op_rl_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_rl_hl(std::uint8_t& flags);

		void op_rr_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_rr_hl(std::uint8_t& flags);

		void op_sla_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_sla_hl(std::uint8_t& flags);

		void op_sra_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_sra_hl(std::uint8_t& flags);

		void op_swap_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_swap_hl(std::uint8_t& flags);

		void op_srl_r8(std::uint8_t& reg, std::uint8_t& flags);

		void op_srl_hl(std::uint8_t& flags);

		void op_bit_r8(std::uint8_t bit, std::uint8_t& reg, std::uint8_t& flags);

		void op_bit_hl(std::uint8_t bit, std::uint8_t& flags);

		void op_res_r8(std::uint8_t bit, std::uint8_t& reg);

		void op_res_hl(std::uint8_t bit);

		void op_set_r8(std::uint8_t bit, std::uint8_t& reg);

		void op_set_hl(std::uint8_t bit);

	private:

		registers		registers_;
		daas			daas_;
		std::uint8_t	ime_;
		std::uint64_t	cycle_;
		mmu&			mmu_;
		ppu&			ppu_;
		operations		ops_;
		operations		ops_cb_;
		state			state_		= state::STOPPED;
	};

}
