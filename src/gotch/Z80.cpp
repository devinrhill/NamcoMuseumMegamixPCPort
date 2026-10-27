// 1:1 port of the gotch Z80 core (nmwii2.rso). Every function cites its RSO2 address; the opcode handlers cite the dispatcher's jump table
// (RSO2 0x80dcbb78 main, 0x80dcb778 CB, 0x80dcb3a0 DD, 0x80dcadcc ED, 0x80dca9f4 FD, 0x80dcafbc DD CB, 0x80dca610 FD CB).
//
// Quirks of the original, kept on purpose (marked "quirk:" at the site):
//  * FD-prefixed 8-bit ops on B read the FD register table's entry 0, which Z80_Init never writes: offset 0 = the top byte of the cycle budget.
//  * FD 09/19/29/39 (ADD IY,rr) use the DD pair table: ADD IY,IY adds IX.
//  * NMIgo clears IFF1 before testing it, so IFF2 is never updated.
//  * OTIR / OTDR loop inside one call and move PC back 2 per extra pass (the repeat of LDIR & co. re-executes the opcode instead).
//  * CPI / CPD / INI / IND / OUTI / OUTD cost no cycles; CPI / CPD set N only when the result is not zero.
//  * BIT never sets S (it ORs in bit 7 of the bit NUMBER); LD A,I / LD A,R put IFF2 into flag bit 3 (0x08), not P/V.
//  * DI clears every status bit above 0xfc (also STOP / HALTWAIT / RESET / DISABLE), IM 0 / 1 / 2 keep only the low byte; RST 0 is the native hook.
//  * INTgo clears 0xff5c: also the upper status bits. The R register counts all 8 bits, once per instruction (prefixes included).
#include "gotch/Z80.hpp"

namespace gotch
{
namespace
{
// the register offset tables Z80_Init writes (RSO2 bss 0x811dca58 + 8 / 0x10 / 0x18 / 0x20 / 0x24 / 0x28 / 0x2c)
const std::uint8_t kR8[8] = {0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x21, 0x20};    // B C D E H L (F) A
const std::uint8_t kR8IX[8] = {0x22, 0x23, 0x24, 0x25, 0x30, 0x31, 0x21, 0x20};  // H / L -> IXh / IXl
const std::uint8_t kR8IY[8] = {0x00, 0x23, 0x24, 0x25, 0x32, 0x33, 0x21, 0x20};  // quirk: entry 0 is never written (bss zero)
const std::uint8_t kRR[4] = {0x22, 0x24, 0x26, 0x34};                             // BC DE HL SP
const std::uint8_t kRRPush[4] = {0x22, 0x24, 0x26, 0x20};                         // BC DE HL AF
const std::uint8_t kRRIX[4] = {0x22, 0x24, 0x30, 0x34};                           // BC DE IX SP (DD and FD ADD use it)
// +0x2c {BC DE IY SP} is written by Z80_Init but read by nothing

struct Tables
{
	std::uint8_t sz[256], szp[256], inc[256], dec[256];
	Tables()
	{
		for (int v = 0; v < 256; v++)
		{
			const std::uint8_t s = (std::uint8_t)((v & 0xa8) | (v == 0 ? 0x40 : 0));
			int bits = 0;
			for (int b = 0; b < 8; b++) bits += (v >> b) & 1;
			sz[v] = s;
			szp[v] = (std::uint8_t)(s | ((bits & 1) == 0 ? 0x04 : 0));
			inc[v] = (std::uint8_t)(s | ((v & 0x0f) == 0 ? 0x10 : 0) | (v == 0x80 ? 0x04 : 0));
			dec[v] = (std::uint8_t)(s | 0x02 | ((v & 0x0f) == 0x0f ? 0x10 : 0) | (v == 0x7f ? 0x04 : 0));
		}
	}
};
const Tables& T()
{
	static const Tables t;
	return t;
}

using Env = Z80_env;

std::uint8_t ReadM1(Env& z, std::uint16_t a) { return z.ReadM1(a); }          // RSO2 0x80d0d7e0 _Emu_ReadM1
std::uint8_t Read8(Env& z, std::uint16_t a) { return z.ReadMemory8(a); }      // RSO2 0x80d130d8 _Emu_ReadMemory8
void Write8(Env& z, std::uint16_t a, std::uint8_t v) { z.WriteMemory8(a, v); } // RSO2 0x80d13054 _Emu_WriteMemory8
std::uint8_t ReadIO8(Env& z, std::uint16_t p) { return z.ReadIO8(p); }        // RSO2 0x80d133b8 _Emu_ReadIO8
void WriteIO8(Env& z, std::uint16_t p, std::uint8_t v) { z.WriteIO8(p, v); }  // RSO2 0x80d1339c _Emu_WriteIO8

// RSO2 0x80d0d588 _Emu_ReadMemory16: high byte first
std::uint16_t Read16(Env& z, std::uint16_t a)
{
	const std::uint8_t hi = z.ReadMemory8((std::uint16_t)(a + 1));
	const std::uint8_t lo = z.ReadMemory8(a);
	return (std::uint16_t)(hi << 8 | lo);
}

// RSO2 0x80d0d508 _Emu_WriteMemory16: low byte first
void Write16(Env& z, std::uint16_t a, std::uint16_t v)
{
	z.WriteMemory8(a, (std::uint8_t)v);
	z.WriteMemory8((std::uint16_t)(a + 1), (std::uint8_t)(v >> 8));
}

// RSO2 0x80d130f0 _Exchange
void Exchange(Env& z, int a, int b)
{
	const std::uint16_t t = z.r16(a);
	z.set16(a, z.r16(b));
	z.set16(b, t);
}

// RSO2 0x80d135b0 Uncd: undefined opcode
void Uncd(Env& z) { z.setStatus((std::uint16_t)(z.status() | Env::ST_STOP)); }

std::uint8_t& Fl(Env& z) { return z.m[Env::F]; }

// RSO2 0x80d13070 _Emu_MakeFlagInc8
void Inc8(Env& z, std::uint8_t& r)
{
	r = (std::uint8_t)(r + 1);
	Fl(z) = (std::uint8_t)((Fl(z) & 1) | T().inc[r]);
}

// RSO2 0x80d130a4 _Emu_MakeFlagDec8
void Dec8(Env& z, std::uint8_t& r)
{
	r = (std::uint8_t)(r - 1);
	Fl(z) = (std::uint8_t)((Fl(z) & 1) | T().dec[r]);
}

// RSO2 0x80d13104 _Emu_MakeFlagAdd16 (S / Z / P/V kept, bits 3 / 5 not touched)
void Add16(Env& z, int dst, std::uint16_t b)
{
	const std::uint32_t a = z.r16(dst);
	const std::uint32_t r = a + b;
	z.set16(dst, (std::uint16_t)r);
	Fl(z) = (std::uint8_t)((Fl(z) & 0xc4) | ((r >> 16) & 1) | (((a ^ b ^ r) >> 8) & 0x10));
}

// RSO2 0x80d13668 _Emu_MakeFlagAddc16
void Adc16(Env& z, int dst, std::uint16_t b)
{
	const std::uint32_t a = z.r16(dst);
	const std::uint32_t r = (Fl(z) & 1) + a + b;
	z.set16(dst, (std::uint16_t)r);
	const std::uint32_t v = (((a ^ b) ^ 0x8000) & (b ^ r) & 0x8000) >> 13;
	Fl(z) = (std::uint8_t)((((a ^ b ^ r) >> 8) & 0x10) | ((r >> 16) & 1) | v | ((r & 0xffff) == 0 ? 0x40 : 0) | ((r >> 15) & 1 ? 0x80 : 0));
}

// RSO2 0x80d135e8 _Emu_MakeFlagSubc16
void Sbc16(Env& z, int dst, std::uint16_t b)
{
	const std::uint32_t a = z.r16(dst);
	const std::uint32_t r = a - b - (Fl(z) & 1);
	z.set16(dst, (std::uint16_t)r);
	const std::uint32_t v = ((a ^ b) & (a ^ r) & 0x8000) >> 13;
	Fl(z) = (std::uint8_t)(((r >> 16) & 1) | 0x02 | (((a ^ b ^ r) >> 8) & 0x10) | v | ((r & 0xffff) == 0 ? 0x40 : 0) | ((r >> 15) & 1 ? 0x80 : 0));
}

// RSO2 0x80d13140 _Emu_MakeFlagAdd8 / 0x80d1319c _Emu_MakeFlagAddc8
void Add8(Env& z, std::uint8_t& d, std::uint8_t b, std::uint32_t carry)
{
	const std::uint32_t a = d;
	const std::uint32_t r = carry + a + b;
	d = (std::uint8_t)r;
	Fl(z) = (std::uint8_t)(((r >> 8) & 1) | T().sz[r & 0xff] | ((a ^ b ^ r) & 0x10) | ((((a ^ b) ^ 0x80) & (r ^ b) & 0x80) >> 5));
}

// RSO2 0x80d13204 _Emu_MakeFlagSub8 / 0x80d13260 _Emu_MakeFlagSubc8
void Sub8(Env& z, std::uint8_t& d, std::uint8_t b, std::uint32_t carry)
{
	const std::uint32_t a = d;
	const std::uint32_t r = a - b - carry;
	d = (std::uint8_t)r;
	Fl(z) = (std::uint8_t)(((r >> 8) & 1) | 0x02 | T().sz[r & 0xff] | ((a ^ b ^ r) & 0x10) | (((a ^ b) & (a ^ r) & 0x80) >> 5));
}

// RSO2 0x80d13344 _Emu_MakeFlagCmp8
void Cmp8(Env& z, std::uint8_t a8, std::uint8_t b8)
{
	const std::uint32_t a = a8, b = b8;
	const std::uint32_t r = a - b;
	const std::uint32_t r8 = r & 0xff;
	Fl(z) = (std::uint8_t)(((r >> 8) & 1) | 0x02 | T().sz[r8] | ((b ^ a ^ r8) & 0x10) | (((a ^ b) & (a ^ r8) & 0x80) >> 5));
}

// RSO2 0x80d132c8 _Emu_MakeFlagAnd8 / 0x80d132f4 _Emu_MakeFlagXor8 / 0x80d1331c _Emu_MakeFlagOr8
void And8(Env& z, std::uint8_t& d, std::uint8_t b)
{
	d = (std::uint8_t)(d & b);
	Fl(z) = (std::uint8_t)(T().szp[d] | 0x10);
}
void Xor8(Env& z, std::uint8_t& d, std::uint8_t b)
{
	d = (std::uint8_t)(d ^ b);
	Fl(z) = T().szp[d];
}
void Or8(Env& z, std::uint8_t& d, std::uint8_t b)
{
	d = (std::uint8_t)(d | b);
	Fl(z) = T().szp[d];
}

// the eight ALU ops of x0..xf (ADD ADC SUB SBC AND XOR OR CP), operand already fetched
void Alu(Env& z, int op, std::uint8_t v)
{
	std::uint8_t& a = z.r8(Env::A);
	switch ((op >> 3) & 7)
	{
	case 0: Add8(z, a, v, 0); break;
	case 1: Add8(z, a, v, Fl(z) & 1u); break;
	case 2: Sub8(z, a, v, 0); break;
	case 3: Sub8(z, a, v, Fl(z) & 1u); break;
	case 4: And8(z, a, v); break;
	case 5: Xor8(z, a, v); break;
	case 6: Or8(z, a, v); break;
	default: Cmp8(z, a, v); break;
	}
}

// RSO2 0x80d133d0 Rlc / 0x80d13404 Rrc / 0x80d13434 Rl / 0x80d1346c Rr / 0x80d134a4 Sla / 0x80d134d0 Sra / 0x80d13504 Sll / 0x80d13534 Srl
void Shift(Env& z, int kind, std::uint8_t& r)
{
	const std::uint32_t v = r;
	const auto& szp = T().szp;
	switch (kind & 7)
	{
	case 0:
	{
		std::uint32_t t = v << 1;
		const std::uint32_t c = (t >> 8) & 1;
		t |= c;
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)(c | szp[t & 0xff]);
		break;
	}
	case 1:
	{
		const std::uint32_t t = (v >> 1) | ((v << 7) & 0x80);
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)((v & 1) | szp[t]);
		break;
	}
	case 2:
	{
		const std::uint32_t t = (Fl(z) & 1u) | ((v << 1) & 0x1fe);
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)(szp[t & 0xff] | ((t >> 8) & 1));
		break;
	}
	case 3:
	{
		const std::uint32_t t = (v >> 1) | ((Fl(z) & 1u) << 7);
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)((v & 1) | szp[t & 0xff]);
		break;
	}
	case 4:
	{
		const std::uint32_t t = (v << 1) & 0xfe;
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)(((v >> 7) & 1) | szp[t]);
		break;
	}
	case 5:
	{
		const std::int32_t s = (std::int8_t)v;
		const std::uint32_t t = (std::uint32_t)(s >> 1) & 0xff;
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)((v & 1) | szp[t]);
		break;
	}
	case 6:
	{
		const std::uint32_t t = ((v << 1) & 0xfe) | 1;
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)(((v >> 7) & 1) | szp[t]);
		break;
	}
	default:
	{
		const std::uint32_t t = v >> 1;
		r = (std::uint8_t)t;
		Fl(z) = (std::uint8_t)((v & 1) | szp[t]);
		break;
	}
	}
}

// RSO2 0x80d13560 _Emu_MakeFlagBit; quirk: "n & 0x80" (the bit NUMBER) where the S flag was meant
void Bit(Env& z, std::uint8_t v, std::int32_t n)
{
	const std::uint32_t b = v & (1u << n);
	Fl(z) = (std::uint8_t)((Fl(z) & 1) | (b == 0 ? 0x40 : 0) | 0x10 | (n & 0x80));
}

// RSO2 0x80d135c0 _Emu_MakeFlagIn
void FlagIn(Env& z, std::uint8_t v) { Fl(z) = (std::uint8_t)((Fl(z) & 1) | T().szp[v]); }

// RSO2 0x80d136e8 _Emu_MakeFlagAIR; quirk: IFF2 lands in bit 3 (0x08)
void FlagAIR(Env& z) { Fl(z) = (std::uint8_t)((Fl(z) & 1) | T().sz[z.r8(Env::A)] | ((z.status() << 2) & 0x08)); }

// RSO2 0x80d1371c _Emu_MakeFlagRxd
void FlagRxd(Env& z) { Fl(z) = (std::uint8_t)((Fl(z) & 1) | T().szp[z.r8(Env::A)]); }

// RSO2 0x80d13744 _Emu_sub_ldi (step +1) / 0x80d139f8 _Emu_sub_ldd (step -1)
void BlockLd(Env& z, int step)
{
	const std::uint16_t hl = z.r16(Env::HL);
	z.set16(Env::HL, (std::uint16_t)(hl + step));
	const std::uint8_t v = Read8(z, hl);
	const std::uint32_t n = (std::uint32_t)(v + z.r8(Env::A)) & 0xff;
	const std::uint16_t de = z.r16(Env::DE);
	z.set16(Env::DE, (std::uint16_t)(de + step));
	Write8(z, de, v);
	const std::uint16_t bc = (std::uint16_t)(z.r16(Env::BC) - 1);
	z.set16(Env::BC, bc);
	Fl(z) = (std::uint8_t)((Fl(z) & 0xc1) | ((n & 0x02) ? 0x20 : 0) | ((n & 0x08) ? 0x08 : 0) | (bc != 0 ? 0x04 : 0));
}

// RSO2 0x80d13814 _Emu_sub_cpi (+1) / 0x80d13ac8 _Emu_sub_cpd (-1); quirk: N only when A != (HL)
void BlockCp(Env& z, int step)
{
	const std::uint16_t hl = z.r16(Env::HL);
	z.set16(Env::HL, (std::uint16_t)(hl + step));
	const std::uint32_t v = Read8(z, hl);
	const std::uint32_t a = z.r8(Env::A);
	const std::uint32_t r = a - v;
	z.set16(Env::BC, (std::uint16_t)(z.r16(Env::BC) - 1));
	std::uint32_t n = r & 0xff;
	std::uint32_t f = Fl(z) & 1;
	if (n == 0)
	{
		f |= 0x40;
	}
	else
	{
		f |= ((n >> 7) & 1) ? 0x80 : 0;
		f |= (n ^ v ^ a) & 0x10;
		f |= 0x02;
	}
	if (f & 0x10)
	{
		n = 0xff;
	}
	f |= ((n >> 1) & 1) ? 0x20 : 0;
	f |= ((n >> 3) & 1) ? 0x08 : 0;
	f |= z.r16(Env::BC) != 0 ? 0x04 : 0;
	Fl(z) = (std::uint8_t)f;
}

// RSO2 0x80d138f8 _Emu_sub_ini (+1) / 0x80d13bac _Emu_sub_ind (-1)
void BlockIn(Env& z, int step)
{
	const std::uint8_t v = ReadIO8(z, z.r16(Env::BC));
	const std::uint16_t hl = z.r16(Env::HL);
	z.set16(Env::HL, (std::uint16_t)(hl + step));
	Write8(z, hl, v);
	const std::uint8_t b = (std::uint8_t)(z.r8(Env::B) - 1);
	z.r8(Env::B) = b;
	Fl(z) = b == 0 ? 0x42 : 0x02;
}

// RSO2 0x80d13978 _Emu_sub_outi (+1) / 0x80d13c2c _Emu_sub_outd (-1)
void BlockOut(Env& z, int step)
{
	const std::uint16_t hl = z.r16(Env::HL);
	z.set16(Env::HL, (std::uint16_t)(hl + step));
	const std::uint8_t v = Read8(z, hl);
	WriteIO8(z, z.r16(Env::BC), v);
	const std::uint8_t b = (std::uint8_t)(z.r8(Env::B) - 1);
	z.r8(Env::B) = b;
	Fl(z) = b == 0 ? 0x42 : 0x02;
}

std::uint16_t PC(Env& z) { return z.r16(Env::PC); }
void AddPC(Env& z, int d) { z.set16(Env::PC, (std::uint16_t)(z.r16(Env::PC) + d)); }

void Push(Env& z, std::uint16_t v)
{
	const std::uint16_t sp = (std::uint16_t)(z.r16(Env::SP) - 2);
	z.set16(Env::SP, sp);
	Write16(z, sp, v);
}

std::uint16_t Pop(Env& z)
{
	const std::uint16_t v = Read16(z, z.r16(Env::SP));
	z.set16(Env::SP, (std::uint16_t)(z.r16(Env::SP) + 2));
	return v;
}

// the condition of the cc opcodes (bits 3..5): NZ Z NC C PO PE P M
bool Cond(Env& z, int op)
{
	const std::uint8_t f = Fl(z);
	switch ((op >> 3) & 7)
	{
	case 0: return (f & 0x40) == 0;
	case 1: return (f & 0x40) != 0;
	case 2: return (f & 0x01) == 0;
	case 3: return (f & 0x01) != 0;
	case 4: return (f & 0x04) == 0;
	case 5: return (f & 0x04) != 0;
	case 6: return (f & 0x80) == 0;
	default: return (f & 0x80) != 0;
	}
}

// main table 0x27 (RSO2 0x80d0dddc): DAA from the Zilog table, written as the original's chain of cases
void Daa(Env& z)
{
	const std::uint32_t a = z.r8(Env::A);
	std::uint32_t f = Fl(z);
	const std::uint32_t c = f & 1;
	const bool h = (f & 0x10) != 0;
	const std::uint32_t hi = a >> 4;
	const std::uint32_t lo = a & 0x0f;
	std::uint32_t diff = 0;
	if (!c && hi <= 9 && !h && lo <= 9) diff = 0;
	else if (!c && hi <= 9 && h && lo <= 9) diff = 6;
	else if (!c && hi <= 8 && lo >= 10) diff = 6;
	else if (!c && hi >= 10 && !h && lo <= 9) diff = 0x60;
	else if (c && !h && lo <= 9) diff = 0x60;
	else if (c && h && lo <= 9) diff = 0x66;
	else if (c && lo >= 10) diff = 0x66;
	else if (!c && hi >= 9 && lo >= 10) diff = 0x66;
	else if (!c && hi >= 10 && h && lo <= 9) diff = 0x66;
	// carry (the original's second and fifth cases change nothing; kept as comments of the original's structure)
	if (!c && lo <= 9) f &= 0xfe;
	if (!(f & 1) && hi >= 9 && lo >= 10) f |= 1;
	if (!(f & 1) && hi >= 10 && lo <= 9) f |= 1;
	// half carry
	if (!(f & 2) && lo <= 9) f &= 0xef;
	if (!(f & 2) && lo >= 10) f |= 0x10;
	if ((f & 2) && (f & 0x10) && lo >= 6) f &= 0xef;
	if ((f & 2) && (f & 0x10) && lo <= 5) f |= 0x10;
	if (f & 2) diff = (0u - diff) & 0xff;
	const std::uint32_t r = a + diff;
	const std::uint32_t r8 = r & 0xff;
	int bits = 0;
	for (int b = 0; b < 8; b++) bits += (r >> b) & 1;
	z.r8(Env::A) = (std::uint8_t)r;
	Fl(z) = (std::uint8_t)((f & 0x13) | (r8 == 0 ? 0x40 : 0) | (r & 0x80) | (r & 0x08) | (r & 0x20) | (((1 + bits) & 1) << 2));
}

// CB prefix (table RSO2 0x80dcb778); PC still points at the CB byte
void ExecCB(Env& z)
{
	const std::uint8_t op = ReadM1(z, (std::uint16_t)(PC(z) + 1));
	const int n = (op >> 3) & 7;
	if ((op & 7) != 6)
	{
		std::uint8_t& r = z.r8(kR8[op & 7]);
		if (op < 0x40) Shift(z, op >> 3, r);                       // 0x80d0fd1c..0x80d1010c
		else if (op < 0x80) Bit(z, r, n);                          // 0x80d1019c
		else if (op < 0xc0) r = (std::uint8_t)(r & ~(1u << n));    // 0x80d10230
		else r = (std::uint8_t)(r | (1u << n));                    // 0x80d102e0
		z.addCycles(-8);
	}
	else
	{
		const std::uint16_t hl = z.r16(Env::HL);
		std::uint8_t v = Read8(z, hl);
		if (op < 0x40)
		{
			Shift(z, op >> 3, v);  // 0x80d0fd58..0x80d10148
			Write8(z, hl, v);
			z.addCycles(-15);
		}
		else if (op < 0x80)
		{
			Bit(z, v, n);  // 0x80d101e4
			z.addCycles(-12);
		}
		else
		{
			v = op < 0xc0 ? (std::uint8_t)(v & ~(1u << n)) : (std::uint8_t)(v | (1u << n));  // 0x80d1027c / 0x80d10328
			Write8(z, hl, v);
			z.addCycles(-15);
		}
	}
	AddPC(z, 2);
}

// ED prefix (table RSO2 0x80dcadcc, ops 0x40..0xbb; everything else is Uncd)
void ExecED(Env& z)
{
	const std::uint8_t op = ReadM1(z, (std::uint16_t)(PC(z) + 1));
	switch (op)
	{
	case 0x40: case 0x48: case 0x50: case 0x58: case 0x60: case 0x68: case 0x78:  // 0x80d11580 IN r,(C)
	{
		std::uint8_t& r = z.r8(kR8[(op >> 3) & 7]);
		r = ReadIO8(z, z.r16(Env::BC));
		FlagIn(z, r);
		z.addCycles(-12);
		AddPC(z, 2);
		break;
	}
	case 0x41: case 0x49: case 0x51: case 0x59: case 0x61: case 0x69: case 0x79:  // 0x80d115d4 OUT (C),r
		WriteIO8(z, z.r16(Env::BC), z.r8(kR8[(op >> 3) & 7]));
		z.addCycles(-12);
		AddPC(z, 2);
		break;
	case 0x42: case 0x52: case 0x62: case 0x72:  // 0x80d11614 SBC HL,rr
		Sbc16(z, Env::HL, z.r16(kRR[(op >> 4) & 3]));
		z.addCycles(-15);
		AddPC(z, 2);
		break;
	case 0x4a: case 0x5a: case 0x6a: case 0x7a:  // 0x80d117b0 ADC HL,rr
		Adc16(z, Env::HL, z.r16(kRR[(op >> 4) & 3]));
		z.addCycles(-15);
		AddPC(z, 2);
		break;
	case 0x43: case 0x53: case 0x63: case 0x73:  // 0x80d11654 LD (nn),rr
	{
		const std::uint16_t a = Read16(z, (std::uint16_t)(PC(z) + 2));
		Write16(z, a, z.r16(kRR[(op >> 4) & 3]));
		z.addCycles(-20);
		AddPC(z, 4);
		break;
	}
	case 0x4b: case 0x5b: case 0x6b: case 0x7b:  // 0x80d117f0 LD rr,(nn)
	{
		const std::uint16_t a = Read16(z, (std::uint16_t)(PC(z) + 2));
		z.set16(kRR[(op >> 4) & 3], Read16(z, a));
		z.addCycles(-20);
		AddPC(z, 4);
		break;
	}
	case 0x44:  // 0x80d116b4 NEG
	{
		std::uint8_t t = 0;
		Sub8(z, t, z.r8(Env::A), 0);
		z.r8(Env::A) = t;
		z.addCycles(-8);
		AddPC(z, 2);
		break;
	}
	case 0x45:  // 0x80d116f8 RETN
	{
		std::uint16_t st = (std::uint16_t)(z.status() & 0xffee);
		z.setStatus(st);
		if (st & Env::ST_IFF2) z.setStatus((std::uint16_t)(st | Env::ST_IFF1));
		z.set16(Env::PC, Pop(z));
		z.addCycles(-14);
		break;
	}
	case 0x4d:  // 0x80d1184c RETI
		z.set16(Env::PC, Pop(z));
		z.addCycles(-14);
		break;
	case 0x46:  // 0x80d1175c IM 0 (quirk: & 0xf3 also clears the upper status bits)
		z.setStatus((std::uint16_t)(z.status() & 0xf3));
		z.addCycles(-8);
		AddPC(z, 2);
		break;
	case 0x56:  // 0x80d118b8 IM 1
		z.setStatus((std::uint16_t)((z.status() & 0xf3) | 4));
		z.addCycles(-8);
		AddPC(z, 2);
		break;
	case 0x5e:  // 0x80d1191c IM 2
		z.setStatus((std::uint16_t)((z.status() & 0xf3) | 8));
		z.addCycles(-8);
		AddPC(z, 2);
		break;
	case 0x47:  // 0x80d11788 LD I,A
		z.r8(Env::I) = z.r8(Env::A);
		z.addCycles(-9);
		AddPC(z, 2);
		break;
	case 0x4f:  // 0x80d11890 LD R,A
		z.r8(Env::R) = z.r8(Env::A);
		z.addCycles(-9);
		AddPC(z, 2);
		break;
	case 0x57:  // 0x80d118e8 LD A,I
		z.r8(Env::A) = z.r8(Env::I);
		FlagAIR(z);
		z.addCycles(-9);
		AddPC(z, 2);
		break;
	case 0x5f:  // 0x80d1194c LD A,R
		z.r8(Env::A) = (std::uint8_t)(z.r8(Env::R) & 0x7f);
		FlagAIR(z);
		z.addCycles(-9);
		AddPC(z, 2);
		break;
	case 0x67:  // 0x80d11984 RRD
	{
		const std::uint16_t hl = z.r16(Env::HL);
		const std::uint8_t t = Read8(z, hl);
		const std::uint8_t a = z.r8(Env::A);
		z.r8(Env::A) = (std::uint8_t)((a & 0xf0) | (t & 0x0f));
		Write8(z, hl, (std::uint8_t)((t >> 4) | ((a << 4) & 0xf0)));
		FlagRxd(z);
		z.addCycles(-18);
		AddPC(z, 2);
		break;
	}
	case 0x6f:  // 0x80d11a04 RLD
	{
		const std::uint16_t hl = z.r16(Env::HL);
		const std::uint8_t t = Read8(z, hl);
		const std::uint8_t a = z.r8(Env::A);
		z.r8(Env::A) = (std::uint8_t)((a & 0xf0) | (t >> 4));
		Write8(z, hl, (std::uint8_t)((t << 4) | (a & 0x0f)));
		FlagRxd(z);
		z.addCycles(-18);
		AddPC(z, 2);
		break;
	}
	case 0x70:  // 0x80d11a84 IN F,(C): flags only
		FlagIn(z, ReadIO8(z, z.r16(Env::BC)));
		z.addCycles(-12);
		AddPC(z, 2);
		break;
	case 0xa0: BlockLd(z, 1); z.addCycles(-16); AddPC(z, 2); break;  // 0x80d11ac4 LDI
	case 0xa8: BlockLd(z, -1); z.addCycles(-16); AddPC(z, 2); break; // 0x80d11b40 LDD
	case 0xa1: BlockCp(z, 1); AddPC(z, 2); break;                     // 0x80d11aec CPI (quirk: no cycles)
	case 0xa9: BlockCp(z, -1); AddPC(z, 2); break;                    // 0x80d11b68 CPD
	case 0xa2: BlockIn(z, 1); AddPC(z, 2); break;                     // 0x80d11b08 INI
	case 0xaa: BlockIn(z, -1); AddPC(z, 2); break;                    // 0x80d11b84 IND
	case 0xa3: BlockOut(z, 1); AddPC(z, 2); break;                    // 0x80d11b24 OUTI
	case 0xab: BlockOut(z, -1); AddPC(z, 2); break;                   // 0x80d11ba0 OUTD
	case 0xb0: case 0xb8:  // 0x80d11bbc LDIR / 0x80d11d04 LDDR
		BlockLd(z, op == 0xb0 ? 1 : -1);
		z.addCycles(-16);
		if (Fl(z) & 0x04)
		{
			z.addCycles(-5);
			AddPC(z, -2);
		}
		AddPC(z, 2);
		break;
	case 0xb1: case 0xb9:  // 0x80d11c0c CPIR / 0x80d11d54 CPDR
		BlockCp(z, op == 0xb1 ? 1 : -1);
		z.addCycles(-16);
		if ((Fl(z) & 0x04) && !(Fl(z) & 0x40))
		{
			z.addCycles(-5);
			AddPC(z, -2);
		}
		AddPC(z, 2);
		break;
	case 0xb2: case 0xba:  // 0x80d11c64 INIR / 0x80d11dac INDR
		BlockIn(z, op == 0xb2 ? 1 : -1);
		z.addCycles(-16);
		if (z.r8(Env::B) != 0)
		{
			z.addCycles(-5);
			AddPC(z, -2);
		}
		AddPC(z, 2);
		break;
	case 0xb3: case 0xbb:  // 0x80d11cb4 OTIR / 0x80d11dfc OTDR (quirk: loops here, PC drifts back 2 per extra pass)
		for (;;)
		{
			BlockOut(z, op == 0xb3 ? 1 : -1);
			z.addCycles(-16);
			if (z.r8(Env::B) == 0) break;
			z.addCycles(-5);
			AddPC(z, -2);
		}
		AddPC(z, 2);
		break;
	default:  // 0x80d11e4c
		Uncd(z);
		break;
	}
}

// DD / FD prefix (tables RSO2 0x80dcb3a0 / 0x80dca9f4: ops 0x04..0xf9) with DD CB / FD CB (0x80dcafbc / 0x80dca610: ops 0x06..0xfe)
void ExecXY(Env& z, int X, const std::uint8_t* r8x, bool fd)
{
	const std::uint8_t op = ReadM1(z, (std::uint16_t)(PC(z) + 1));
	// (X+d) with d at PC+2
	auto ea = [&]() { return (std::uint16_t)(z.r16(X) + (std::int8_t)Read8(z, (std::uint16_t)(PC(z) + 2))); };
	switch (op)
	{
	case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x3c:  // INC r
		Inc8(z, z.r8(r8x[(op >> 3) & 7]));
		z.addCycles(-8);
		AddPC(z, 2);
		return;
	case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x3d:  // DEC r
		Dec8(z, z.r8(r8x[(op >> 3) & 7]));
		z.addCycles(-8);
		AddPC(z, 2);
		return;
	case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x3e:  // LD r,n
	{
		const std::uint8_t v = Read8(z, (std::uint16_t)(PC(z) + 2));
		z.r8(r8x[(op >> 3) & 7]) = v;
		z.addCycles(-11);
		AddPC(z, 3);
		return;
	}
	case 0x09: case 0x19: case 0x29: case 0x39:  // ADD X,rr (quirk: FD uses the DD table too)
		Add16(z, X, z.r16(kRRIX[(op >> 4) & 3]));
		z.addCycles(-15);
		AddPC(z, 2);
		return;
	case 0x21:  // LD X,nn
		z.set16(X, Read16(z, (std::uint16_t)(PC(z) + 2)));
		z.addCycles(-14);
		AddPC(z, 4);
		return;
	case 0x22:  // LD (nn),X
		Write16(z, Read16(z, (std::uint16_t)(PC(z) + 2)), z.r16(X));
		z.addCycles(-20);
		AddPC(z, 4);
		return;
	case 0x23:
		z.set16(X, (std::uint16_t)(z.r16(X) + 1));
		z.addCycles(-10);
		AddPC(z, 2);
		return;
	case 0x2a:  // LD X,(nn)
	{
		const std::uint16_t a = Read16(z, (std::uint16_t)(PC(z) + 2));
		z.set16(X, Read16(z, a));
		z.addCycles(-20);
		AddPC(z, 4);
		return;
	}
	case 0x2b:
		z.set16(X, (std::uint16_t)(z.r16(X) - 1));
		z.addCycles(-10);
		AddPC(z, 2);
		return;
	case 0x34:  // INC (X+d)
	{
		const std::uint16_t a = ea();
		std::uint8_t v = Read8(z, a);
		Inc8(z, v);
		Write8(z, a, v);
		z.addCycles(-23);
		AddPC(z, 3);
		return;
	}
	case 0x35:  // DEC (X+d) (FD 0x80d12158 reads d again for the write address)
	{
		const std::uint16_t a = ea();
		std::uint8_t v = Read8(z, a);
		Dec8(z, v);
		Write8(z, fd ? ea() : a, v);
		z.addCycles(-23);
		AddPC(z, 3);
		return;
	}
	case 0x36:  // LD (X+d),n
	{
		const std::uint8_t v = Read8(z, (std::uint16_t)(PC(z) + 3));
		Write8(z, ea(), v);
		z.addCycles(-19);
		AddPC(z, 4);
		return;
	}
	case 0x46: case 0x4e: case 0x56: case 0x5e: case 0x66: case 0x6e: case 0x7e:  // LD r,(X+d) (plain register table)
	{
		const std::uint8_t v = Read8(z, ea());
		z.r8(kR8[(op >> 3) & 7]) = v;
		z.addCycles(-19);
		AddPC(z, 3);
		return;
	}
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:  // LD (X+d),r
	{
		const std::uint16_t a = ea();
		Write8(z, a, z.r8(kR8[op & 7]));
		z.addCycles(-19);
		AddPC(z, 3);
		return;
	}
	case 0x86: case 0x8e: case 0x96: case 0x9e: case 0xa6: case 0xae: case 0xb6: case 0xbe:  // ALU A,(X+d)
		Alu(z, op, Read8(z, ea()));
		z.addCycles(-19);
		AddPC(z, 3);
		return;
	case 0xe1:  // POP X
		z.set16(X, Pop(z));
		z.addCycles(-14);
		AddPC(z, 2);
		return;
	case 0xe3:  // EX (SP),X
	{
		const std::uint16_t t = Read16(z, z.r16(Env::SP));
		Write16(z, z.r16(Env::SP), z.r16(X));
		z.set16(X, t);
		z.addCycles(-23);
		AddPC(z, 2);
		return;
	}
	case 0xe5:  // PUSH X
		Push(z, z.r16(X));
		z.addCycles(-15);
		AddPC(z, 2);
		return;
	case 0xe9:  // JP (X)
		z.set16(Env::PC, z.r16(X));
		z.addCycles(-8);
		return;
	case 0xf9:
		z.set16(Env::SP, z.r16(X));
		z.addCycles(-10);
		AddPC(z, 2);
		return;
	case 0xcb:
	{
		// the sub-opcode is read with ReadMemory8 (not an M1 fetch) at PC+3; only the (X+d) forms exist
		const std::uint8_t op3 = Read8(z, (std::uint16_t)(PC(z) + 3));
		if (op3 < 0x06 || (op3 & 7) != 6)
		{
			Uncd(z);
			return;
		}
		const int n = (op3 >> 3) & 7;
		const std::uint16_t a = ea();
		std::uint8_t v = Read8(z, a);
		if (op3 < 0x40)
		{
			Shift(z, op3 >> 3, v);
			Write8(z, a, v);
			z.addCycles(-23);
		}
		else if (op3 < 0x80)
		{
			Bit(z, v, n);
			z.addCycles(-20);
		}
		else
		{
			v = op3 < 0xc0 ? (std::uint8_t)(v & ~(1u << n)) : (std::uint8_t)(v | (1u << n));
			Write8(z, a, v);
			z.addCycles(-23);
		}
		AddPC(z, 4);
		return;
	}
	default:
		break;
	}
	// LD r,r' and ALU A,r with the prefix's register table (IXh / IXl for H / L)
	if (op >= 0x40 && op < 0x80 && op != 0x76 && (op & 7) != 6 && (op & 0xf8) != 0x70)
	{
		z.r8(r8x[(op >> 3) & 7]) = z.r8(r8x[op & 7]);
		z.addCycles(-8);
		AddPC(z, 2);
		return;
	}
	if (op >= 0x80 && op < 0xc0 && (op & 7) != 6)
	{
		Alu(z, op, z.r8(r8x[op & 7]));
		z.addCycles(-8);
		AddPC(z, 2);
		return;
	}
	Uncd(z);  // 0x80d11530 / 0x80d13024
}

// the unnamed static dispatcher (RSO2 0x80d0d7f8), main table 0x80dcbb78; PC still points at the opcode
void Execute(Env& z, std::uint8_t op)
{
	switch (op)
	{
	case 0x00:
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0x01: case 0x11: case 0x21: case 0x31:  // LD rr,nn
		z.set16(kRR[(op >> 4) & 3], Read16(z, (std::uint16_t)(PC(z) + 1)));
		z.addCycles(-10);
		AddPC(z, 3);
		return;
	case 0x02:
		Write8(z, z.r16(Env::BC), z.r8(Env::A));
		z.addCycles(-7);
		AddPC(z, 1);
		return;
	case 0x12:
		Write8(z, z.r16(Env::DE), z.r8(Env::A));
		z.addCycles(-7);
		AddPC(z, 1);
		return;
	case 0x0a:
		z.r8(Env::A) = Read8(z, z.r16(Env::BC));
		z.addCycles(-7);
		AddPC(z, 1);
		return;
	case 0x1a:
		z.r8(Env::A) = Read8(z, z.r16(Env::DE));
		z.addCycles(-7);
		AddPC(z, 1);
		return;
	case 0x03: case 0x13: case 0x23: case 0x33:
	{
		const int rr = kRR[(op >> 4) & 3];
		z.set16(rr, (std::uint16_t)(z.r16(rr) + 1));
		z.addCycles(-6);
		AddPC(z, 1);
		return;
	}
	case 0x0b: case 0x1b: case 0x2b: case 0x3b:
	{
		const int rr = kRR[(op >> 4) & 3];
		z.set16(rr, (std::uint16_t)(z.r16(rr) - 1));
		z.addCycles(-6);
		AddPC(z, 1);
		return;
	}
	case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x3c:
		Inc8(z, z.r8(kR8[(op >> 3) & 7]));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x3d:
		Dec8(z, z.r8(kR8[(op >> 3) & 7]));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x3e:  // LD r,n
	{
		const std::uint8_t v = Read8(z, (std::uint16_t)(PC(z) + 1));
		z.r8(kR8[(op >> 3) & 7]) = v;
		z.addCycles(-7);
		AddPC(z, 2);
		return;
	}
	case 0x07:  // RLCA 0x80d0d9d4
	{
		const std::uint8_t a = z.r8(Env::A);
		const std::uint8_t r = (std::uint8_t)((a << 1) | (a >> 7));
		z.r8(Env::A) = r;
		Fl(z) = (std::uint8_t)((Fl(z) & 0xc4) | (r & 0x29));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	}
	case 0x0f:  // RRCA 0x80d0daf8
	{
		const std::uint8_t a = z.r8(Env::A);
		const std::uint8_t r = (std::uint8_t)((a >> 1) | (a << 7));
		z.r8(Env::A) = r;
		Fl(z) = (std::uint8_t)((Fl(z) & 0xc4) | (a & 1) | (r & 0x28));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	}
	case 0x17:  // RLA 0x80d0dbf0
	{
		const std::uint8_t a = z.r8(Env::A);
		const std::uint8_t r = (std::uint8_t)((a << 1) | (Fl(z) & 1));
		z.r8(Env::A) = r;
		Fl(z) = (std::uint8_t)((Fl(z) & 0xc4) | (a >> 7) | (r & 0x28));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	}
	case 0x1f:  // RRA 0x80d0dcc8
	{
		const std::uint8_t a = z.r8(Env::A);
		const std::uint8_t r = (std::uint8_t)((a >> 1) | ((Fl(z) & 1) << 7));
		z.r8(Env::A) = r;
		Fl(z) = (std::uint8_t)((Fl(z) & 0xc4) | (a & 1) | (r & 0x28));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	}
	case 0x08:
		Exchange(z, Env::AF, Env::AF2);
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0x09: case 0x19: case 0x29: case 0x39:
		Add16(z, Env::HL, z.r16(kRR[(op >> 4) & 3]));
		z.addCycles(-11);
		AddPC(z, 1);
		return;
	case 0x10:  // DJNZ 0x80d0db54
	{
		z.addCycles(-8);
		const std::uint8_t b = (std::uint8_t)(z.r8(Env::B) - 1);
		z.r8(Env::B) = b;
		if (b != 0)
		{
			AddPC(z, (std::int8_t)Read8(z, (std::uint16_t)(PC(z) + 1)));
			z.addCycles(-5);
		}
		AddPC(z, 2);
		return;
	}
	case 0x18:  // JR
	{
		const std::int8_t d = (std::int8_t)Read8(z, (std::uint16_t)(PC(z) + 1));
		z.addCycles(-12);
		AddPC(z, d + 2);
		return;
	}
	case 0x20: case 0x28: case 0x30: case 0x38:  // JR cc (cc 0..3)
		if (Cond(z, op - 0x20))
		{
			AddPC(z, (std::int8_t)Read8(z, (std::uint16_t)(PC(z) + 1)));
			z.addCycles(-5);
		}
		z.addCycles(-7);
		AddPC(z, 2);
		return;
	case 0x22:
	{
		const std::uint16_t a = Read16(z, (std::uint16_t)(PC(z) + 1));
		Write16(z, a, z.r16(Env::HL));
		z.addCycles(-16);
		AddPC(z, 3);
		return;
	}
	case 0x2a:
	{
		const std::uint16_t a = Read16(z, (std::uint16_t)(PC(z) + 1));
		z.set16(Env::HL, Read16(z, a));
		z.addCycles(-16);
		AddPC(z, 3);
		return;
	}
	case 0x27:
		Daa(z);
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0x2f:  // CPL 0x80d0e234
	{
		const std::uint8_t a = (std::uint8_t)(z.r8(Env::A) ^ 0xff);
		z.r8(Env::A) = a;
		Fl(z) = (std::uint8_t)((Fl(z) & 0xd7) | 0x12 | (a & 0x28));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	}
	case 0x32:
	{
		const std::uint16_t a = Read16(z, (std::uint16_t)(PC(z) + 1));
		Write8(z, a, z.r8(Env::A));
		z.addCycles(-13);
		AddPC(z, 3);
		return;
	}
	case 0x3a:
	{
		const std::uint16_t a = Read16(z, (std::uint16_t)(PC(z) + 1));
		z.r8(Env::A) = Read8(z, a);
		z.addCycles(-13);
		AddPC(z, 3);
		return;
	}
	case 0x34: case 0x35:
	{
		const std::uint16_t hl = z.r16(Env::HL);
		std::uint8_t v = Read8(z, hl);
		if (op == 0x34) Inc8(z, v);
		else Dec8(z, v);
		Write8(z, hl, v);
		z.addCycles(-11);
		AddPC(z, 1);
		return;
	}
	case 0x36:
	{
		const std::uint8_t v = Read8(z, (std::uint16_t)(PC(z) + 1));
		Write8(z, z.r16(Env::HL), v);
		z.addCycles(-10);
		AddPC(z, 2);
		return;
	}
	case 0x37:  // SCF
		Fl(z) = (std::uint8_t)((Fl(z) & 0xc4) | (z.r8(Env::A) & 0x28) | 1);
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0x3f:  // CCF 0x80d0e50c
	{
		const std::uint8_t f = (std::uint8_t)(Fl(z) & 0xc5);
		Fl(z) = (std::uint8_t)((f | (z.r8(Env::A) & 0x28) | ((f << 4) & 0x10)) ^ 1);
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	}
	case 0x76:  // HALT 0x80d0e604
		z.setStatus((std::uint16_t)(z.status() | Env::ST_HALT));
		z.setCycles(0);
		AddPC(z, 1);
		return;
	case 0xc0: case 0xc8: case 0xd0: case 0xd8: case 0xe0: case 0xe8: case 0xf0: case 0xf8:  // RET cc
		if (Cond(z, op))
		{
			z.set16(Env::PC, (std::uint16_t)(Pop(z) - 1));
			z.addCycles(-6);
		}
		z.addCycles(-5);
		AddPC(z, 1);
		return;
	case 0xc1: case 0xd1: case 0xe1: case 0xf1:
		z.set16(kRRPush[(op >> 4) & 3], Pop(z));
		z.addCycles(-10);
		AddPC(z, 1);
		return;
	case 0xc5: case 0xd5: case 0xe5: case 0xf5:
		Push(z, z.r16(kRRPush[(op >> 4) & 3]));
		z.addCycles(-11);
		AddPC(z, 1);
		return;
	case 0xc2: case 0xca: case 0xd2: case 0xda: case 0xe2: case 0xea: case 0xf2: case 0xfa:  // JP cc
		if (Cond(z, op)) z.set16(Env::PC, (std::uint16_t)(Read16(z, (std::uint16_t)(PC(z) + 1)) - 3));
		z.addCycles(-10);
		AddPC(z, 3);
		return;
	case 0xc3:
		z.set16(Env::PC, Read16(z, (std::uint16_t)(PC(z) + 1)));
		z.addCycles(-10);
		return;
	case 0xc4: case 0xcc: case 0xd4: case 0xdc: case 0xe4: case 0xec: case 0xf4: case 0xfc:  // CALL cc
		if (Cond(z, op))
		{
			Push(z, (std::uint16_t)(PC(z) + 3));
			z.set16(Env::PC, (std::uint16_t)(Read16(z, (std::uint16_t)(PC(z) + 1)) - 3));
			z.addCycles(-7);
		}
		z.addCycles(-10);
		AddPC(z, 3);
		return;
	case 0xcd:
		Push(z, (std::uint16_t)(PC(z) + 3));
		z.set16(Env::PC, Read16(z, (std::uint16_t)(PC(z) + 1)));
		z.addCycles(-17);
		return;
	case 0xc6: case 0xce: case 0xd6: case 0xde: case 0xe6: case 0xee: case 0xf6: case 0xfe:  // ALU A,n
		Alu(z, op, Read8(z, (std::uint16_t)(PC(z) + 1)));
		z.addCycles(-7);
		AddPC(z, 2);
		return;
	case 0xc7:  // 0x80d0ec90: quirk: RST 0 is the native hook; PC stays (the hook sets it)
		z.ExtFunc();
		z.addCycles(-11);
		return;
	case 0xcf: case 0xd7: case 0xdf: case 0xe7: case 0xef: case 0xf7: case 0xff:  // RST n
		Push(z, (std::uint16_t)(PC(z) + 1));
		z.set16(Env::PC, (std::uint16_t)(op & 0x38));
		z.addCycles(-11);
		return;
	case 0xc9:
		z.set16(Env::PC, Pop(z));
		z.addCycles(-10);
		return;
	case 0xd3:  // OUT (n),A
	{
		const std::uint8_t n = Read8(z, (std::uint16_t)(PC(z) + 1));
		WriteIO8(z, (std::uint16_t)(z.r8(Env::A) << 8 | n), z.r8(Env::A));
		z.addCycles(-11);
		AddPC(z, 2);
		return;
	}
	case 0xdb:  // IN A,(n)
	{
		const std::uint8_t n = Read8(z, (std::uint16_t)(PC(z) + 1));
		z.r8(Env::A) = ReadIO8(z, (std::uint16_t)(z.r8(Env::A) << 8 | n));
		z.addCycles(-11);
		AddPC(z, 2);
		return;
	}
	case 0xd9:  // EXX
		Exchange(z, Env::HL, Env::HL2);
		Exchange(z, Env::DE, Env::DE2);
		Exchange(z, Env::BC, Env::BC2);
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0xe3:  // EX (SP),HL
	{
		const std::uint16_t t = Read16(z, z.r16(Env::SP));
		Write16(z, z.r16(Env::SP), z.r16(Env::HL));
		z.set16(Env::HL, t);
		z.addCycles(-19);
		AddPC(z, 1);
		return;
	}
	case 0xe9:
		z.set16(Env::PC, z.r16(Env::HL));
		z.addCycles(-4);
		return;
	case 0xeb:
		Exchange(z, Env::DE, Env::HL);
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0xf3:  // DI (quirk: & 0xfc clears the upper status bits too)
		z.setStatus((std::uint16_t)(z.status() & 0xfc));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0xfb:  // EI (no one-instruction delay)
		z.setStatus((std::uint16_t)(z.status() | 3));
		z.addCycles(-4);
		AddPC(z, 1);
		return;
	case 0xf9:
		z.set16(Env::SP, z.r16(Env::HL));
		z.addCycles(-6);
		AddPC(z, 1);
		return;
	case 0xcb: ExecCB(z); return;
	case 0xed: ExecED(z); return;
	case 0xdd: ExecXY(z, Env::IX, kR8IX, false); return;
	case 0xfd: ExecXY(z, Env::IY, kR8IY, true); return;
	default:
		break;
	}
	if (op >= 0x40 && op < 0x80)
	{
		if ((op & 7) == 6)  // LD r,(HL)
		{
			z.r8(kR8[(op >> 3) & 7]) = Read8(z, z.r16(Env::HL));
			z.addCycles(-7);
		}
		else if ((op & 0xf8) == 0x70)  // LD (HL),r
		{
			Write8(z, z.r16(Env::HL), z.r8(kR8[op & 7]));
			z.addCycles(-7);
		}
		else
		{
			z.r8(kR8[(op >> 3) & 7]) = z.r8(kR8[op & 7]);
			z.addCycles(-4);
		}
		AddPC(z, 1);
		return;
	}
	// 0x80..0xbf: ALU A,r / A,(HL)
	if ((op & 7) == 6)
	{
		Alu(z, op, Read8(z, z.r16(Env::HL)));
		z.addCycles(-7);
	}
	else
	{
		Alu(z, op, z.r8(kR8[op & 7]));
		z.addCycles(-4);
	}
	AddPC(z, 1);
}

// interrupt push: Write8 of the low byte at SP-2, then the high byte
void PushPC(Env& z)
{
	const std::uint16_t sp = (std::uint16_t)(z.r16(Env::SP) - 2);
	z.set16(Env::SP, sp);
	const std::uint16_t pc = z.r16(Env::PC);
	z.WriteMemory8(sp, (std::uint8_t)pc);
	z.WriteMemory8((std::uint16_t)(z.r16(Env::SP) + 1), (std::uint8_t)(pc >> 8));
}

// RSO2 0x80d0d358 INTgo
void INTgo(Env& z)
{
	const std::uint16_t st = (std::uint16_t)(z.status() & 0xff5c);
	z.setStatus(st);
	const int im = (st >> 2) & 3;
	if (im == 1)
	{
		PushPC(z);
		z.addCycles(-13);
		z.set16(Env::PC, 0x38);
	}
	else if (im == 2)
	{
		PushPC(z);
		z.addCycles(-19);
		const std::uint16_t vec = (std::uint16_t)((z.r8(Env::I) << 8) | (z.r16(Env::BUS) & 0xfe));
		const std::uint8_t hi = z.ReadMemory8((std::uint16_t)(vec + 1));
		const std::uint8_t lo = z.ReadMemory8(vec);
		z.set16(Env::PC, (std::uint16_t)(hi << 8 | lo));
	}
	else
	{
		PushPC(z);
		z.addCycles(-11);
		z.set16(Env::PC, z.r16(Env::BUS));  // IM 0 (and 3): the bus word is the target
	}
}

// RSO2 0x80d0d70c NMIgo (quirk: tests IFF1 after clearing it, so IFF2 never gets set here)
void NMIgo(Env& z)
{
	std::uint16_t st = (std::uint16_t)(z.status() & 0xff9e);
	z.setStatus(st);
	if (st & Env::ST_IFF1) z.setStatus((std::uint16_t)(st | Env::ST_IFF2));
	z.setStatus((std::uint16_t)((z.status() & 0xfffe) | Env::ST_NMI_ACTIVE));
	PushPC(z);
	z.addCycles(-16);
	z.set16(Env::PC, 0x66);
}

// RSO2 0x80d0d7d0 HALTgo
void HALTgo(Env& z) { z.setCycles(0); }

// the common part of Z80_Init / Z80_Reset: everything zero, status keeps only RESET (and the budget is cleared while it is set)
void ClearRegs(Env& z)
{
	const std::uint16_t st = (std::uint16_t)(z.status() & Env::ST_RESET);
	for (int o = 0x20; o < 0x3a; o++) z.m[o] = 0;
	z.setStatus(st);
	if (st != 0) z.setCycles(0);
}
}

const std::uint8_t* Z80_FlagTable(int which)
{
	switch (which)
	{
	case 0: return T().sz;
	case 1: return T().szp;
	case 2: return T().inc;
	default: return T().dec;
	}
}

// RSO2 0x80d0d070
int Z80_SetFlag(Z80_env* z, CPUFLAG flag, void* value)
{
	const bool on = value != nullptr;
	auto bit = [&](std::uint16_t b) { z->setStatus((std::uint16_t)(on ? (z->status() | b) : (z->status() & ~b))); };
	switch (flag)
	{
	case CPUFLAG_DISABLE: bit(Z80_env::ST_DISABLE); return 0;
	case CPUFLAG_IRQ: bit(Z80_env::ST_IRQ); return 0;
	case CPUFLAG_NMI: bit(Z80_env::ST_NMI); return 0;
	case CPUFLAG_RESET: bit(Z80_env::ST_RESET); return 0;
	case CPUFLAG_HALTWAIT: bit(Z80_env::ST_HALTWAIT); return 0;
	case CPUFLAG_SETCYCLE: z->setCycles((std::int32_t)(std::intptr_t)value); return 0;
	case CPUFLAG_GETCYCLE: *(int*)value = z->cycles(); return 0;
	default: return 1;
	}
}

// RSO2 0x80d0d198 (also builds the register tables, which are constants here)
int Z80_Init(Z80_env* z)
{
	ClearRegs(*z);
	return 0;
}

// RSO2 0x80d0d300
int Z80_Reset(Z80_env* z)
{
	ClearRegs(*z);
	return 0;
}

// RSO2 0x80d0d5fc
int Z80_Run(Z80_env* z)
{
	if (z->status() & Z80_env::ST_DISABLE)
	{
		z->setCycles(0);
		return 0;
	}
	if (z->status() & Z80_env::ST_RESET) Z80_Reset(z);
	while (z->cycles() > 0)
	{
		const std::uint16_t st = z->status();
		if (st & Z80_env::ST_NMI)
		{
			NMIgo(*z);
		}
		else if ((st & Z80_env::ST_IFF1) && (st & Z80_env::ST_IRQ))
		{
			INTgo(*z);
		}
		else if (st & Z80_env::ST_HALT)
		{
			HALTgo(*z);
			return (st & Z80_env::ST_HALTWAIT) ? 2 : 0;
		}
		const std::uint8_t op = ReadM1(*z, z->r16(Z80_env::PC));
		z->r8(Z80_env::R) = (std::uint8_t)(z->r8(Z80_env::R) + 1);
		Execute(*z, op);
		if (z->status() & Z80_env::ST_STOP) return 4;
	}
	return 0;
}
}
