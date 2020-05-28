//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <unordered_map>
#include <string>
#include <cassert>

#include <naive_gbe/cpu.hpp>

namespace naive_gbe
{
	class disassembler
	{
	public:

		disassembler()
		{
			set_operations();
			set_operations_ex();
		}

		std::string const& disassembly(std::uint8_t opcode, bool extended)
		{
			operations& ops = extended ? ops_ex_ : ops_;

			return ops[opcode];
		}

		std::string disassembly(std::uint8_t opcode, bool extended, std::uint8_t size, std::uint8_t* addr)
		{
			constexpr std::size_t const BUFSZ = 32;

			char buffer[BUFSZ];
			std::string text;
			operations& ops = extended ? ops_ex_ : ops_;
			auto const& decoded = ops[opcode];

			switch (size)
			{
			case 1:
				text = decoded;
				break;
			case 2:
				snprintf(buffer, BUFSZ, decoded.c_str(), *addr);
				text = buffer;
				break;
			case 3:
				snprintf(buffer, BUFSZ, decoded.c_str(), *reinterpret_cast<std::uint16_t*>(addr));
				text = buffer;
				break;
			}

			return text;
		}

	private:

		using operations = std::unordered_map<std::uint8_t, std::string>;

		void set_operations()
		{
			auto& ops = ops_;
			ops.reserve(0x100);

			for (std::size_t addr = 0; addr < 0x100; ++addr)
				ops[static_cast<std::uint8_t>(addr)] = "UNDEF";

			ops[0x00] = "NOP";
			ops[0x01] = "LD     BC,    0x%04X";
			ops[0x02] = "LD   (BC),    A";
			ops[0x03] = "INC    BC";
			ops[0x04] = "INC     B";
			ops[0x05] = "DEC     B";
			ops[0x06] = "LD      B,    0x%02X";
			ops[0x0a] = "LD      A,    (BC)";
			ops[0x0b] = "DEC    BC";
			ops[0x0c] = "INC     C";
			ops[0x0d] = "DEC     C";
			ops[0x0e] = "LD      C,    0x%02X";

			ops[0x10] = "STOP";
			ops[0x11] = "LD     DE,    0x%04X";
			ops[0x12] = "LD   (DE),    A";
			ops[0x13] = "INC    DE";
			ops[0x14] = "INC     D";
			ops[0x15] = "DEC     D";
			ops[0x16] = "LD      D,    0x%02X";
			ops[0x1a] = "LD      A,    (DE)";
			ops[0x1b] = "DEC    DE";
			ops[0x1c] = "INC     E";
			ops[0x1d] = "DEC     E";
			ops[0x1e] = "LD      E,    0x%02X";

			ops[0x21] = "LD     HL,    0x%04X";
			ops[0x22] = "LDI  (HL),    A";
			ops[0x23] = "INC    HL";
			ops[0x24] = "INC     H";
			ops[0x25] = "DEC     H";
			ops[0x26] = "LD      H,    0x%02X";
			ops[0x2a] = "LDI     A,    (HL)";
			ops[0x2b] = "DEC    HL";
			ops[0x2c] = "INC     L";
			ops[0x2d] = "DEC     L";
			ops[0x2e] = "LD      L,    0x%02X";

			ops[0x31] = "LD     SP,    0x%04X";
			ops[0x32] = "LDD  (HL),    A";
			ops[0x33] = "INC    SP";
			ops[0x34] = "INC  (HL)";
			ops[0x35] = "DEC  (HL)";
			ops[0x36] = "LD     HL,    0x%02X";
			ops[0x3a] = "LDD     A,    (HL)";
			ops[0x3b] = "DEC    SP";
			ops[0x3c] = "INC     A";
			ops[0x3d] = "DEC     A";
			ops[0x3e] = "LD      A,    0x%02X";

			ops[0xcb] = "CB";

			ops[0x40] = "LD      B,    B";
			ops[0x41] = "LD      B,    C";
			ops[0x42] = "LD      B,    D";
			ops[0x43] = "LD      B,    E";
			ops[0x44] = "LD      B,    H";
			ops[0x45] = "LD      B,    L";
			ops[0x46] = "LD      B,    (HL)";
			ops[0x47] = "LD      B,    A";
			ops[0x48] = "LD      C,    B";
			ops[0x49] = "LD      C,    C";
			ops[0x4a] = "LD      C,    D";
			ops[0x4b] = "LD      C,    E";
			ops[0x4c] = "LD      C,    H";
			ops[0x4d] = "LD      C,    L";
			ops[0x4e] = "LD      C,    (HL)";
			ops[0x4f] = "LD      C,    A";

			ops[0x50] = "LD      D,    B";
			ops[0x51] = "LD      D,    C";
			ops[0x52] = "LD      D,    D";
			ops[0x53] = "LD      D,    E";
			ops[0x54] = "LD      D,    H";
			ops[0x55] = "LD      D,    L";
			ops[0x56] = "LD      D,    (HL)";
			ops[0x57] = "LD      D,    A";
			ops[0x58] = "LD      E,    B";
			ops[0x59] = "LD      E,    C";
			ops[0x5a] = "LD      E,    D";
			ops[0x5b] = "LD      E,    E";
			ops[0x5c] = "LD      E,    H";
			ops[0x5d] = "LD      E,    L";
			ops[0x5e] = "LD      E,    (HL)";
			ops[0x5f] = "LD      E,    A";

			ops[0x60] = "LD      H,    B";
			ops[0x61] = "LD      H,    C";
			ops[0x62] = "LD      H,    D";
			ops[0x63] = "LD      H,    E";
			ops[0x64] = "LD      H,    H";
			ops[0x65] = "LD      H,    L";
			ops[0x66] = "LD      H,    (HL)";
			ops[0x67] = "LD      H,    A";
			ops[0x68] = "LD      L,    B";
			ops[0x69] = "LD      L,    C";
			ops[0x6a] = "LD      L,    D";
			ops[0x6b] = "LD      L,    E";
			ops[0x6c] = "LD      L,    H";
			ops[0x6d] = "LD      L,    L";
			ops[0x6e] = "LD      L,    (HL)";
			ops[0x6f] = "LD      L,    A";

			ops[0x70] = "LD      (HL),    B";
			ops[0x71] = "LD      (HL),    C";
			ops[0x72] = "LD      (HL),    D";
			ops[0x73] = "LD      (HL),    E";
			ops[0x74] = "LD      (HL),    H";
			ops[0x75] = "LD      (HL),    L";
			ops[0x76] = "HALT";
			ops[0x77] = "LD      (HL),    A";
			ops[0x78] = "LD      A,       B";
			ops[0x79] = "LD      A,       C";
			ops[0x7a] = "LD      A,       D";
			ops[0x7b] = "LD      A,       E";
			ops[0x7c] = "LD      A,       H";
			ops[0x7d] = "LD      A,       L";
			ops[0x7e] = "LD      A,       (HL)";
			ops[0x7f] = "LD      A,       A";


			ops[0xa8] = "XOR     B";
			ops[0xa9] = "XOR     C";
			ops[0xaa] = "XOR     D";
			ops[0xab] = "XOR     E";
			ops[0xac] = "XOR     H";
			ops[0xad] = "XOR     L";
			ops[0xae] = "XOR     (HL)";
			ops[0xaf] = "XOR     A";

			ops[0xd3] = "UNDEF";
			ops[0xdb] = "UNDEF";
			ops[0xdd] = "UNDEF";

			ops[0xe3] = "UNDEF";
			ops[0xe4] = "UNDEF";
			ops[0xeb] = "UNDEF";
			ops[0xec] = "UNDEF";
			ops[0xed] = "UNDEF";

			ops[0xf4] = "UNDEF";
			ops[0xfc] = "UNDEF";
			ops[0xfd] = "UNDEF";

			assert(ops.size() == 0x100);
		}

		void set_operations_ex()
		{
			auto& ops = ops_ex_;
			ops.reserve(0x100);

			ops[0x00] = "CB RLC  B";
			ops[0x01] = "CB RLC  C";
			ops[0x02] = "CB RLC  D";
			ops[0x03] = "CB RLC  E";
			ops[0x04] = "CB RLC  H";
			ops[0x05] = "CB RLC  L";
			ops[0x06] = "CB RLC  (HL)";
			ops[0x07] = "CB RLC  A";
			ops[0x08] = "CB RRC  B";
			ops[0x09] = "CB RRC  C";
			ops[0x0a] = "CB RRC  D";
			ops[0x0b] = "CB RRC  E";
			ops[0x0c] = "CB RRC  H";
			ops[0x0d] = "CB RRC  L";
			ops[0x0e] = "CB RRC  (HL)";
			ops[0x0f] = "CB RRC  A";
								
			ops[0x10] = "CB RL   B";
			ops[0x11] = "CB RL   C";
			ops[0x12] = "CB RL   D";
			ops[0x13] = "CB RL   E";
			ops[0x14] = "CB RL   H";
			ops[0x15] = "CB RL   L";
			ops[0x16] = "CB RL   (HL)";
			ops[0x17] = "CB RL   A";
			ops[0x18] = "CB RR   B";
			ops[0x19] = "CB RR   C";
			ops[0x1a] = "CB RR   D";
			ops[0x1b] = "CB RR   E";
			ops[0x1c] = "CB RR   H";
			ops[0x1d] = "CB RR   L";
			ops[0x1e] = "CB RR   (HL)";
			ops[0x1f] = "CB RR   A";

			ops[0x20] = "CB SLA  B";
			ops[0x21] = "CB SLA  C";
			ops[0x22] = "CB SLA  D";
			ops[0x23] = "CB SLA  E";
			ops[0x24] = "CB SLA  H";
			ops[0x25] = "CB SLA  L";
			ops[0x26] = "CB SLA  (HL)";
			ops[0x27] = "CB SLA  A";
			ops[0x28] = "CB SRA  B";
			ops[0x29] = "CB SRA  C";
			ops[0x2a] = "CB SRA  D";
			ops[0x2b] = "CB SRA  E";
			ops[0x2c] = "CB SRA  H";
			ops[0x2d] = "CB SRA  L";
			ops[0x2e] = "CB SRA  (HL)";
			ops[0x2f] = "CB SRA  A";

			ops[0x30] = "CB SWAP B";
			ops[0x31] = "CB SWAP C";
			ops[0x32] = "CB SWAP D";
			ops[0x33] = "CB SWAP E";
			ops[0x34] = "CB SWAP H";
			ops[0x35] = "CB SWAP L";
			ops[0x36] = "CB SWAP (HL)";
			ops[0x37] = "CB SWAP A";
			ops[0x38] = "CB SRL  B";
			ops[0x39] = "CB SRL  C";
			ops[0x3a] = "CB SRL  D";
			ops[0x3b] = "CB SRL  E";
			ops[0x3c] = "CB SRL  H";
			ops[0x3d] = "CB SRL  L";
			ops[0x3e] = "CB SRL  (HL)";
			ops[0x3f] = "CB SRL  A";

			ops[0x40] = "CB BIT  0,    B";
			ops[0x41] = "CB BIT  0,    C";
			ops[0x42] = "CB BIT  0,    D";
			ops[0x43] = "CB BIT  0,    E";
			ops[0x44] = "CB BIT  0,    H";
			ops[0x45] = "CB BIT  0,    L";
			ops[0x46] = "CB BIT  0,    (HL)";
			ops[0x47] = "CB BIT  0,    A";
			ops[0x48] = "CB BIT  1,    B";
			ops[0x49] = "CB BIT  1,    C";
			ops[0x4a] = "CB BIT  1,    D";
			ops[0x4b] = "CB BIT  1,    E";
			ops[0x4c] = "CB BIT  1,    H";
			ops[0x4d] = "CB BIT  1,    L";
			ops[0x4e] = "CB BIT  1,    (HL)";
			ops[0x4f] = "CB BIT  1,    A";

			ops[0x50] = "CB BIT  2,    B";
			ops[0x51] = "CB BIT  2,    C";
			ops[0x52] = "CB BIT  2,    D";
			ops[0x53] = "CB BIT  2,    E";
			ops[0x54] = "CB BIT  2,    H";
			ops[0x55] = "CB BIT  2,    L";
			ops[0x56] = "CB BIT  2,    (HL)";
			ops[0x57] = "CB BIT  2,    A";
			ops[0x58] = "CB BIT  3,    B";
			ops[0x59] = "CB BIT  3,    C";
			ops[0x5a] = "CB BIT  3,    D";
			ops[0x5b] = "CB BIT  3,    E";
			ops[0x5c] = "CB BIT  3,    H";
			ops[0x5d] = "CB BIT  3,    L";
			ops[0x5e] = "CB BIT  3,    (HL)";
			ops[0x5f] = "CB BIT  3,    A";

			ops[0x60] = "CB BIT  4,    B";
			ops[0x61] = "CB BIT  4,    C";
			ops[0x62] = "CB BIT  4,    D";
			ops[0x63] = "CB BIT  4,    E";
			ops[0x64] = "CB BIT  4,    H";
			ops[0x65] = "CB BIT  4,    L";
			ops[0x66] = "CB BIT  4,    (HL)";
			ops[0x67] = "CB BIT  4,    A";
			ops[0x68] = "CB BIT  5,    B";
			ops[0x69] = "CB BIT  5,    C";
			ops[0x6a] = "CB BIT  5,    D";
			ops[0x6b] = "CB BIT  5,    E";
			ops[0x6c] = "CB BIT  5,    H";
			ops[0x6d] = "CB BIT  5,    L";
			ops[0x6e] = "CB BIT  5,    (HL)";
			ops[0x6f] = "CB BIT  5,    A";

			ops[0x70] = "CB BIT  6,    B";
			ops[0x71] = "CB BIT  6,    C";
			ops[0x72] = "CB BIT  6,    D";
			ops[0x73] = "CB BIT  6,    E";
			ops[0x74] = "CB BIT  6,    H";
			ops[0x75] = "CB BIT  6,    L";
			ops[0x76] = "CB BIT  6,    (HL)";
			ops[0x77] = "CB BIT  6,    A";
			ops[0x78] = "CB BIT  7,    B";
			ops[0x79] = "CB BIT  7,    C";
			ops[0x7a] = "CB BIT  7,    D";
			ops[0x7b] = "CB BIT  7,    E";
			ops[0x7c] = "CB BIT  7,    H";
			ops[0x7d] = "CB BIT  7,    L";
			ops[0x7e] = "CB BIT  7,    (HL)";
			ops[0x7f] = "CB BIT  7,    A";

			ops[0x80] = "CB RES  0,    B";
			ops[0x81] = "CB RES  0,    C";
			ops[0x82] = "CB RES  0,    D";
			ops[0x83] = "CB RES  0,    E";
			ops[0x84] = "CB RES  0,    H";
			ops[0x85] = "CB RES  0,    L";
			ops[0x86] = "CB RES  0,    (HL)";
			ops[0x87] = "CB RES  0,    A";
			ops[0x88] = "CB RES  1,    B";
			ops[0x89] = "CB RES  1,    C";
			ops[0x8a] = "CB RES  1,    D";
			ops[0x8b] = "CB RES  1,    E";
			ops[0x8c] = "CB RES  1,    H";
			ops[0x8d] = "CB RES  1,    L";
			ops[0x8e] = "CB RES  1,    (HL)";
			ops[0x8f] = "CB RES  1,    A";

			ops[0x90] = "CB RES  2,    B";
			ops[0x91] = "CB RES  2,    C";
			ops[0x92] = "CB RES  2,    D";
			ops[0x93] = "CB RES  2,    E";
			ops[0x94] = "CB RES  2,    H";
			ops[0x95] = "CB RES  2,    L";
			ops[0x96] = "CB RES  2,    (HL)";
			ops[0x97] = "CB RES  2,    A";
			ops[0x98] = "CB RES  3,    B";
			ops[0x99] = "CB RES  3,    C";
			ops[0x9a] = "CB RES  3,    D";
			ops[0x9b] = "CB RES  3,    E";
			ops[0x9c] = "CB RES  3,    H";
			ops[0x9d] = "CB RES  3,    L";
			ops[0x9e] = "CB RES  3,    (HL)";
			ops[0x9f] = "CB RES  3,    A";

			ops[0xa0] = "CB RES  4,    B";
			ops[0xa1] = "CB RES  4,    C";
			ops[0xa2] = "CB RES  4,    D";
			ops[0xa3] = "CB RES  4,    E";
			ops[0xa4] = "CB RES  4,    H";
			ops[0xa5] = "CB RES  4,    L";
			ops[0xa6] = "CB RES  4,    (HL)";
			ops[0xa7] = "CB RES  4,    A";
			ops[0xa8] = "CB RES  5,    B";
			ops[0xa9] = "CB RES  5,    C";
			ops[0xaa] = "CB RES  5,    D";
			ops[0xab] = "CB RES  5,    E";
			ops[0xac] = "CB RES  5,    H";
			ops[0xad] = "CB RES  5,    L";
			ops[0xae] = "CB RES  5,    (HL)";
			ops[0xaf] = "CB RES  5,    A";

			ops[0xb0] = "CB RES  6,    B";
			ops[0xb1] = "CB RES  6,    C";
			ops[0xb2] = "CB RES  6,    D";
			ops[0xb3] = "CB RES  6,    E";
			ops[0xb4] = "CB RES  6,    H";
			ops[0xb5] = "CB RES  6,    L";
			ops[0xb6] = "CB RES  6,    (HL)";
			ops[0xb7] = "CB RES  6,    A";
			ops[0xb8] = "CB RES  7,    B";
			ops[0xb9] = "CB RES  7,    C";
			ops[0xba] = "CB RES  7,    D";
			ops[0xbb] = "CB RES  7,    E";
			ops[0xbc] = "CB RES  7,    H";
			ops[0xbd] = "CB RES  7,    L";
			ops[0xbe] = "CB RES  7,    (HL)";
			ops[0xbf] = "CB RES  7,    A";

			ops[0xc0] = "CB SET  0,    B";
			ops[0xc1] = "CB SET  0,    C";
			ops[0xc2] = "CB SET  0,    D";
			ops[0xc3] = "CB SET  0,    E";
			ops[0xc4] = "CB SET  0,    H";
			ops[0xc5] = "CB SET  0,    L";
			ops[0xc6] = "CB SET  0,    (HL)";
			ops[0xc7] = "CB SET  0,    A";
			ops[0xc8] = "CB SET  1,    B";
			ops[0xc9] = "CB SET  1,    C";
			ops[0xca] = "CB SET  1,    D";
			ops[0xcb] = "CB SET  1,    E";
			ops[0xcc] = "CB SET  1,    H";
			ops[0xcd] = "CB SET  1,    L";
			ops[0xce] = "CB SET  1,    (HL)";
			ops[0xcf] = "CB SET  1,    A";

			ops[0xd0] = "CB SET  2,    B";
			ops[0xd1] = "CB SET  2,    C";
			ops[0xd2] = "CB SET  2,    D";
			ops[0xd3] = "CB SET  2,    E";
			ops[0xd4] = "CB SET  2,    H";
			ops[0xd5] = "CB SET  2,    L";
			ops[0xd6] = "CB SET  2,    (HL)";
			ops[0xd7] = "CB SET  2,    A";
			ops[0xd8] = "CB SET  3,    B";
			ops[0xd9] = "CB SET  3,    C";
			ops[0xda] = "CB SET  3,    D";
			ops[0xdb] = "CB SET  3,    E";
			ops[0xdc] = "CB SET  3,    H";
			ops[0xdd] = "CB SET  3,    L";
			ops[0xde] = "CB SET  3,    (HL)";
			ops[0xdf] = "CB SET  3,    A";

			ops[0xe0] = "CB SET  4,    B";
			ops[0xe1] = "CB SET  4,    C";
			ops[0xe2] = "CB SET  4,    D";
			ops[0xe3] = "CB SET  4,    E";
			ops[0xe4] = "CB SET  4,    H";
			ops[0xe5] = "CB SET  4,    L";
			ops[0xe6] = "CB SET  4,    (HL)";
			ops[0xe7] = "CB SET  4,    A";
			ops[0xe8] = "CB SET  5,    B";
			ops[0xe9] = "CB SET  5,    C";
			ops[0xea] = "CB SET  5,    D";
			ops[0xeb] = "CB SET  5,    E";
			ops[0xec] = "CB SET  5,    H";
			ops[0xed] = "CB SET  5,    L";
			ops[0xee] = "CB SET  5,    (HL)";
			ops[0xef] = "CB SET  5,    A";

			ops[0xf0] = "CB SET  6,    B";
			ops[0xf1] = "CB SET  6,    C";
			ops[0xf2] = "CB SET  6,    D";
			ops[0xf3] = "CB SET  6,    E";
			ops[0xf4] = "CB SET  6,    H";
			ops[0xf5] = "CB SET  6,    L";
			ops[0xf6] = "CB SET  6,    (HL)";
			ops[0xf7] = "CB SET  6,    A";
			ops[0xf8] = "CB SET  7,    B";
			ops[0xf9] = "CB SET  7,    C";
			ops[0xfa] = "CB SET  7,    D";
			ops[0xfb] = "CB SET  7,    E";
			ops[0xfc] = "CB SET  7,    H";
			ops[0xfd] = "CB SET  7,    L";
			ops[0xfe] = "CB SET  7,    (HL)";
			ops[0xff] = "CB SET  7,    A";

			assert(ops.size() == 0x100);
		}

		operations	ops_;
		operations	ops_ex_;
	};
}
