#include <string>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include "mos6510.h"
#include <stdio.h>

namespace MOS6510 {
void Cpu::execute()
{
    uint16_t programCounter = m_programCounter;
    uint8_t opcode = m_sramPtr[programCounter];

    std::string ocs = "";
    switch (opcode) {
        case ADC_abs: ocs = "ADC_abs"; adc(AddrMode::ABS);                break;
        case ADC_abx: ocs = "ADC_abx"; adc(AddrMode::ABX);                break;
        case ADC_aby: ocs = "ADC_aby"; adc(AddrMode::ABY);                break;
        case ADC_imm: ocs = "ADC_imm"; adc(AddrMode::IMM);                break;
        case ADC_izx: ocs = "ADC_izx"; adc(AddrMode::IZX);                break;
        case ADC_izy: ocs = "ADC_izy"; adc(AddrMode::IZY);                break;
        case ADC_zp:  ocs = "ADC_zp";  adc(AddrMode::ZP);                 break;
        case ADC_zpx: ocs = "ADC_zpx"; adc(AddrMode::ZPX);                break;
        case AHX_aby: ocs = "AHX_aby"; break;
        case AHX_izy: ocs = "AHX_izy"; break;
        case ALR_imm: ocs = "ALR_imm"; break;
        case ANC_im2: ocs = "ANC_im2"; break;
        case ANC_imm: ocs = "ANC_imm"; break;
        case AND_abs: ocs = "AND_abs"; andi(AddrMode::ABS);               break;
        case AND_abx: ocs = "AND_abx"; andi(AddrMode::ABX);               break;
        case AND_aby: ocs = "AND_aby"; andi(AddrMode::ABY);               break;
        case AND_imm: ocs = "AND_imm"; andi(AddrMode::IMM);               break;
        case AND_izx: ocs = "AND_izx"; andi(AddrMode::IZX);               break;
        case AND_izy: ocs = "AND_izy"; andi(AddrMode::IZY);               break;
        case AND_zp:  ocs = "AND_zp";  andi(AddrMode::ZP);                break;
        case AND_zpx: ocs = "AND_zpx"; andi(AddrMode::ZPX);               break;
        case ARR_imm: ocs = "ARR_imm"; break;
        case ASL: ocs = "ASL"; break;
        case ASL_abs: ocs = "ASL_abs"; break;
        case ASL_abx: ocs = "ASL_abx"; break;
        case ASL_zp: ocs = "ASL_zp"; break;
        case ASL_zpx: ocs = "ASL_zpx"; break;
        case AXS_imm: ocs = "AXS_imm"; break;
        case BCC_rel: ocs = "BCC_rel"; br(m_status.bits.carryFlag, 0);    break;
        case BCS_rel: ocs = "BCS_rel"; br(m_status.bits.carryFlag, 1);    break;
        case BEQ_rel: ocs = "BEQ_rel"; br(m_status.bits.zeroFlag, 1);     break;
        case BIT_abs: ocs = "BIT_abs"; break;
        case BIT_zp: ocs = "BIT_zp"; break;
        case BMI_rel: ocs = "BMI_rel"; br(m_status.bits.negativeFlag, 1); break;
        case BNE_rel: ocs = "BNE_rel"; br(m_status.bits.zeroFlag, 0);     break;
        case BPL_rel: ocs = "BPL_rel"; br(m_status.bits.negativeFlag, 0); break;
        case BRK:     ocs = "BRK"; break;
        case BVC_rel: ocs = "BVC_rel"; br(m_status.bits.overflowFlag, 0); break;
        case BVS_rel: ocs = "BVS_rel"; br(m_status.bits.overflowFlag, 1); break;
        case CLC:     ocs = "CLC";     clc();                             break;
        case CLD:     ocs = "CLD";     cld();                             break;
        case CLI:     ocs = "CLI";     cli();                             break;
        case CLV:     ocs = "CLV";     clv();                             break;
        case CMP_abs: ocs = "CMP_abs"; cmp(m_accumulator, AddrMode::ABS); break;
        case CMP_abx: ocs = "CMP_abx"; cmp(m_accumulator, AddrMode::ABX); break;
        case CMP_aby: ocs = "CMP_aby"; cmp(m_accumulator, AddrMode::ABY); break;
        case CMP_imm: ocs = "CMP_imm"; cmp(m_accumulator, AddrMode::IMM); break;
        case CMP_izx: ocs = "CMP_izx"; cmp(m_accumulator, AddrMode::IZX); break;
        case CMP_izy: ocs = "CMP_izy"; cmp(m_accumulator, AddrMode::IZY); break;
        case CMP_zp:  ocs = "CMP_zp";  cmp(m_accumulator, AddrMode::ZP);  break;
        case CMP_zpx: ocs = "CMP_zpx"; cmp(m_accumulator, AddrMode::ZPX); break;
        case CPX_abs: ocs = "CPX_abs"; cmp(m_xIndex, AddrMode::ABS);      break;
        case CPX_imm: ocs = "CPX_imm"; cmp(m_xIndex, AddrMode::IMM);      break;
        case CPX_zp:  ocs = "CPX_zp";  cmp(m_xIndex, AddrMode::ZP);       break;
        case CPY_abs: ocs = "CPY_abs"; cmp(m_yIndex, AddrMode::ABS);      break;
        case CPY_imm: ocs = "CPY_imm"; cmp(m_yIndex, AddrMode::IMM);      break;
        case CPY_zp:  ocs = "CPY_zp";  cmp(m_yIndex, AddrMode::ZP);       break;
        case DCP_abs: ocs = "DCP_abs"; break;
        case DCP_abx: ocs = "DCP_abx"; break;
        case DCP_aby: ocs = "DCP_aby"; break;
        case DCP_izx: ocs = "DCP_izx"; assert(0); break;
        case DCP_izy: ocs = "DCP_izy"; break;
        case DCP_zp:  ocs = "DCP_zp";  break;
        case DCP_zpx: ocs = "DCP_zpx"; break;
        case DEC_abs: ocs = "DEC_abs"; dec(AddrMode::ABS);                break;
        case DEC_abx: ocs = "DEC_abx"; dec(AddrMode::ABX);                break;
        case DEC_zp:  ocs = "DEC_zp";  dec(AddrMode::ZP);                 break;
        case DEC_zpx: ocs = "DEC_zpx"; dec(AddrMode::ZPX);                break;
        case DEX:     ocs = "DEX";     der(m_xIndex);                     break;
        case DEY:     ocs = "DEY";     der(m_yIndex);                     break;
        case EOR_abs: ocs = "EOR_abs"; eor(AddrMode::ABS);                break;
        case EOR_abx: ocs = "EOR_abx"; eor(AddrMode::ABX);                break;
        case EOR_aby: ocs = "EOR_aby"; eor(AddrMode::ABY);                break;
        case EOR_imm: ocs = "EOR_imm"; eor(AddrMode::IMM);                break;
        case EOR_izx: ocs = "EOR_izx"; eor(AddrMode::IZX);                break;
        case EOR_izy: ocs = "EOR_izy"; eor(AddrMode::IZY);                break;
        case EOR_zp:  ocs = "EOR_zp";  eor(AddrMode::ZP);                 break;
        case EOR_zpx: ocs = "EOR_zpx"; eor(AddrMode::ZPX);                break;
        case INC_abs: ocs = "INC_abs"; inc(AddrMode::ABS);                break;
        case INC_abx: ocs = "INC_abx"; inc(AddrMode::ABX);                break;
        case INC_zp:  ocs = "INC_zp";  inc(AddrMode::ZP);                 break;
        case INC_zpx: ocs = "INC_zpx"; inc(AddrMode::ZPX);                break;
        case INX:     ocs = "INX";     inr(m_xIndex);                     break;
        case INY:     ocs = "INY";     inr(m_yIndex);                     break;
        case ISC_abs: ocs = "ISC_abs"; break;
        case ISC_abx: ocs = "ISC_abx"; break;
        case ISC_aby: ocs = "ISC_aby"; break;
        case ISC_izx: ocs = "ISC_izx"; break;
        case ISC_izy: ocs = "ISC_izy"; break;
        case ISC_zp: ocs = "ISC_zp"; break;
        case ISC_zpx: ocs = "ISC_zpx"; break;
        case JMP_abs: ocs = "JMP_abs"; jmp(AddrMode::ABS);                break;
        case JMP_ind: ocs = "JMP_ind"; jmp(AddrMode::IND);                break;
        case JSR: ocs = "JSR"; jsr(); break;
        case KIL0: ocs = "KIL0"; break;
        case KIL1: ocs = "KIL1"; break;
        case KIL2: ocs = "KIL2"; break;
        case KIL3: ocs = "KIL3"; break;
        case KIL4: ocs = "KIL4"; break;
        case KIL5: ocs = "KIL5"; break;
        case KIL6: ocs = "KIL6"; break;
        case KIL7: ocs = "KIL7"; break;
        case KIL9: ocs = "KIL9"; break;
        case KILB: ocs = "KILB"; break;
        case KILD: ocs = "KILD"; break;
        case KILF: ocs = "KILF"; break;
        case LAS_aby: ocs = "LAS_aby";                                    break;
        case LAX_abs: ocs = "LAX_abs";                                    break;
        case LAX_aby: ocs = "LAX_aby";                                    break;
        case LAX_imm: ocs = "LAX_imm";                                    break;
        case LAX_izx: ocs = "LAX_izx";                                    break;
        case LAX_izy: ocs = "LAX_izy";                                    break;
        case LAX_zp:  ocs = "LAX_zp";                                     break;
        case LAX_zpy: ocs = "LAX_zpy";                                    break;
        case LDA_abs: ocs = "LDA_abs"; ldr(m_accumulator, AddrMode::ABS); break;
        case LDA_abx: ocs = "LDA_abx"; ldr(m_accumulator, AddrMode::ABX); break;
        case LDA_aby: ocs = "LDA_aby"; ldr(m_accumulator, AddrMode::ABY); break;
        case LDA_imm: ocs = "LDA_imm"; ldr(m_accumulator, AddrMode::IMM); break;
        case LDA_izx: ocs = "LDA_izx"; ldr(m_accumulator, AddrMode::IZX); break;
        case LDA_izy: ocs = "LDA_izy"; ldr(m_accumulator, AddrMode::IZY); break;
        case LDA_zp:  ocs = "LDA_zp";  ldr(m_accumulator, AddrMode::ZP);  break;
        case LDA_zpx: ocs = "LDA_zpx"; ldr(m_accumulator, AddrMode::ZPX); break;
        case LDX_abs: ocs = "LDX_abs"; ldr(m_xIndex, AddrMode::ABS);      break;
        case LDX_aby: ocs = "LDX_aby"; ldr(m_xIndex, AddrMode::ABY);      break;
        case LDX_imm: ocs = "LDX_imm"; ldr(m_xIndex, AddrMode::IMM);      break;
        case LDX_zp:  ocs = "LDX_zp";  ldr(m_xIndex, AddrMode::ZP);       break;
        case LDX_zpy: ocs = "LDX_zpy"; ldr(m_xIndex, AddrMode::ZPY);      break;
        case LDY_abs: ocs = "LDY_abs"; ldr(m_yIndex, AddrMode::ABS);      break;
        case LDY_abx: ocs = "LDY_abx"; ldr(m_yIndex, AddrMode::ABX);      break;
        case LDY_imm: ocs = "LDY_imm"; ldr(m_yIndex, AddrMode::IMM);      break;
        case LDY_zp:  ocs = "LDY_zp";  ldr(m_yIndex, AddrMode::ZP);       break;
        case LDY_zpx: ocs = "LDY_zpx"; ldr(m_yIndex, AddrMode::ZPX);      break;
        case LSR:     ocs = "LSR";                                        break;
        case LSR_abs: ocs = "LSR_abs";                                    break;
        case LSR_abx: ocs = "LSR_abx";                                    break;
        case LSR_zp:  ocs = "LSR_zp";                                     break;
        case LSR_zpx: ocs = "LSR_zpx";                                    break;
        case NOP1:    ocs = "NOP1";    nop();                             break;
        case NOP3:    ocs = "NOP3";    nop();                             break;
        case NOP5:    ocs = "NOP5";    nop();                             break;
        case NOP7:    ocs = "NOP7";    nop();                             break;
        case NOPD:    ocs = "NOPD";    nop();                             break;
        case NOPE:    ocs = "NOPE";    nop();                             break;
        case NOPF:    ocs = "NOPF";    nop();                             break;
        case NOP_ab1: ocs = "NOP_ab1"; nop();                             break;
        case NOP_ab3: ocs = "NOP_ab3"; nop();                             break;
        case NOP_ab5: ocs = "NOP_ab5"; nop();                             break;
        case NOP_ab7: ocs = "NOP_ab7"; nop();                             break;
        case NOP_abD: ocs = "NOP_abD"; nop();                             break;
        case NOP_abF: ocs = "NOP_abF"; nop();                             break;
        case NOP_abs: ocs = "NOP_abs"; nop();                             break;
        case NOP_im2: ocs = "NOP_im2"; nop();                             break;
        case NOP_im3: ocs = "NOP_im3"; nop();                             break;
        case NOP_im4: ocs = "NOP_im4"; nop();                             break;
        case NOP_im5: ocs = "NOP_im5"; nop();                             break;
        case NOP_imm: ocs = "NOP_imm"; nop();                             break;
        case NOP_zp0: ocs = "NOP_zp0"; nop();                             break;
        case NOP_zp1: ocs = "NOP_zp1"; nop();                             break;
        case NOP_zp3: ocs = "NOP_zp3"; nop();                             break;
        case NOP_zp4: ocs = "NOP_zp4"; nop();                             break;
        case NOP_zp5: ocs = "NOP_zp5"; nop();                             break;
        case NOP_zp6: ocs = "NOP_zp6"; nop();                             break;
        case NOP_zp7: ocs = "NOP_zp7"; nop();                             break;
        case NOP_zpD: ocs = "NOP_zpD"; nop();                             break;
        case NOP_zpF: ocs = "NOP_zpF"; nop();                             break;
        case ORA_abs: ocs = "ORA_abs"; ora(AddrMode::ABS);                break;
        case ORA_abx: ocs = "ORA_abx"; ora(AddrMode::ABX);                break;
        case ORA_aby: ocs = "ORA_aby"; ora(AddrMode::ABY);                break;
        case ORA_imm: ocs = "ORA_imm"; ora(AddrMode::IMM);                break;
        case ORA_izx: ocs = "ORA_izx"; ora(AddrMode::IZX);                break;
        case ORA_izy: ocs = "ORA_izy"; ora(AddrMode::IZY);                break;
        case ORA_zp:  ocs = "ORA_zp";  ora(AddrMode::ZP);                 break;
        case ORA_zpx: ocs = "ORA_zpx"; ora(AddrMode::ZPX);                break;
        case PHA: ocs = "PHA"; break;
        case PHP: ocs = "PHP"; break;
        case PLA: ocs = "PLA"; break;
        case PLP: ocs = "PLP"; break;
        case RLA_abs: ocs = "RLA_abs"; break;
        case RLA_abx: ocs = "RLA_abx"; break;
        case RLA_aby: ocs = "RLA_aby"; break;
        case RLA_izx: ocs = "RLA_izx"; break;
        case RLA_izy: ocs = "RLA_izy"; break;
        case RLA_zp: ocs = "RLA_zp"; break;
        case RLA_zpx: ocs = "RLA_zpx"; break;
        case ROL:     ocs = "ROL";     rol(AddrMode::IMP);                break;
        case ROL_abs: ocs = "ROL_abs"; rol(AddrMode::ABS);                break;
        case ROL_abx: ocs = "ROL_abx"; rol(AddrMode::ABX);                break;
        case ROL_zp:  ocs = "ROL_zp";  rol(AddrMode::ZP);                 break;
        case ROL_zpx: ocs = "ROL_zpx"; rol(AddrMode::ZPX);                break;
        case ROR:     ocs = "ROR";     ror(AddrMode::IMP);                break;
        case ROR_abs: ocs = "ROR_abs"; ror(AddrMode::ABS);                break;
        case ROR_abx: ocs = "ROR_abx"; ror(AddrMode::ABX);                break;
        case ROR_zp:  ocs = "ROR_zp";  ror(AddrMode::ZP);                 break;
        case ROR_zpx: ocs = "ROR_zpx"; ror(AddrMode::ZPX);                break;
        case RRA_abs: ocs = "RRA_abs"; break;
        case RRA_abx: ocs = "RRA_abx"; break;
        case RRA_aby: ocs = "RRA_aby"; break;
        case RRA_izx: ocs = "RRA_izx"; break;
        case RRA_izy: ocs = "RRA_izy"; break;
        case RRA_zp: ocs = "RRA_zp"; break;
        case RRA_zpx: ocs = "RRA_zpx"; break;
        case RTI: ocs = "RTI"; break;
        case RTS:     ocs = "RTS";     rts();                             break;
        case SAX_abs: ocs = "SAX_abs"; break;
        case SAX_izx: ocs = "SAX_izx"; break;
        case SAX_zpy: ocs = "SAX_zpy"; break;
        case SBC_abs: ocs = "SBC_abs"; break;
        case SBC_abx: ocs = "SBC_abx"; break;
        case SBC_aby: ocs = "SBC_aby"; break;
        case SBC_im2: ocs = "SBC_im2"; break;
        case SBC_imm: ocs = "SBC_imm"; break;
        case SBC_izx: ocs = "SBC_izx"; break;
        case SBC_izy: ocs = "SBC_izy"; break;
        case SBC_zp: ocs = "SBC_zp"; break;
        case SBC_zpx: ocs = "SBC_zpx"; break;
        case SEC: ocs = "SEC"; break;
        case SED: ocs = "SED"; break;
        case SEI: ocs = "SEI"; sei(); break;
        case SHX_aby: ocs = "SHX_aby"; break;
        case SHY_abx: ocs = "SHY_abx"; break;
        case SLO_abs: ocs = "SLO_abs"; break;
        case SLO_abx: ocs = "SLO_abx"; break;
        case SLO_aby: ocs = "SLO_aby"; break;
        case SLO_izx: ocs = "SLO_izx"; break;
        case SLO_izy: ocs = "SLO_izy"; break;
        case SLO_zp: ocs = "SLO_zp"; break;
        case SLO_zpx: ocs = "SLO_zpx"; break;
        case SRE_abs: ocs = "SRE_abs"; break;
        case SRE_abx: ocs = "SRE_abx"; break;
        case SRE_aby: ocs = "SRE_aby"; break;
        case SRE_izx: ocs = "SRE_izx"; break;
        case SRE_izy: ocs = "SRE_izy"; break;
        case SRE_zp4: ocs = "SRE_zp4"; break;
        case SRE_zp8: ocs = "SRE_zp8"; break;
        case SRE_zpx: ocs = "SRE_zpx"; break;
        case STA_abs: ocs = "STA_abs"; str(m_accumulator, AddrMode::ABS); break;
        case STA_abx: ocs = "STA_abx"; str(m_accumulator, AddrMode::ABX); break;
        case STA_aby: ocs = "STA_aby"; str(m_accumulator, AddrMode::ABY); break;
        case STA_izx: ocs = "STA_izx"; str(m_accumulator, AddrMode::IZX); break;
        case STA_izy: ocs = "STA_izy"; str(m_accumulator, AddrMode::IZY); break;
        case STA_zp3: ocs = "STA_zp3"; str(m_accumulator, AddrMode::ZP);  break;
        case STA_zpx: ocs = "STA_zpx"; str(m_accumulator, AddrMode::ZPX); break;
        case STX_abs: ocs = "STX_abs"; str(m_xIndex, AddrMode::ABS);      break;
        case STX_zp3: ocs = "STX_zp3"; str(m_xIndex, AddrMode::ZP);       break;
        case STX_zpx: ocs = "STX_zpx"; str(m_xIndex, AddrMode::ZPX);      break;
        case STY_abs: ocs = "STY_abs"; str(m_yIndex, AddrMode::ABS);      break;
        case STY_zp3: ocs = "STY_zp3"; str(m_yIndex, AddrMode::ZP);       break;
        case STY_zpx: ocs = "STY_zpx"; str(m_yIndex, AddrMode::ZPX);      break;
        case TAS_aby: ocs = "TAS_aby"; break;
        case TAX:     ocs = "TAX";     tsd(m_accumulator, m_xIndex);      break;
        case TAY:     ocs = "TAY";     tsd(m_accumulator, m_yIndex);      break;
        case TSX:     ocs = "TSX";     tsx();                             break;
        case TXA:     ocs = "TXA";     tsd(m_xIndex, m_accumulator);      break;
        case TXS:     ocs = "TXS";     txs();                             break;
        case TYA:     ocs = "TYA";     tsd(m_yIndex, m_accumulator);      break;
        case XAA_imm: ocs = "XAA_imm"; break;
        default: ocs = "unknown"; break;
    }

    printf("PC:0x%04X, OP:%7s, NEW PC:0x%4X, A:0x%02X, X:0x%02X, Y:0x%02X, S:0x%02X, M:0x%02X\n",
            programCounter,
            ocs.c_str(),
            m_programCounter,
            m_accumulator,
            m_xIndex,
            m_yIndex,
            m_status.all,
            m_sramPtr[0xD012]);
    assert(programCounter != m_programCounter); // if these are equal we did nothing
}

void Cpu::adc(const AddrMode mode)
{
    uint8_t op = m_sramPtr[computeAddress(mode)];
    uint16_t result = ((uint16_t)m_accumulator) + op + m_status.bits.carryFlag;
    m_status.bits.carryFlag = (result > 0xFF);
    m_accumulator = result & 0xFF;
}

void Cpu::andi(const AddrMode mode)
{
    m_accumulator &= m_sramPtr[computeAddress(mode)];
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::br(uint8_t flag, uint8_t condition)
{
    if(flag == condition) {
        m_programCounter = computeAddress(AddrMode::REL);
    } else {
        m_programCounter += 2;
    }
}

void Cpu::clc()
{
    ++m_programCounter;
    m_status.bits.carryFlag = 0;
}

void Cpu::cld()
{
    ++m_programCounter;
    m_status.bits.decimalModeFlag = 0;
}

void Cpu::cli()
{
    ++m_programCounter;
    m_status.bits.interruptDisableFlag = 0;
}

void Cpu::clv()
{
    ++m_programCounter;
    m_status.bits.overflowFlag = 0;
}

void Cpu::cmp(uint8_t r, const AddrMode mode)
{
    uint8_t op = m_sramPtr[computeAddress(mode)];
    m_status.bits.carryFlag = (op <= r);
    m_status.bits.negativeFlag = ((r - op) & 0x80) > 0;
    m_status.bits.zeroFlag = (op == r);
}

void Cpu::dec(const AddrMode mode)
{
    der(m_sramPtr[computeAddress(mode)]);
}

void Cpu::der(uint8_t& r)
{
    --r;
    m_status.bits.negativeFlag = (r & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == r);
    ++m_programCounter;
}

void Cpu::eor(const AddrMode mode)
{
    m_accumulator ^= m_sramPtr[computeAddress(mode)];
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::inc(const AddrMode mode)
{
    inr(m_sramPtr[computeAddress(mode)]);
}

void Cpu::inr(uint8_t& r)
{
    ++r;
    m_status.bits.negativeFlag = (r & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == r);
    ++m_programCounter;
}

void Cpu::jmp(const AddrMode mode)
{
    m_programCounter = computeAddress(mode);
}

void Cpu::jsr()
{
    writeWord(m_programCounter + 2, m_stackPointer - 1);
    m_stackPointer -= 2;
    m_programCounter = readWord(++m_programCounter);
}

void Cpu::ldr(uint8_t &r, const AddrMode mode)
{
    r = m_sramPtr[computeAddress(mode)];
    m_status.bits.negativeFlag = (r & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == r);
}

void Cpu::nop()
{
    ++m_programCounter;
}

void Cpu::ora(const AddrMode mode)
{
    m_accumulator |= m_sramPtr[computeAddress(mode)];
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::rol(const AddrMode mode)
{
    uint8_t *target = &m_accumulator;
    if(AddrMode::IMP != mode) {
        target = &m_sramPtr[computeAddress(mode)];
    } else {
        ++m_programCounter;
    }
    
    bool newCarry = (*target & 0x80) > 0;
    *target <<= 1;
    *target += m_status.bits.carryFlag;
    m_status.bits.negativeFlag = (*target & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == *target);
    m_status.bits.carryFlag = newCarry;
}

void Cpu::ror(const AddrMode mode)
{
    uint8_t *target = &m_accumulator;
    if(AddrMode::IMP != mode) {
        target = &m_sramPtr[computeAddress(mode)];
    } else {
        ++m_programCounter;
    }
    
    bool newCarry = (*target & 0x01) > 0;
    *target >>= 1;
    *target += m_status.bits.carryFlag ? 0x80 : 0x00;
    m_status.bits.negativeFlag = (*target & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == *target);
    m_status.bits.carryFlag = newCarry;
}

void Cpu::rts()
{
    m_stackPointer += 2;
    m_programCounter = readWord(m_stackPointer - 1) + 1;
}

void Cpu::sec()
{
    ++m_programCounter;
    m_status.bits.carryFlag = 1;
}

void Cpu::sed()
{
    ++m_programCounter;
    m_status.bits.decimalModeFlag = 1;
}

void Cpu::sei()
{
    ++m_programCounter;
    m_status.bits.interruptDisableFlag = 1;
}

void Cpu::str(const uint8_t &r, const AddrMode mode)
{
    m_sramPtr[computeAddress(mode)] = r;
}

void Cpu::tsd(const uint8_t &src, uint8_t &dst)
{
    ++m_programCounter;
    dst = src;
    m_status.bits.negativeFlag = (dst & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == dst);
}

void Cpu::tsx()
{
    ++m_programCounter;
    m_xIndex = m_stackPointer & 0xFF;
}

void Cpu::txs()
{
    ++m_programCounter;
    m_stackPointer = 0x100 | m_xIndex;
}

void Cpu::init()
{
    m_sramPtr[0] = 0xFF;
    m_sramPtr[1] = 0x07;
    m_programCounter = readWord(0xFFFC);
    m_stackPointer = 0x1FF;
}

uint16_t Cpu::readWord(uint16_t addr)
{
    uint16_t word = m_sramPtr[addr + 1] << 8;
    word |= m_sramPtr[addr];
    return word;
}

void Cpu::writeWord(uint16_t word, uint16_t addr)
{
    m_sramPtr[addr] = word & 0xFF;
    m_sramPtr[addr + 1] = (word >> 8) & 0xFF;
}

uint16_t Cpu::computeAddress(const AddrMode mode)
{
    ++m_programCounter;
    uint16_t addr = 0;
    switch(mode) {
        case AddrMode::IMM:
            addr = m_programCounter++;
            break;
        case AddrMode::REL:
            addr = m_programCounter + ((int8_t)m_sramPtr[m_programCounter++]) + 1;
            break;
        case AddrMode::ZP:
            addr = m_sramPtr[m_programCounter++];
            break;
        case AddrMode::ZPX:
            addr = m_sramPtr[m_programCounter++] + m_xIndex;
            break;
        case AddrMode::ZPY:
            addr = m_sramPtr[m_programCounter++] + m_yIndex;
            break;
        case AddrMode::ABS:
            addr = readWord(m_programCounter);
            m_programCounter += 2;
            break;
        case AddrMode::ABX:
            addr = readWord(m_programCounter) + m_xIndex;
            m_programCounter += 2;
            break;
        case AddrMode::ABY:
            addr = readWord(m_programCounter) + m_yIndex;
            m_programCounter += 2;
            break;
        case AddrMode::IND:
            addr = m_sramPtr[readWord(m_programCounter)];
            m_programCounter++;
            break;
        case AddrMode::IZX:
            addr = m_sramPtr[m_programCounter++ + m_xIndex];
            break;
        case AddrMode::IZY:
            addr = m_sramPtr[m_programCounter++ + m_yIndex];
            break;
    };

    return addr;
}

Cpu::Cpu()
    : m_sramPtr(new uint8_t[65536])
    , m_ownsSramPtr(true)
{
    init();
}

Cpu::Cpu(uint8_t *sramPtr)
    : m_sramPtr(sramPtr)
    , m_ownsSramPtr(false)
{
    init();
}

Cpu::~Cpu()
{
    if(m_ownsSramPtr) {
        delete[] m_sramPtr;
    }
}

} // namespace MOS6510
