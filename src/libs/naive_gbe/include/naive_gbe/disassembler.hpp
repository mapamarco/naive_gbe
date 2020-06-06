//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <iomanip>

#include <naive_gbe/mmu.hpp>

namespace naive_gbe
{
	class disassembler
	{
	public:

		disassembler(mmu& mmu) :
			mmu_(mmu)
		{
			set_operations();
			set_operations_cb();
		}

		void print_u16(std::ostream& out, std::uint16_t value)
		{
			out << std::setw(4) << std::setfill('0')
				<< std::hex << static_cast<int>(value) << ' ';
		}

		void print_u8(std::ostream& out, std::uint8_t value)
		{
			out << std::setw(2) << std::setfill('0')
				<< std::hex << static_cast<int>(value) << ' ';
		}

		std::string decode(std::uint16_t addr)
		{
			std::ostringstream out;
			std::uint8_t opcode = mmu_[addr];
			operation op = ops_[opcode];

			print_u16(out, addr);
			out << ' ';

			if (opcode == 0xcb)
			{
				opcode = mmu_[addr + 1];
				op = ops_cb_[opcode];
			}

			for (std::uint8_t i = 0; i < op.size_; ++i, ++addr)
				print_u8(out, mmu_[addr]);

			int padding = 4 + (3 - op.size_) * 3;
			out << std::setw(padding) << std::setfill(' ') << ' ';

			auto it = std::begin(op.tokens_);
			out << *it << std::setw(6 - it->size()) << std::setfill(' ') << ' ';

			while (++it != std::end(op.tokens_))
				out << *it << (it + 1 != std::end(op.tokens_) ? ", " : " ");

			return out.str();
		}

	private:

		struct operation
		{
			using token_list = std::vector<std::string>;

			std::uint8_t	size_ = 1;
			std::uint8_t	cycles_ = 4;
			token_list		tokens_ = {};
		};

		using operations = std::unordered_map<std::uint8_t, operation>;

		void set_operations()
		{
			auto& ops = ops_;
			ops.reserve(0x100);

			ops[0x00] = operation{ 1,  4, { "nop" } };
			ops[0x01] = operation{ 3, 12, { "ld", "bc", "d16" } };
			ops[0x02] = operation{ 1,  8, { "ld", "(bc)", "a" } };
			ops[0x03] = operation{ 1,  8, { "inc", "bc" } };
			ops[0x04] = operation{ 1,  4, { "inc", "b" } };
			ops[0x05] = operation{ 1,  4, { "dec", "b" } };
			ops[0x06] = operation{ 2,  8, { "ld", "b", "d8" } };
			ops[0x07] = operation{ 1,  4, { "rlca" } };
			ops[0x08] = operation{ 3, 20, { "ld", "(a16)", "sp" } };
			ops[0x09] = operation{ 1,  8, { "add", "hl", "bc" } };
			ops[0x0a] = operation{ 1,  8, { "ld", "a", "(bc)" } };
			ops[0x0b] = operation{ 1,  8, { "dec", "bc" } };
			ops[0x0c] = operation{ 1,  4, { "inc", "c" } };
			ops[0x0d] = operation{ 1,  4, { "dec", "c" } };
			ops[0x0e] = operation{ 2,  8, { "ld", "c", "d8" } };
			ops[0x0f] = operation{ 1,  4, { "rrca" } };

			ops[0x10] = operation{ 2,  4, { "stop" } };
			ops[0x11] = operation{ 3, 12, { "ld", "de", "d16" } };
			ops[0x12] = operation{ 1,  8, { "ld", "(de)", "a" } };
			ops[0x13] = operation{ 1,  8, { "inc", "de" } };
			ops[0x14] = operation{ 1,  4, { "inc", "d" } };
			ops[0x15] = operation{ 1,  4, { "dec", "d" } };
			ops[0x16] = operation{ 2,  8, { "ld", "d", "d8" } };
			ops[0x17] = operation{ 1,  4, { "rla" } };
			ops[0x18] = operation{ 2,  8, { "jr", "r8" } };
			ops[0x19] = operation{ 1,  8, { "add", "hl", "de" } };
			ops[0x1a] = operation{ 1,  8, { "ld", "a", "(de)" } };
			ops[0x1b] = operation{ 1,  8, { "dec", "de" } };
			ops[0x1c] = operation{ 1,  4, { "inc", "e" } };
			ops[0x1d] = operation{ 1,  4, { "dec", "e" } };
			ops[0x1e] = operation{ 2,  8, { "ld", "e", "d8" } };
			ops[0x1f] = operation{ 1,  4, { "rra" } };

			ops[0x20] = operation{ 2,  8, { "jr", "nz", "r8" } };
			ops[0x21] = operation{ 3, 12, { "ld", "hl", "d16" } };
			ops[0x22] = operation{ 1,  8, { "ld", "(hl+)", "a" } };
			ops[0x23] = operation{ 1,  8, { "inc", "hl" } };
			ops[0x24] = operation{ 1,  4, { "inc", "h" } };
			ops[0x25] = operation{ 1,  4, { "dec", "h" } };
			ops[0x26] = operation{ 2,  8, { "ld", "h", "d8" } };
			ops[0x27] = operation{ 1,  4, { "daa" } };
			ops[0x28] = operation{ 2,  8, { "jr", "z", "r8" } };
			ops[0x29] = operation{ 1,  8, { "add", "hl", "hl" } };
			ops[0x2a] = operation{ 1,  8, { "ld", "a", "(hl+)" } };
			ops[0x2b] = operation{ 1,  8, { "dec", "hl" } };
			ops[0x2c] = operation{ 1,  4, { "inc", "l" } };
			ops[0x2d] = operation{ 1,  4, { "dec", "l" } };
			ops[0x2e] = operation{ 2,  8, { "ld", "l", "d8" } };
			ops[0x2f] = operation{ 1,  4, { "cpl" } };

			ops[0x30] = operation{ 2,  8, { "jr", "nc", "r8" } };
			ops[0x31] = operation{ 3, 12, { "ld", "sp", "d16" } };
			ops[0x32] = operation{ 1,  8, { "ld", "(hl-)", "a" } };
			ops[0x33] = operation{ 1,  8, { "inc", "sp" } };
			ops[0x34] = operation{ 1, 12, { "inc", "(hl)" } };
			ops[0x35] = operation{ 1, 12, { "dec", "(hl)" } };
			ops[0x36] = operation{ 2, 12, { "ld", "(hl)", "d8" } };
			ops[0x37] = operation{ 1,  4, { "scf" } };
			ops[0x38] = operation{ 2,  8, { "jr", "c", "r8" } };
			ops[0x39] = operation{ 1,  8, { "add", "hl", "sp" } };
			ops[0x3a] = operation{ 1,  8, { "ld", "a", "(hl-)" } };
			ops[0x3b] = operation{ 1,  8, { "dec", "sp" } };
			ops[0x3c] = operation{ 1,  4, { "inc", "a" } };
			ops[0x3d] = operation{ 1,  4, { "dec", "a" } };
			ops[0x3e] = operation{ 2,  8, { "ld", "a", "d8" } };
			ops[0x3f] = operation{ 1,  4, { "ccf" } };

			ops[0x40] = operation{ 1,  4, { "ld", "b", "b" } };
			ops[0x41] = operation{ 1,  4, { "ld", "b", "c" } };
			ops[0x42] = operation{ 1,  4, { "ld", "b", "d" } };
			ops[0x43] = operation{ 1,  4, { "ld", "b", "e" } };
			ops[0x44] = operation{ 1,  4, { "ld", "b", "h" } };
			ops[0x45] = operation{ 1,  4, { "ld", "b", "l" } };
			ops[0x46] = operation{ 1,  8, { "ld", "b", "(hl)" } };
			ops[0x47] = operation{ 1,  4, { "ld", "b", "a" } };
			ops[0x48] = operation{ 1,  4, { "ld", "c", "b" } };
			ops[0x49] = operation{ 1,  4, { "ld", "c", "c" } };
			ops[0x4a] = operation{ 1,  4, { "ld", "c", "d" } };
			ops[0x4b] = operation{ 1,  4, { "ld", "c", "e" } };
			ops[0x4c] = operation{ 1,  4, { "ld", "c", "h" } };
			ops[0x4d] = operation{ 1,  4, { "ld", "c", "l" } };
			ops[0x4e] = operation{ 1,  8, { "ld", "c", "(hl)" } };
			ops[0x4f] = operation{ 1,  4, { "ld", "c", "a" } };

			ops[0x50] = operation{ 1,  4, { "ld", "d", "b" } };
			ops[0x51] = operation{ 1,  4, { "ld", "d", "c" } };
			ops[0x52] = operation{ 1,  4, { "ld", "d", "d" } };
			ops[0x53] = operation{ 1,  4, { "ld", "d", "e" } };
			ops[0x54] = operation{ 1,  4, { "ld", "d", "h" } };
			ops[0x55] = operation{ 1,  4, { "ld", "d", "l" } };
			ops[0x56] = operation{ 1,  8, { "ld", "d", "(hl)" } };
			ops[0x57] = operation{ 1,  4, { "ld", "d", "a" } };
			ops[0x58] = operation{ 1,  4, { "ld", "e", "b" } };
			ops[0x59] = operation{ 1,  4, { "ld", "e", "c" } };
			ops[0x5a] = operation{ 1,  4, { "ld", "e", "d" } };
			ops[0x5b] = operation{ 1,  4, { "ld", "e", "e" } };
			ops[0x5c] = operation{ 1,  4, { "ld", "e", "h" } };
			ops[0x5d] = operation{ 1,  4, { "ld", "e", "l" } };
			ops[0x5e] = operation{ 1,  8, { "ld", "e", "(hl)" } };
			ops[0x5f] = operation{ 1,  4, { "ld", "e", "a" } };

			ops[0x60] = operation{ 1,  4, { "ld", "h", "b" } };
			ops[0x61] = operation{ 1,  4, { "ld", "h", "c" } };
			ops[0x62] = operation{ 1,  4, { "ld", "h", "d" } };
			ops[0x63] = operation{ 1,  4, { "ld", "h", "e" } };
			ops[0x64] = operation{ 1,  4, { "ld", "h", "h" } };
			ops[0x65] = operation{ 1,  4, { "ld", "h", "l" } };
			ops[0x66] = operation{ 1,  8, { "ld", "h", "(hl)" } };
			ops[0x67] = operation{ 1,  4, { "ld", "h", "a" } };
			ops[0x68] = operation{ 1,  4, { "ld", "l", "b" } };
			ops[0x69] = operation{ 1,  4, { "ld", "l", "c" } };
			ops[0x6a] = operation{ 1,  4, { "ld", "l", "d" } };
			ops[0x6b] = operation{ 1,  4, { "ld", "l", "e" } };
			ops[0x6c] = operation{ 1,  4, { "ld", "l", "h" } };
			ops[0x6d] = operation{ 1,  4, { "ld", "l", "l" } };
			ops[0x6e] = operation{ 1,  8, { "ld", "l", "(hl)" } };
			ops[0x6f] = operation{ 1,  4, { "ld l,a" } };

			ops[0x70] = operation{ 1,  8, { "ld", "(hl)", "b" } };
			ops[0x71] = operation{ 1,  8, { "ld", "(hl)", "c" } };
			ops[0x72] = operation{ 1,  8, { "ld", "(hl)", "d" } };
			ops[0x73] = operation{ 1,  8, { "ld", "(hl)", "e" } };
			ops[0x74] = operation{ 1,  8, { "ld", "(hl)", "h" } };
			ops[0x75] = operation{ 1,  8, { "ld", "(hl)", "l" } };
			ops[0x76] = operation{ 1,  4, { "halt" } };
			ops[0x77] = operation{ 1,  8, { "ld", "(hl)", "a" } };
			ops[0x78] = operation{ 1,  4, { "ld", "a", "b" } };
			ops[0x79] = operation{ 1,  4, { "ld", "a", "c" } };
			ops[0x7a] = operation{ 1,  4, { "ld", "a", "d" } };
			ops[0x7b] = operation{ 1,  4, { "ld", "a", "e" } };
			ops[0x7c] = operation{ 1,  4, { "ld", "a", "h" } };
			ops[0x7d] = operation{ 1,  4, { "ld", "a", "l" } };
			ops[0x7e] = operation{ 1,  8, { "ld", "a", "(hl)" } };
			ops[0x7f] = operation{ 1,  4, { "ld", "a", "a" } };

			ops[0x80] = operation{ 1,  4, { "add", "a", "b" } };
			ops[0x81] = operation{ 1,  4, { "add", "a", "c" } };
			ops[0x82] = operation{ 1,  4, { "add", "a", "d" } };
			ops[0x83] = operation{ 1,  4, { "add", "a", "e" } };
			ops[0x84] = operation{ 1,  4, { "add", "a", "h" } };
			ops[0x85] = operation{ 1,  4, { "add", "a", "l" } };
			ops[0x86] = operation{ 1,  8, { "add", "a", "(hl)" } };
			ops[0x87] = operation{ 1,  4, { "add", "a", "a" } };
			ops[0x88] = operation{ 1,  4, { "adc", "a", "b" } };
			ops[0x89] = operation{ 1,  4, { "adc", "a", "c" } };
			ops[0x8a] = operation{ 1,  4, { "adc", "a", "d" } };
			ops[0x8b] = operation{ 1,  4, { "adc", "a", "e" } };
			ops[0x8c] = operation{ 1,  4, { "adc", "a", "h" } };
			ops[0x8d] = operation{ 1,  4, { "adc", "a", "l" } };
			ops[0x8e] = operation{ 1,  8, { "adc", "a", "(hl)" } };
			ops[0x8f] = operation{ 1,  4, { "adc", "a", "a" } };

			ops[0x90] = operation{ 1,  4, { "sub", "b" } };
			ops[0x91] = operation{ 1,  4, { "sub", "c" } };
			ops[0x92] = operation{ 1,  4, { "sub", "d" } };
			ops[0x93] = operation{ 1,  4, { "sub", "e" } };
			ops[0x94] = operation{ 1,  4, { "sub", "h" } };
			ops[0x95] = operation{ 1,  4, { "sub", "l" } };
			ops[0x96] = operation{ 1,  8, { "sub", "(hl)" } };
			ops[0x97] = operation{ 1,  4, { "sub", "a" } };
			ops[0x98] = operation{ 1,  4, { "sbc", "a", "b" } };
			ops[0x99] = operation{ 1,  4, { "sbc", "a", "c" } };
			ops[0x9a] = operation{ 1,  4, { "sbc", "a", "d" } };
			ops[0x9b] = operation{ 1,  4, { "sbc", "a", "e" } };
			ops[0x9c] = operation{ 1,  4, { "sbc", "a", "h" } };
			ops[0x9d] = operation{ 1,  4, { "sbc", "a", "l" } };
			ops[0x9e] = operation{ 1,  8, { "sbc", "a", "(hl)" } };
			ops[0x9f] = operation{ 1,  4, { "sbc", "a", "a" } };

			ops[0xa0] = operation{ 1,  4, { "and", "b" } };
			ops[0xa1] = operation{ 1,  4, { "and", "c" } };
			ops[0xa2] = operation{ 1,  4, { "and", "d" } };
			ops[0xa3] = operation{ 1,  4, { "and", "e" } };
			ops[0xa4] = operation{ 1,  4, { "and", "h" } };
			ops[0xa5] = operation{ 1,  4, { "and", "l" } };
			ops[0xa6] = operation{ 1,  8, { "and", "(hl)" } };
			ops[0xa7] = operation{ 1,  4, { "and", "a" } };
			ops[0xa8] = operation{ 1,  4, { "xor", "b" } };
			ops[0xa9] = operation{ 1,  4, { "xor", "c" } };
			ops[0xaa] = operation{ 1,  4, { "xor", "d" } };
			ops[0xab] = operation{ 1,  4, { "xor", "e" } };
			ops[0xac] = operation{ 1,  4, { "xor", "h" } };
			ops[0xad] = operation{ 1,  4, { "xor", "l" } };
			ops[0xae] = operation{ 1,  8, { "xor", "(hl)" } };
			ops[0xaf] = operation{ 1,  4, { "xor", "a" } };

			ops[0xb0] = operation{ 1,  4, { "or", "b" } };
			ops[0xb1] = operation{ 1,  4, { "or", "c" } };
			ops[0xb2] = operation{ 1,  4, { "or", "d" } };
			ops[0xb3] = operation{ 1,  4, { "or", "e" } };
			ops[0xb4] = operation{ 1,  4, { "or", "h" } };
			ops[0xb5] = operation{ 1,  4, { "or", "l" } };
			ops[0xb6] = operation{ 1,  8, { "or", "(hl)" } };
			ops[0xb7] = operation{ 1,  4, { "or", "a" } };
			ops[0xb8] = operation{ 1,  4, { "cp", "b" } };
			ops[0xb9] = operation{ 1,  4, { "cp", "c" } };
			ops[0xba] = operation{ 1,  4, { "cp", "d" } };
			ops[0xbb] = operation{ 1,  4, { "cp", "e" } };
			ops[0xbc] = operation{ 1,  4, { "cp", "h" } };
			ops[0xbd] = operation{ 1,  4, { "cp", "l" } };
			ops[0xbe] = operation{ 1,  8, { "cp", "(hl)" } };
			ops[0xbf] = operation{ 1,  4, { "cp", "a" } };

			ops[0xc0] = operation{ 1,  8, { "ret", "nz" } };
			ops[0xc1] = operation{ 1, 12, { "pop", "bc" } };
			ops[0xc2] = operation{ 3, 12, { "jp", "nz", "a16" } };
			ops[0xc3] = operation{ 3, 16, { "jp", "a16" } };
			ops[0xc4] = operation{ 3, 12, { "call", "nz", "a16" } };
			ops[0xc5] = operation{ 1, 16, { "push", "bc" } };
			ops[0xc6] = operation{ 2,  8, { "add", "a", "d8" } };
			ops[0xc7] = operation{ 1, 16, { "rst", "00h" } };
			ops[0xc8] = operation{ 1,  8, { "ret", "z" } };
			ops[0xc9] = operation{ 1, 16, { "ret" } };
			ops[0xca] = operation{ 3, 12, { "jp", "z", "a16" } };
			ops[0xcb] = operation{ 0,  0, { "prefix", "cb" } };
			ops[0xcc] = operation{ 3, 12, { "call", "z", "a16" } };
			ops[0xcd] = operation{ 3, 24, { "call", "a16" } };
			ops[0xce] = operation{ 2,  8, { "adc", "a", "d8" } };
			ops[0xcf] = operation{ 1, 16, { "rst", "08h" } };

			ops[0xd0] = operation{ 1,  8, { "ret", "nc" } };
			ops[0xd1] = operation{ 1, 12, { "pop", "de" } };
			ops[0xd2] = operation{ 3, 12, { "jp", "nc", "a16" } };
			ops[0xd3] = operation{ 1,  4, { "inv" } };
			ops[0xd4] = operation{ 3, 12, { "call", "nc", "a16" } };
			ops[0xd5] = operation{ 1, 16, { "push", "de" } };
			ops[0xd6] = operation{ 2,  8, { "sub", "d8" } };
			ops[0xd7] = operation{ 1, 16, { "rst", "10h" } };
			ops[0xd8] = operation{ 1,  8, { "ret", "c" } };
			ops[0xd9] = operation{ 1, 16, { "reti" } };
			ops[0xda] = operation{ 3, 12, { "jp", "c", "a16" } };
			ops[0xdb] = operation{ 1,  4, { "inv" } };
			ops[0xdc] = operation{ 3, 12, { "call", "c", "a16" } };
			ops[0xdd] = operation{ 1,  4, { "inv" } };
			ops[0xde] = operation{ 2,  8, { "sbc", "a", "d8" } };
			ops[0xdf] = operation{ 1, 16, { "rst", "18h" } };

			ops[0xe0] = operation{ 2, 12, { "ldh", "(a8)", "a" } };
			ops[0xe1] = operation{ 1, 12, { "pop", "hl" } };
			ops[0xe2] = operation{ 2,  8, { "ld", "(c)", "a" } };
			ops[0xe3] = operation{ 1,  4, { "inv" } };
			ops[0xe4] = operation{ 1,  4, { "inv" } };
			ops[0xe5] = operation{ 1, 16, { "push", "hl" } };
			ops[0xe6] = operation{ 1,  4, { "and", "d8" } };
			ops[0xe7] = operation{ 1, 16, { "rst", "20h" } };
			ops[0xe8] = operation{ 2, 16, { "add", "sp", "r8" } };
			ops[0xe9] = operation{ 1,  4, { "jp", "(hl)" } };
			ops[0xea] = operation{ 3, 16, { "ld", "(a16)", "a" } };
			ops[0xeb] = operation{ 1,  4, { "inv" } };
			ops[0xec] = operation{ 1,  4, { "inv" } };
			ops[0xed] = operation{ 1,  4, { "inv" } };
			ops[0xee] = operation{ 2,  8, { "xor", "d8" } };
			ops[0xef] = operation{ 1, 16, { "rst", "28h" } };
#
			ops[0xf0] = operation{ 2, 12, { "ldh", "a", "(a8)" } };
			ops[0xf1] = operation{ 1, 12, { "pop", "af" } };
			ops[0xf2] = operation{ 2,  8, { "ld", "a", "(c)" } };
			ops[0xf3] = operation{ 1,  4, { "di" } };
			ops[0xf4] = operation{ 1,  4, { "inv" } };
			ops[0xf5] = operation{ 1, 16, { "push", "af" } };
			ops[0xf6] = operation{ 2,  8, { "or", "d8" } };
			ops[0xf7] = operation{ 1, 16, { "rst", "30h" } };
			ops[0xf8] = operation{ 2, 12, { "ld", "hl", "sp+r8" } };
			ops[0xf9] = operation{ 1,  4, { "ld", "sp", "hl" } };
			ops[0xfa] = operation{ 3, 16, { "ld", "a", "(a16)" } };
			ops[0xfb] = operation{ 1,  4, { "ei" } };
			ops[0xfc] = operation{ 1,  4, { "inv" } };
			ops[0xfd] = operation{ 1,  4, { "inv" } };
			ops[0xfe] = operation{ 2,  8, { "cp", "d8" } };
			ops[0xff] = operation{ 1, 16, { "rst", "38h" } };

			assert(ops.size() == 0x100);
		}

		void set_operations_cb()
		{
			auto& ops = ops_cb_;
			ops.reserve(0x100);

			ops[0x00] = operation{ 2,  8, { "rlc", "b" } };
			ops[0x01] = operation{ 2,  8, { "rlc", "c" } };
			ops[0x02] = operation{ 2,  8, { "rlc", "d" } };
			ops[0x03] = operation{ 2,  8, { "rlc", "e" } };
			ops[0x04] = operation{ 2,  8, { "rlc", "h" } };
			ops[0x05] = operation{ 2,  8, { "rlc", "l" } };
			ops[0x06] = operation{ 2, 16, { "rlc", "(hl)" } };
			ops[0x07] = operation{ 2,  8, { "rlc", "a" } };
			ops[0x08] = operation{ 2,  8, { "rrc", "b" } };
			ops[0x09] = operation{ 2,  8, { "rrc", "c" } };
			ops[0x0a] = operation{ 2,  8, { "rrc", "d" } };
			ops[0x0b] = operation{ 2,  8, { "rrc", "e" } };
			ops[0x0c] = operation{ 2,  8, { "rrc", "h" } };
			ops[0x0d] = operation{ 2,  8, { "rrc", "l" } };
			ops[0x0e] = operation{ 2, 16, { "rrc", "(hl)" } };
			ops[0x0f] = operation{ 2,  8, { "rrc", "a" } };

			ops[0x10] = operation{ 2,  8, { "rl", "b" } };
			ops[0x11] = operation{ 2,  8, { "rl", "c" } };
			ops[0x12] = operation{ 2,  8, { "rl", "d" } };
			ops[0x13] = operation{ 2,  8, { "rl", "e" } };
			ops[0x14] = operation{ 2,  8, { "rl", "h" } };
			ops[0x15] = operation{ 2,  8, { "rl", "l" } };
			ops[0x16] = operation{ 2, 16, { "rl", "(hl)" } };
			ops[0x17] = operation{ 2,  8, { "rl", "a" } };
			ops[0x18] = operation{ 2,  8, { "rr", "b" } };
			ops[0x19] = operation{ 2,  8, { "rr", "c" } };
			ops[0x1a] = operation{ 2,  8, { "rr", "d" } };
			ops[0x1b] = operation{ 2,  8, { "rr", "e" } };
			ops[0x1c] = operation{ 2,  8, { "rr", "h" } };
			ops[0x1d] = operation{ 2,  8, { "rr", "l" } };
			ops[0x1e] = operation{ 2, 16, { "rr", "(hl)" } };
			ops[0x1f] = operation{ 2,  8, { "rr", "a" } };

			ops[0x20] = operation{ 2,  8, { "sla", "b" } };
			ops[0x21] = operation{ 2,  8, { "sla", "c" } };
			ops[0x22] = operation{ 2,  8, { "sla", "d" } };
			ops[0x23] = operation{ 2,  8, { "sla", "e" } };
			ops[0x24] = operation{ 2,  8, { "sla", "h" } };
			ops[0x25] = operation{ 2,  8, { "sla", "l" } };
			ops[0x26] = operation{ 2, 16, { "sla", "(hl)" } };
			ops[0x27] = operation{ 2,  8, { "sla", "a" } };
			ops[0x28] = operation{ 2,  8, { "sra", "b" } };
			ops[0x29] = operation{ 2,  8, { "sra", "c" } };
			ops[0x2a] = operation{ 2,  8, { "sra", "d" } };
			ops[0x2b] = operation{ 2,  8, { "sra", "e" } };
			ops[0x2c] = operation{ 2,  8, { "sra", "h" } };
			ops[0x2d] = operation{ 2,  8, { "sra", "l" } };
			ops[0x2e] = operation{ 2, 16, { "sra", "(hl)" } };
			ops[0x2f] = operation{ 2,  8, { "sra", "a" } };

			ops[0x30] = operation{ 2,  8, { "swap", "b" } };
			ops[0x31] = operation{ 2,  8, { "swap", "c" } };
			ops[0x32] = operation{ 2,  8, { "swap", "d" } };
			ops[0x33] = operation{ 2,  8, { "swap", "e" } };
			ops[0x34] = operation{ 2,  8, { "swap", "h" } };
			ops[0x35] = operation{ 2,  8, { "swap", "l" } };
			ops[0x36] = operation{ 2, 16, { "swap", "(hl)" } };
			ops[0x37] = operation{ 2,  8, { "swap", "a" } };
			ops[0x38] = operation{ 2,  8, { "srl", "b" } };
			ops[0x39] = operation{ 2,  8, { "srl", "c" } };
			ops[0x3a] = operation{ 2,  8, { "srl", "d" } };
			ops[0x3b] = operation{ 2,  8, { "srl", "e" } };
			ops[0x3c] = operation{ 2,  8, { "srl", "h" } };
			ops[0x3d] = operation{ 2,  8, { "srl", "l" } };
			ops[0x3e] = operation{ 2, 16, { "srl", "(hl)" } };
			ops[0x3f] = operation{ 2,  8, { "srl", "a" } };

			ops[0x40] = operation{ 2,  8, { "bit", "0", "b" } };
			ops[0x41] = operation{ 2,  8, { "bit", "0", "c" } };
			ops[0x42] = operation{ 2,  8, { "bit", "0", "d" } };
			ops[0x43] = operation{ 2,  8, { "bit", "0", "e" } };
			ops[0x44] = operation{ 2,  8, { "bit", "0", "h" } };
			ops[0x45] = operation{ 2,  8, { "bit", "0", "l" } };
			ops[0x46] = operation{ 2, 16, { "bit", "0", "(hl)" } };
			ops[0x47] = operation{ 2,  8, { "bit", "0", "a" } };
			ops[0x48] = operation{ 2,  8, { "bit", "1", "b" } };
			ops[0x49] = operation{ 2,  8, { "bit", "1", "c" } };
			ops[0x4a] = operation{ 2,  8, { "bit", "1", "d" } };
			ops[0x4b] = operation{ 2,  8, { "bit", "1", "e" } };
			ops[0x4c] = operation{ 2,  8, { "bit", "1", "h" } };
			ops[0x4d] = operation{ 2,  8, { "bit", "1", "l" } };
			ops[0x4e] = operation{ 2, 16, { "bit", "1", "(hl)" } };
			ops[0x4f] = operation{ 2,  8, { "bit", "1", "a" } };

			ops[0x50] = operation{ 2,  8, { "bit", "2", "b" } };
			ops[0x51] = operation{ 2,  8, { "bit", "2", "c" } };
			ops[0x52] = operation{ 2,  8, { "bit", "2", "d" } };
			ops[0x53] = operation{ 2,  8, { "bit", "2", "e" } };
			ops[0x54] = operation{ 2,  8, { "bit", "2", "h" } };
			ops[0x55] = operation{ 2,  8, { "bit", "2", "l" } };
			ops[0x56] = operation{ 2, 16, { "bit", "2", "(hl)" } };
			ops[0x57] = operation{ 2,  8, { "bit", "2", "a" } };
			ops[0x58] = operation{ 2,  8, { "bit", "3", "b" } };
			ops[0x59] = operation{ 2,  8, { "bit", "3", "c" } };
			ops[0x5a] = operation{ 2,  8, { "bit", "3", "d" } };
			ops[0x5b] = operation{ 2,  8, { "bit", "3", "e" } };
			ops[0x5c] = operation{ 2,  8, { "bit", "3", "h" } };
			ops[0x5d] = operation{ 2,  8, { "bit", "3", "l" } };
			ops[0x5e] = operation{ 2, 16, { "bit", "3", "(hl)" } };
			ops[0x5f] = operation{ 2,  8, { "bit", "3", "a" } };

			ops[0x60] = operation{ 2,  8, { "bit", "4", "b" } };
			ops[0x61] = operation{ 2,  8, { "bit", "4", "c" } };
			ops[0x62] = operation{ 2,  8, { "bit", "4", "d" } };
			ops[0x63] = operation{ 2,  8, { "bit", "4", "e" } };
			ops[0x64] = operation{ 2,  8, { "bit", "4", "h" } };
			ops[0x65] = operation{ 2,  8, { "bit", "4", "l" } };
			ops[0x66] = operation{ 2, 16, { "bit", "4", "(hl)" } };
			ops[0x67] = operation{ 2,  8, { "bit", "4", "a" } };
			ops[0x68] = operation{ 2,  8, { "bit", "5", "b" } };
			ops[0x69] = operation{ 2,  8, { "bit", "5", "c" } };
			ops[0x6a] = operation{ 2,  8, { "bit", "5", "d" } };
			ops[0x6b] = operation{ 2,  8, { "bit", "5", "e" } };
			ops[0x6c] = operation{ 2,  8, { "bit", "5", "h" } };
			ops[0x6d] = operation{ 2,  8, { "bit", "5", "l" } };
			ops[0x6e] = operation{ 2, 16, { "bit", "5", "(hl)" } };
			ops[0x6f] = operation{ 2,  8, { "bit", "5", "a" } };

			ops[0x70] = operation{ 2,  8, { "bit", "6", "b" } };
			ops[0x71] = operation{ 2,  8, { "bit", "6", "c" } };
			ops[0x72] = operation{ 2,  8, { "bit", "6", "d" } };
			ops[0x73] = operation{ 2,  8, { "bit", "6", "e" } };
			ops[0x74] = operation{ 2,  8, { "bit", "6", "h" } };
			ops[0x75] = operation{ 2,  8, { "bit", "6", "l" } };
			ops[0x76] = operation{ 2, 16, { "bit", "6", "(hl)" } };
			ops[0x77] = operation{ 2,  8, { "bit", "6", "a" } };
			ops[0x78] = operation{ 2,  8, { "bit", "7", "b" } };
			ops[0x79] = operation{ 2,  8, { "bit", "7", "c" } };
			ops[0x7a] = operation{ 2,  8, { "bit", "7", "d" } };
			ops[0x7b] = operation{ 2,  8, { "bit", "7", "e" } };
			ops[0x7c] = operation{ 2,  8, { "bit", "7", "h" } };
			ops[0x7d] = operation{ 2,  8, { "bit", "7", "l" } };
			ops[0x7e] = operation{ 2, 16, { "bit", "7", "(hl)" } };
			ops[0x7f] = operation{ 2,  8, { "bit", "7", "a" } };

			ops[0x80] = operation{ 2,  8, { "res", "0", "b" } };
			ops[0x81] = operation{ 2,  8, { "res", "0", "c" } };
			ops[0x82] = operation{ 2,  8, { "res", "0", "d" } };
			ops[0x83] = operation{ 2,  8, { "res", "0", "e" } };
			ops[0x84] = operation{ 2,  8, { "res", "0", "h" } };
			ops[0x85] = operation{ 2,  8, { "res", "0", "l" } };
			ops[0x86] = operation{ 2, 16, { "res", "0", "(hl)" } };
			ops[0x87] = operation{ 2,  8, { "res", "0", "a" } };
			ops[0x88] = operation{ 2,  8, { "res", "1", "b" } };
			ops[0x89] = operation{ 2,  8, { "res", "1", "c" } };
			ops[0x8a] = operation{ 2,  8, { "res", "1", "d" } };
			ops[0x8b] = operation{ 2,  8, { "res", "1", "e" } };
			ops[0x8c] = operation{ 2,  8, { "res", "1", "h" } };
			ops[0x8d] = operation{ 2,  8, { "res", "1", "l" } };
			ops[0x8e] = operation{ 2, 16, { "res", "1", "(hl)" } };
			ops[0x8f] = operation{ 2,  8, { "res", "1", "a" } };

			ops[0x90] = operation{ 2,  8, { "res", "2", "b" } };
			ops[0x91] = operation{ 2,  8, { "res", "2", "c" } };
			ops[0x92] = operation{ 2,  8, { "res", "2", "d" } };
			ops[0x93] = operation{ 2,  8, { "res", "2", "e" } };
			ops[0x94] = operation{ 2,  8, { "res", "2", "h" } };
			ops[0x95] = operation{ 2,  8, { "res", "2", "l" } };
			ops[0x96] = operation{ 2, 16, { "res", "2", "(hl)" } };
			ops[0x97] = operation{ 2,  8, { "res", "2", "a" } };
			ops[0x98] = operation{ 2,  8, { "res", "3", "b" } };
			ops[0x99] = operation{ 2,  8, { "res", "3", "c" } };
			ops[0x9a] = operation{ 2,  8, { "res", "3", "d" } };
			ops[0x9b] = operation{ 2,  8, { "res", "3", "e" } };
			ops[0x9c] = operation{ 2,  8, { "res", "3", "h" } };
			ops[0x9d] = operation{ 2,  8, { "res", "3", "l" } };
			ops[0x9e] = operation{ 2, 16, { "res", "3", "(hl)" } };
			ops[0x9f] = operation{ 2,  8, { "res", "3", "a" } };

			ops[0xa0] = operation{ 2,  8, { "res", "4", "b" } };
			ops[0xa1] = operation{ 2,  8, { "res", "4", "c" } };
			ops[0xa2] = operation{ 2,  8, { "res", "4", "d" } };
			ops[0xa3] = operation{ 2,  8, { "res", "4", "e" } };
			ops[0xa4] = operation{ 2,  8, { "res", "4", "h" } };
			ops[0xa5] = operation{ 2,  8, { "res", "4", "l" } };
			ops[0xa6] = operation{ 2, 16, { "res", "4", "(hl)" } };
			ops[0xa7] = operation{ 2,  8, { "res", "4", "a" } };
			ops[0xa8] = operation{ 2,  8, { "res", "5", "b" } };
			ops[0xa9] = operation{ 2,  8, { "res", "5", "c" } };
			ops[0xaa] = operation{ 2,  8, { "res", "5", "d" } };
			ops[0xab] = operation{ 2,  8, { "res", "5", "e" } };
			ops[0xac] = operation{ 2,  8, { "res", "5", "h" } };
			ops[0xad] = operation{ 2,  8, { "res", "5", "l" } };
			ops[0xae] = operation{ 2, 16, { "res", "5", "(hl)" } };
			ops[0xaf] = operation{ 2,  8, { "res", "5", "a" } };

			ops[0xb0] = operation{ 2,  8, { "res", "6", "b" } };
			ops[0xb1] = operation{ 2,  8, { "res", "6", "c" } };
			ops[0xb2] = operation{ 2,  8, { "res", "6", "d" } };
			ops[0xb3] = operation{ 2,  8, { "res", "6", "e" } };
			ops[0xb4] = operation{ 2,  8, { "res", "6", "h" } };
			ops[0xb5] = operation{ 2,  8, { "res", "6", "l" } };
			ops[0xb6] = operation{ 2, 16, { "res", "6", "(hl)" } };
			ops[0xb7] = operation{ 2,  8, { "res", "6", "a" } };
			ops[0xb8] = operation{ 2,  8, { "res", "7", "b" } };
			ops[0xb9] = operation{ 2,  8, { "res", "7", "c" } };
			ops[0xba] = operation{ 2,  8, { "res", "7", "d" } };
			ops[0xbb] = operation{ 2,  8, { "res", "7", "e" } };
			ops[0xbc] = operation{ 2,  8, { "res", "7", "h" } };
			ops[0xbd] = operation{ 2,  8, { "res", "7", "l" } };
			ops[0xbe] = operation{ 2, 16, { "res", "7", "(hl)" } };
			ops[0xbf] = operation{ 2,  8, { "res", "7", "a" } };

			ops[0xc0] = operation{ 2,  8, { "set", "0", "b" } };
			ops[0xc1] = operation{ 2,  8, { "set", "0", "c" } };
			ops[0xc2] = operation{ 2,  8, { "set", "0", "d" } };
			ops[0xc3] = operation{ 2,  8, { "set", "0", "e" } };
			ops[0xc4] = operation{ 2,  8, { "set", "0", "h" } };
			ops[0xc5] = operation{ 2,  8, { "set", "0", "l" } };
			ops[0xc6] = operation{ 2, 16, { "set", "0", "(hl)" } };
			ops[0xc7] = operation{ 2,  8, { "set", "0", "a" } };
			ops[0xc8] = operation{ 2,  8, { "set", "1", "b" } };
			ops[0xc9] = operation{ 2,  8, { "set", "1", "c" } };
			ops[0xca] = operation{ 2,  8, { "set", "1", "d" } };
			ops[0xcb] = operation{ 2,  8, { "set", "1", "e" } };
			ops[0xcc] = operation{ 2,  8, { "set", "1", "h" } };
			ops[0xcd] = operation{ 2,  8, { "set", "1", "l" } };
			ops[0xce] = operation{ 2, 16, { "set", "1", "(hl)" } };
			ops[0xcf] = operation{ 2,  8, { "set", "1", "a" } };

			ops[0xd0] = operation{ 2,  8, { "set", "2", "b" } };
			ops[0xd1] = operation{ 2,  8, { "set", "2", "c" } };
			ops[0xd2] = operation{ 2,  8, { "set", "2", "d" } };
			ops[0xd3] = operation{ 2,  8, { "set", "2", "e" } };
			ops[0xd4] = operation{ 2,  8, { "set", "2", "h" } };
			ops[0xd5] = operation{ 2,  8, { "set", "2", "l" } };
			ops[0xd6] = operation{ 2, 16, { "set", "2", "(hl)" } };
			ops[0xd7] = operation{ 2,  8, { "set", "2", "a" } };
			ops[0xd8] = operation{ 2,  8, { "set", "3", "b" } };
			ops[0xd9] = operation{ 2,  8, { "set", "3", "c" } };
			ops[0xda] = operation{ 2,  8, { "set", "3", "d" } };
			ops[0xdb] = operation{ 2,  8, { "set", "3", "e" } };
			ops[0xdc] = operation{ 2,  8, { "set", "3", "h" } };
			ops[0xdd] = operation{ 2,  8, { "set", "3", "l" } };
			ops[0xde] = operation{ 2, 16, { "set", "3", "(hl)" } };
			ops[0xdf] = operation{ 2,  8, { "set", "3", "a" } };

			ops[0xe0] = operation{ 2,  8, { "set", "4", "b" } };
			ops[0xe1] = operation{ 2,  8, { "set", "4", "c" } };
			ops[0xe2] = operation{ 2,  8, { "set", "4", "d" } };
			ops[0xe3] = operation{ 2,  8, { "set", "4", "e" } };
			ops[0xe4] = operation{ 2,  8, { "set", "4", "h" } };
			ops[0xe5] = operation{ 2,  8, { "set", "4", "l" } };
			ops[0xe6] = operation{ 2, 16, { "set", "4", "(hl)" } };
			ops[0xe7] = operation{ 2,  8, { "set", "4", "a" } };
			ops[0xe8] = operation{ 2,  8, { "set", "5", "b" } };
			ops[0xe9] = operation{ 2,  8, { "set", "5", "c" } };
			ops[0xea] = operation{ 2,  8, { "set", "5", "d" } };
			ops[0xeb] = operation{ 2,  8, { "set", "5", "e" } };
			ops[0xec] = operation{ 2,  8, { "set", "5", "h" } };
			ops[0xed] = operation{ 2,  8, { "set", "5", "l" } };
			ops[0xee] = operation{ 2, 16, { "set", "5", "(hl)" } };
			ops[0xef] = operation{ 2,  8, { "set", "5", "a" } };

			ops[0xf0] = operation{ 2,  8, { "set", "6", "b" } };
			ops[0xf1] = operation{ 2,  8, { "set", "6", "c" } };
			ops[0xf2] = operation{ 2,  8, { "set", "6", "d" } };
			ops[0xf3] = operation{ 2,  8, { "set", "6", "e" } };
			ops[0xf4] = operation{ 2,  8, { "set", "6", "h" } };
			ops[0xf5] = operation{ 2,  8, { "set", "6", "l" } };
			ops[0xf6] = operation{ 2, 16, { "set", "6", "(hl)" } };
			ops[0xf7] = operation{ 2,  8, { "set", "6", "a" } };
			ops[0xf8] = operation{ 2,  8, { "set", "7", "b" } };
			ops[0xf9] = operation{ 2,  8, { "set", "7", "c" } };
			ops[0xfa] = operation{ 2,  8, { "set", "7", "d" } };
			ops[0xfb] = operation{ 2,  8, { "set", "7", "e" } };
			ops[0xfc] = operation{ 2,  8, { "set", "7", "h" } };
			ops[0xfd] = operation{ 2,  8, { "set", "7", "l" } };
			ops[0xfe] = operation{ 2, 16, { "set", "7", "(hl)" } };
			ops[0xff] = operation{ 2,  8, { "set", "7", "a" } };

			assert(ops.size() == 0x100);
		}

		mmu&		mmu_;
		operations	ops_;
		operations	ops_cb_;
	};
}
