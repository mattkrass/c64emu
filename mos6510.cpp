#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <assert.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "mos6510.h"

namespace MOS6510 {

const char cgromLookup[] =
    { '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
      'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
      'X', 'Y', 'Z', '[', '~', ']', '^', '<', ' ', '!', '"', '#',
      '$', '%', '&', 0x27,'(', ')', '*', '+', ',', '-', '.', '/',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';',
      '<', '=', '>', '?', '-', '~', '~', '~', '~', '~', '~', '~',
      '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
      '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
      '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
      '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
      '~', '~', '~', '~', '~', '~', '~', '~' };

void Cpu::debugPrompt()
{
    bool promptActive = true;
    while (promptActive) {
        std::cout << "dbg> ";
        std::string line;
        std::string token;
        std::getline(std::cin, line);
        std::stringstream ss(line);
        std::vector<std::string> args;
        while(std::getline(ss, token, ' ')) {
            if(0 < token.length()) {
                args.push_back(token);
            }
        }

        if(0 < args.size()) {
            if("exit" == args[0]) {
                exit(0); // shut it down!
            } else if("run" == args[0]) {
                return;
            }

            std::map<std::string, Cpu::cmdFunc>::iterator it = m_cmdMap.find(args[0]);
            if(m_cmdMap.cend() != it) {
                cmdFunc fn = it->second;
                promptActive = (this->*fn)(args);
            } else {
                std::cout << "Invalid command." << std::endl;
            }
        }
    }
}

static uint16_t parseString(const std::string& num)
{
    uint16_t result = 0;
    const char *str = num.c_str();
    std::stringstream ss(num);
    if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) { // hex
        ss >> std::hex >> result;
    } else { // decimal or borken
        ss >> result;
    }

    return result;
}

bool Cpu::dbgRead(const std::vector<std::string>& args)
{
    uint16_t addr = 0;
    uint16_t numBytes = 1;
    if(2 > args.size()) {
        std::cout << args[0] << " <address> [number of bytes]" << std::endl;
    } else if (2 == args.size()) {
        addr = parseString(args[1]);
    } else {
        addr = parseString(args[1]);
        numBytes = parseString(args[2]);
    }

    for(int idx = 0; idx < numBytes; ++idx) {
        if(0 == idx % 24)  {
            if (idx) {
                printf("\n");
            }
            printf("0x%04X  ", addr + idx);
        }
        printf(" %02X", m_memory.read(addr + idx));
    }

    printf("\n");
    return true; // stay in debug
}

bool Cpu::dbgWrit(const std::vector<std::string>& args)
{
    uint16_t addr = 0;
    if(2 > args.size()) {
        std::cout << args[0] << " <address> [list of bytes to write]" << std::endl;
        return true; // drop back to the debug menu
    }

    addr = parseString(args[1]);
    for(size_t idx = 2; idx < args.size(); ++idx) {
        printf("A: 0x%04X B: 0x%02X\n", addr, parseString(args[idx]));
        m_memory.write(addr++, parseString(args[idx]), 0);
    }

    return true; // stay in debug
}

bool Cpu::dbgSdmp(const std::vector<std::string>& args)
{
    printf("Screen dump");
    for(int i = 0x0000; i < 0x03E8; ++i) {
        uint8_t data = m_memory.read(i + 0x0400);
        char c = cgromLookup[(data & 0x7F)];
        if(0 == (i % 40)) {
            printf(":\n0x%04X ", (i + 0x0400));
        }
        printf("%c", c);
    }

    printf(":\n");
    return true; // stay in debug
}

bool Cpu::dbgStep(const std::vector<std::string>& args)
{
    uint16_t stepCount = 1;
    if(1 > args.size()) {
        stepCount = parseString(args[1]);
    }

    m_stepCount = stepCount;
    m_stepping = true;

    return false;
}

bool Cpu::dbgBrka(const std::vector<std::string>& args)
{
    uint16_t addr = 0;
    if(2 > args.size()) {
        std::cout << "brka <address>" << std::endl;
    } else {
        addr = parseString(args[1]);
    }

    addBreakpoint(addr);

    return true; // stay in debug
}

bool Cpu::dbgBrkd(const std::vector<std::string>& args)
{
    uint16_t addr = 0;
    if(2 > args.size()) {
        std::cout << "brkd <address>" << std::endl;
    } else {
        addr = parseString(args[1]);
    }

    removeBreakpoint(addr);

    return true; // stay in debug
}

bool Cpu::dbgLsbp(const std::vector<std::string>& args)
{
    std::set<uint16_t>::const_iterator it = m_breakpointSet.cbegin();
    for(int idx = 0; it != m_breakpointSet.cend(); ++it, ++idx) {
        printf("%d: 0x%04X\n", idx, *it);
    }

    return true; // stay in debug
}

bool Cpu::dbgSeti(const std::vector<std::string>& args)
{
    m_pendingIrq = true;
    return true; // stay in debug
}

bool Cpu::dbgClri(const std::vector<std::string>& args)
{
    m_pendingIrq = false;
    return true; // stay in debug
}

bool Cpu::dbgSreg(const std::vector<std::string>& args)
{
    uint16_t value = 0;
    if(3 > args.size()) {
        std::cout << "sreg <A/X/Y/S> <new value>" << std::endl;
    } else {
        value = parseString(args[2]);
    }

    const std::string& reg = args[1];
    uint8_t* regptr;

    if("A" == reg) {
        regptr = &m_accumulator;
    } else if("X" == reg) {
        regptr = &m_xIndex;
    } else if("Y" == reg) {
        regptr = &m_yIndex;
    } else if("S" == reg) {
        regptr = &m_status.all;
    } else {
        std::cout << "Invalid register." << std::endl;
        return true;
    }

    m_status.bits.negativeFlag = (value & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == value);
    *regptr = value;

    return true; // stay in debug
}

bool Cpu::dbgPstk(const std::vector<std::string>& args)
{
    printStack();   
    return true; // stay in debug
}

void Cpu::execute(bool debugBreak)
{
    ++m_totalCycleCount;
    if(CpuState::READ_NEXT_OPCODE == m_cpuState) {
        if (0 == m_status.bits.interruptDisableFlag && m_pendingIrq) {
            isr();
        }

        assert(canary1  == 0xAAAA);
        assert(canary2  == 0xAAAA);
        assert(canary3  == 0xAAAA);
        assert(canary4  == 0xAAAA);
        assert(canary5  == 0xAAAA);
        assert(canary6  == 0xAAAA);
        assert(canary7  == 0xAAAA);
        assert(canary8  == 0xAAAA);
        assert(canary9  == 0xAAAA);
        assert(canary10 == 0xAAAA);
        assert(canary11 == 0xAAAA);
        assert(canary12 == 0xAAAA);
        assert(canary13 == 0xAAAA);
        assert(canary14 == 0xAAAA);
        assert(canary15 == 0xAAAA);
        assert(canary16 == 0xAAAA);
        assert(canary17 == 0xAAAA);
        assert(canary18 == 0xAAAA);
        assert(canary19 == 0xAAAA);
        assert(canary20 == 0xAAAA);
        assert(canary21 == 0xAAAA);
        assert(canary22 == 0xAAAA);
        assert(canary23 == 0xAAAA);
        assert(canary24 == 0xAAAA);
        ///assert(m_programCounter > 0x4000);
        m_opcode = m_memory.read(m_programCounter);
        m_opcodeReadFrom = m_programCounter;
        m_opcodeHistory.push_back(m_opcodeReadFrom);
        while(m_opcodeHistory.size() > 100) {
            m_opcodeHistory.pop_front();
        }
        if(m_debugMode || debugBreak || m_stepping) {
            if(m_stepCount) {
                --m_stepCount;
            }

            char status[11];
            snprintf(status, sizeof(status), "S:%c%c%c%c%c%c%c%c",
                    m_status.bits.carryFlag             ? 'C' : '.',
                    m_status.bits.zeroFlag              ? 'Z' : '.',
                    m_status.bits.interruptDisableFlag  ? 'I' : '.',
                    m_status.bits.decimalModeFlag       ? 'D' : '.',
                    m_status.bits.breakFlag             ? 'B' : '.',
                    m_status.bits.unusedFlag            ? 'U' : '.',
                    m_status.bits.overflowFlag          ? 'O' : '.',
                    m_status.bits.negativeFlag          ? 'N' : '.');
            uint16_t pc = m_programCounter;
            const std::string decoded = decodeInstruction(pc);
            printf("PC:0x%04X, OP:%-14s, A:0x%02X, X:0x%02X, Y:0x%02X, %s, SP:0x%03X, CYC:%d\n",
                    m_programCounter,
                    decoded.c_str(),
                    m_accumulator,
                    m_xIndex,
                    m_yIndex,
                    status,
                    m_stackPointer,
                    m_totalCycleCount);

            if(debugBreak) { 
                printf(">>>>>>>>>> CPU paused at 0x%04X <<<<<<<<<<\n", m_programCounter);
                debugPrompt();
            } else if(m_breakpointSet.end() != m_breakpointSet.find(m_programCounter)) {
                printf(">>>>>>>>>> Hit breakpoint at 0x%04X <<<<<<<<<<\n", m_programCounter);
                debugPrompt();
            } else if(m_stepping) {
                if(!m_stepCount) {
                    printf(">>>>>>>>>> Finished step count at 0x%04X <<<<<<<<<<\n", m_programCounter);
                    debugPrompt();
                }
            }

            if(m_stepping && !m_stepCount) {
                m_stepping = false; // we clear this late in case of coincident breakpoint and step count end
            }
        }

        m_programCounter++;
        m_cpuState = INSTRUCTION_IN_PROGRESS;
    } else {
        if(debugBreak) {
            m_debugMode = true;
            printf("Current executing %02X opcode from 0x%04X, state = %d\n", m_opcode, m_opcodeReadFrom, m_instructionState);
            for(size_t idx = 0; idx < m_opcodeHistory.size() && idx < 10; ++idx) {
                printf("%02lu: 0x%04X\n", idx, m_opcodeHistory.at(m_opcodeHistory.size() - idx - 1));
            }
        }
        switch (m_opcode) {
            case ADC_abs: rdAbs(&Cpu::adc);                        break;
            case ADC_abx: rdAbsIdx(&Cpu::adc, m_xIndex);           break;
            case ADC_aby: rdAbsIdx(&Cpu::adc, m_yIndex);           break;
            case ADC_imm: rdImm(&Cpu::adc);                        break;
            case ADC_izx: rdIdxInd(&Cpu::adc);                     break;
            case ADC_izy: rdIndIdx(&Cpu::adc);                     break;
            case ADC_zp:  rmwZeroPage(&Cpu::adc);                  break;
            case ADC_zpx: rmwZeroPageIdx(&Cpu::adc, m_xIndex);     break;
            case AHX_aby:                                          break;
            case AHX_izy:                                          break;
            case ALR_imm:                                          break;
            case ANC_im2:                                          break;
            case ANC_imm:                                          break;
            case AND_abs: rdAbs(&Cpu::andi);                       break;
            case AND_abx: rdAbsIdx(&Cpu::andi, m_xIndex);          break;
            case AND_aby: rdAbsIdx(&Cpu::andi, m_yIndex);          break;
            case AND_imm: rdImm(&Cpu::andi);                       break;
            case AND_izx: rdIdxInd(&Cpu::andi);                    break;
            case AND_izy: rdIndIdx(&Cpu::andi);                    break;
            case AND_zp:  rdZeroPage(&Cpu::andi);                  break;
            case AND_zpx: rdZeroPageIdx(&Cpu::andi, m_xIndex);     break;
            case ARR_imm:                                          break;
            case ASL:     rmwImp(&Cpu::asl);                       break;
            case ASL_abs: rmwAbs(&Cpu::asl);                       break;
            case ASL_abx: rmwAbsIdx(&Cpu::asl, m_xIndex);          break;
            case ASL_zp:  rmwZeroPage(&Cpu::asl);                  break;
            case ASL_zpx: rmwZeroPageIdx(&Cpu::asl, m_xIndex);     break;
            case AXS_imm:                                          break;
            case BCC_rel: br(m_status.bits.carryFlag, 0);          break;
            case BCS_rel: br(m_status.bits.carryFlag, 1);          break;
            case BEQ_rel: br(m_status.bits.zeroFlag, 1);           break;
            case BIT_abs: rdAbs(&Cpu::bit);                        break;
            case BIT_zp:  rdZeroPage(&Cpu::bit);                   break;
            case BMI_rel: br(m_status.bits.negativeFlag, 1);       break;
            case BNE_rel: br(m_status.bits.zeroFlag, 0);           break;
            case BPL_rel: br(m_status.bits.negativeFlag, 0);       break;
            case BRK:                                              break;
            case BVC_rel: br(m_status.bits.overflowFlag, 0);       break;
            case BVS_rel: br(m_status.bits.overflowFlag, 1);       break;
            case CLC:     rdImp(&Cpu::clc);                        break;
            case CLD:     rdImp(&Cpu::cld);                        break;
            case CLI:     rdImp(&Cpu::cli);                        break;
            case CLV:     rdImp(&Cpu::clv);                        break;
            case CMP_abs: rdAbs(&Cpu::cmp);                        break;
            case CMP_abx: rdAbsIdx(&Cpu::cmp, m_xIndex);           break;
            case CMP_aby: rdAbsIdx(&Cpu::cmp, m_yIndex);           break;
            case CMP_imm: rdImm(&Cpu::cmp);                        break;
            case CMP_izx: rdIdxInd(&Cpu::cmp);                     break;
            case CMP_izy: rdIndIdx(&Cpu::cmp);                     break;
            case CMP_zp:  rdZeroPage(&Cpu::cmp);                   break;
            case CMP_zpx: rdZeroPageIdx(&Cpu::cmp, m_xIndex);      break;
            case CPX_abs: rdAbs(&Cpu::cpx);                        break;
            case CPX_imm: rdImm(&Cpu::cpx);                        break;
            case CPX_zp:  rdZeroPage(&Cpu::cpx);                   break;
            case CPY_abs: rdAbs(&Cpu::cpy);                        break;
            case CPY_imm: rdImm(&Cpu::cpy);                        break;
            case CPY_zp:  rdZeroPage(&Cpu::cpy);                   break;
            case DCP_abs:                                          break;
            case DCP_abx:                                          break;
            case DCP_aby:                                          break;
            case DCP_izx:                                          break;
            case DCP_izy:                                          break;
            case DCP_zp:                                           break;
            case DCP_zpx:                                          break;
            case DEC_abs: rmwAbs(&Cpu::dec);                       break;
            case DEC_abx: rmwAbsIdx(&Cpu::dec, m_xIndex);          break;
            case DEC_zp:  rmwZeroPage(&Cpu::dec);                  break;
            case DEC_zpx: rmwZeroPageIdx(&Cpu::dec, m_xIndex);     break;
            case DEX:     rdImp(&Cpu::dex);                        break;
            case DEY:     rdImp(&Cpu::dey);                        break;
            case EOR_abs: rdAbs(&Cpu::eor);                        break;
            case EOR_abx: rdAbsIdx(&Cpu::eor, m_xIndex);           break;
            case EOR_aby: rdAbsIdx(&Cpu::eor, m_yIndex);           break;
            case EOR_imm: rdImm(&Cpu::eor);                        break;
            case EOR_izx: rdIdxInd(&Cpu::eor);                     break;
            case EOR_izy: rdIndIdx(&Cpu::eor);                     break;
            case EOR_zp:  rmwZeroPage(&Cpu::eor);                  break;
            case EOR_zpx: rmwZeroPageIdx(&Cpu::eor, m_xIndex);     break;
            case INC_abs: rmwAbs(&Cpu::inc);                       break;
            case INC_abx: rmwAbsIdx(&Cpu::inc, m_xIndex);          break;
            case INC_zp:  rmwZeroPage(&Cpu::inc);                  break;
            case INC_zpx: rmwZeroPageIdx(&Cpu::inc, m_xIndex);     break;
            case INX:     rdImp(&Cpu::inx);                        break;
            case INY:     rdImp(&Cpu::iny);                        break;
            case ISC_abs:                                          break;
            case ISC_abx:                                          break;
            case ISC_aby:                                          break;
            case ISC_izx:                                          break;
            case ISC_izy:                                          break;
            case ISC_zp:                                           break;
            case ISC_zpx:                                          break;
            case JMP_abs: jmpAbs();                                break;
            case JMP_ind: jmpInd();                                break;
            case JSR:     jsr();                                   break;
            case KIL0:    exit(1);                                 break;
            case KIL1:    exit(1);                                 break;
            case KIL2:    exit(1);                                 break;
            case KIL3:    exit(1);                                 break;
            case KIL4:    exit(1);                                 break;
            case KIL5:    exit(1);                                 break;
            case KIL6:    exit(1);                                 break;
            case KIL7:    exit(1);                                 break;
            case KIL9:    exit(1);                                 break;
            case KILB:    exit(1);                                 break;
            case KILD:    exit(1);                                 break;
            case KILF:    exit(1);                                 break;
            case LAS_aby:                                          break;
            case LAX_abs:                                          break;
            case LAX_aby:                                          break;
            case LAX_imm:                                          break;
            case LAX_izx:                                          break;
            case LAX_izy:                                          break;
            case LAX_zp:                                           break;
            case LAX_zpy:                                          break;
            case LDA_abs: rdAbs(&Cpu::lda);                        break;
            case LDA_abx: rdAbsIdx(&Cpu::lda, m_xIndex);           break;
            case LDA_aby: rdAbsIdx(&Cpu::lda, m_yIndex);           break;
            case LDA_imm: rdImm(&Cpu::lda);                        break;
            case LDA_izx: rdIdxInd(&Cpu::lda);                     break;
            case LDA_izy: rdIndIdx(&Cpu::lda);                     break;
            case LDA_zp:  rdZeroPage(&Cpu::lda);                   break;
            case LDA_zpx: rdZeroPageIdx(&Cpu::lda, m_xIndex);      break;
            case LDX_abs: rdAbs(&Cpu::ldx);                        break;
            case LDX_aby: rdAbsIdx(&Cpu::ldx, m_yIndex);           break;
            case LDX_imm: rdImm(&Cpu::ldx);                        break;
            case LDX_zp:  rdZeroPage(&Cpu::ldx);                   break;
            case LDX_zpy: rdZeroPageIdx(&Cpu::ldx, m_yIndex);      break;
            case LDY_abs: rdAbs(&Cpu::ldy);                        break;
            case LDY_abx: rdAbsIdx(&Cpu::ldy, m_xIndex);           break;
            case LDY_imm: rdImm(&Cpu::ldy);                        break;
            case LDY_zp:  rdZeroPage(&Cpu::ldy);                   break;
            case LDY_zpx: rdZeroPageIdx(&Cpu::ldy, m_xIndex);      break;
            case LSR:     rmwImp(&Cpu::lsr);                       break;
            case LSR_abs: rmwAbs(&Cpu::lsr);                       break;
            case LSR_abx: rmwAbsIdx(&Cpu::lsr, m_xIndex);          break;
            case LSR_zp:  rmwZeroPage(&Cpu::lsr);                  break;
            case LSR_zpx: rmwZeroPageIdx(&Cpu::lsr, m_xIndex);     break;
            case NOP1:    nop();                                   break;
            case NOP3:    nop();                                   break;
            case NOP5:    nop();                                   break;
            case NOP7:    nop();                                   break;
            case NOPD:    nop();                                   break;
            case NOPE:    nop();                                   break;
            case NOPF:    nop();                                   break;
            case NOP_ab1: nop();                                   break;
            case NOP_ab3: nop();                                   break;
            case NOP_ab5: nop();                                   break;
            case NOP_ab7: nop();                                   break;
            case NOP_abD: nop();                                   break;
            case NOP_abF: nop();                                   break;
            case NOP_abs: nop();                                   break;
            case NOP_im2: nop();                                   break;
            case NOP_im3: nop();                                   break;
            case NOP_im4: nop();                                   break;
            case NOP_im5: nop();                                   break;
            case NOP_imm: nop();                                   break;
            case NOP_zp0: nop();                                   break;
            case NOP_zp1: nop();                                   break;
            case NOP_zp3: nop();                                   break;
            case NOP_zp4: nop();                                   break;
            case NOP_zp5: nop();                                   break;
            case NOP_zp6: nop();                                   break;
            case NOP_zp7: nop();                                   break;
            case NOP_zpD: nop();                                   break;
            case NOP_zpF: nop();                                   break;
            case ORA_abs: rdAbs(&Cpu::ora);                        break;
            case ORA_abx: rdAbsIdx(&Cpu::ora, m_xIndex);           break;
            case ORA_aby: rdAbsIdx(&Cpu::ora, m_yIndex);           break;
            case ORA_imm: rdImm(&Cpu::ora);                        break;
            case ORA_izx: rdIdxInd(&Cpu::ora);                     break;
            case ORA_izy: rdIndIdx(&Cpu::ora);                     break;
            case ORA_zp:  rdZeroPage(&Cpu::ora);                   break;
            case ORA_zpx: rdZeroPageIdx(&Cpu::ora, m_xIndex);      break;
            case PHA:     pha();                                   break;
            case PHP:     php();                                   break;
            case PLA:     pla();                                   break;
            case PLP:     plp();                                   break;
            case RLA_abs:                                          break;
            case RLA_abx:                                          break;
            case RLA_aby:                                          break;
            case RLA_izx:                                          break;
            case RLA_izy:                                          break;
            case RLA_zp:                                           break;
            case RLA_zpx:                                          break;
            case ROL:     rmwImp(&Cpu::rol);                       break;
            case ROL_abs: rmwAbs(&Cpu::rol);                       break;
            case ROL_abx: rmwAbsIdx(&Cpu::rol, m_xIndex);          break;
            case ROL_zp:  rmwZeroPage(&Cpu::rol);                  break;
            case ROL_zpx: rmwZeroPageIdx(&Cpu::rol, m_xIndex);     break;
            case ROR:     rmwImp(&Cpu::ror);                       break;
            case ROR_abs: rmwAbs(&Cpu::ror);                       break;
            case ROR_abx: rmwAbsIdx(&Cpu::ror, m_xIndex);          break;
            case ROR_zp:  rmwZeroPage(&Cpu::ror);                  break;
            case ROR_zpx: rmwZeroPageIdx(&Cpu::ror, m_xIndex);     break;
            case RRA_abs:                                          break;
            case RRA_abx:                                          break;
            case RRA_aby:                                          break;
            case RRA_izx:                                          break;
            case RRA_izy:                                          break;
            case RRA_zp:                                           break;
            case RRA_zpx:                                          break;
            case RTI:     rti();                                   break;
            case RTS:     rts();                                   break;
            case SAX_abs:                                          break;
            case SAX_izx:                                          break;
            case SAX_zpy:                                          break;
            case SBC_abs: rmwAbs(&Cpu::sbc);                       break;
            case SBC_abx: rmwAbsIdx(&Cpu::sbc, m_xIndex);          break;
            case SBC_aby: rmwAbsIdx(&Cpu::sbc, m_yIndex);          break;
            case SBC_im2:                                          break;
            case SBC_imm: rdImm(&Cpu::sbc);                        break;
            case SBC_izx: rmwIdxInd(&Cpu::sbc);                    break;
            case SBC_izy: rmwIndIdx(&Cpu::sbc);                    break;
            case SBC_zp:  rmwZeroPage(&Cpu::sbc);                  break;
            case SBC_zpx: rmwZeroPageIdx(&Cpu::sbc, m_xIndex);     break;
            case SEC:     rdImp(&Cpu::sec);                        break;
            case SED:     rdImp(&Cpu::sed);                        break;
            case SEI:     rdImp(&Cpu::sei);                        break;
            case SHX_aby:                                          break;
            case SHY_abx:                                          break;
            case SLO_abs:                                          break;
            case SLO_abx:                                          break;
            case SLO_aby:                                          break;
            case SLO_izx:                                          break;
            case SLO_izy:                                          break;
            case SLO_zp:                                           break;
            case SLO_zpx:                                          break;
            case SRE_abs:                                          break;
            case SRE_abx:                                          break;
            case SRE_aby:                                          break;
            case SRE_izx:                                          break;
            case SRE_izy:                                          break;
            case SRE_zp4:                                          break;
            case SRE_zp8:                                          break;
            case SRE_zpx:                                          break;
            case STA_abs: wrAbs(m_accumulator);                    break;
            case STA_abx: wrAbsIdx(m_accumulator, m_xIndex);       break;
            case STA_aby: wrAbsIdx(m_accumulator, m_yIndex);       break;
            case STA_izx: wrIdxInd(m_accumulator);                 break;
            case STA_izy: wrIndIdx(m_accumulator);                 break;
            case STA_zp3: wrZeroPage(m_accumulator);               break;
            case STA_zpx: wrZeroPageIdx(m_accumulator, m_xIndex);  break;
            case STX_abs: wrAbs(m_xIndex);                         break;
            case STX_zp3: wrZeroPage(m_xIndex);                    break;
            case STX_zpx: wrZeroPageIdx(m_xIndex, m_xIndex);       break;
            case STY_abs: wrAbs(m_yIndex);                         break;
            case STY_zp3: wrZeroPage(m_yIndex);                    break;
            case STY_zpx: wrZeroPageIdx(m_yIndex, m_xIndex);       break;
            case TAS_aby:                                          break;
            case TAX:     rdImp(&Cpu::tax);                        break;
            case TAY:     rdImp(&Cpu::tay);                        break;
            case TSX:     rdImp(&Cpu::tsx);                        break;
            case TXA:     rdImp(&Cpu::txa);                        break;
            case TXS:     rdImp(&Cpu::txs);                        break;
            case TYA:     rdImp(&Cpu::tya);                        break;
            case XAA_imm:                                          break;
            default:                                               break;
        }
    }

    ++m_videoTimer;
    if(34100 < m_videoTimer) { // roughly the ratio of system clock to framerate
        m_videoTimer = 0;
        m_pendingIrq = true;
    }
}

// Implied addressing
void Cpu::rdImp(opFunc operation)
{
    // perform operation
    m_memory.read(m_programCounter);
    (this->*operation)();
    m_cpuState = CpuState::READ_NEXT_OPCODE;
    m_instructionState = 0;
}

void Cpu::rmwImp(opFunc operation)
{
    // perform operation
    m_memory.read(m_programCounter);
    m_operand = m_accumulator;
    (this->*operation)();
    m_accumulator = m_operand;
    m_cpuState = CpuState::READ_NEXT_OPCODE;
    m_instructionState = 0;
}

void Cpu::wrImp(const uint8_t val)
{
    // perform operation
    m_memory.read(m_programCounter);
    m_accumulator = val;
    m_cpuState = CpuState::READ_NEXT_OPCODE;
    m_instructionState = 0;
}

// Immediate addressing
void Cpu::rdImm(opFunc operation)
{
    switch (m_instructionState) {
        case 0: {
            // perform operation
            m_operand = m_memory.read(m_programCounter++);
            (this->*operation)();
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

// Relative addressing
// Zero page addressing
void Cpu::rdZeroPage(opFunc operation)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // perform operation
            m_operand = m_memory.read(m_instAddr);
            (this->*operation)();
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::rmwZeroPage(opFunc operation)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // read operand
            m_operand = m_memory.read(m_instAddr);
            ++m_instructionState;
        } break;
        case 2: {
            // dummy write and perform operation
            m_memory.write(m_instAddr, m_operand, 0);
            (this->*operation)();
            ++m_instructionState;
        } break;
        case 3: {
            // write results
            m_memory.write(m_instAddr, m_operand, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::wrZeroPage(const uint8_t val)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // perform operation
            m_memory.write(m_instAddr, val, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

// Zero page indexed addressing
void Cpu::rdZeroPageIdx(opFunc operation, const uint8_t idx)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // dummy read from address and index it
            m_memory.read(m_instAddr);
            m_instAddr += idx;
            m_instAddr &= 0xFF;
            ++m_instructionState;
        } break;
        case 2: {
            // perform operation
            m_operand = m_memory.read(m_instAddr);
            (this->*operation)();
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::rmwZeroPageIdx(opFunc operation, const uint8_t idx)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // dummy read from address and index it
            m_memory.read(m_instAddr);
            m_instAddr += idx;
            m_instAddr &= 0xFF;
            ++m_instructionState;
        } break;
        case 2: {
            // proper read
            m_operand = m_memory.read(m_instAddr);
            ++m_instructionState;
        } break;
        case 3: {
            // dummy write and perform operation
            m_memory.write(m_instAddr, m_operand, 0);
            (this->*operation)();
            ++m_instructionState;
        } break;
        case 4: {
            // proper write
            m_memory.write(m_instAddr, m_operand, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::wrZeroPageIdx(const uint8_t val, const uint8_t idx)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // dummy read from address and index it
            m_memory.read(m_instAddr);
            m_instAddr += idx;
            m_instAddr &= 0xFF;
            ++m_instructionState;
        } break;
        case 2: {
            // proper write
            m_memory.write(m_instAddr, val, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

// Absolute addressing
void Cpu::rdAbs(opFunc operation)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch high-byte of address, increment PC
            m_instAddr |= m_memory.read(m_programCounter++) << 8;
            ++m_instructionState;
        } break;
        case 2: {
            // perform operation
            m_operand = m_memory.read(m_instAddr);
            (this->*operation)();
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::rmwAbs(opFunc operation)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch high-byte of address, increment PC
            m_instAddr |= m_memory.read(m_programCounter++) << 8;
            ++m_instructionState;
        } break;
        case 2: {
            // read operand
            m_operand = m_memory.read(m_instAddr);
            ++m_instructionState;
        } break;
        case 3: {
            // dummy write and perform operation
            m_memory.write(m_instAddr, m_operand, 0);
            (this->*operation)();
            ++m_instructionState;
        } break;
        case 4: {
            // write results
            m_memory.write(m_instAddr, m_operand, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::wrAbs(const uint8_t val)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch high-byte of address, increment PC
            m_instAddr |= m_memory.read(m_programCounter++) << 8;
            ++m_instructionState;
        } break;
        case 2: {
            // perform operation
            m_memory.write(m_instAddr, val, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::jmpAbs()
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address low, increment PC
            m_operand = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            m_programCounter = m_memory.read(m_programCounter) << 8;
            m_programCounter |= m_operand;
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

// Absolute indexed addressing
void Cpu::rdAbsIdx(opFunc operation, const uint8_t idx)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch high-byte of address, add idx, increment PC
            m_instAddr += idx;
            m_instAddr &= 0xFF;
            m_instAddr |= m_memory.read(m_programCounter++) << 8;
            ++m_instructionState;
        } break;
        case 2: {
            // read from address and fix if needed
            m_operand = m_memory.read(m_instAddr);
            if(idx > (m_instAddr & 0xFF)) {
                m_instAddr += 0x100;
                ++m_instructionState;
            } else {
                // perform operation
                m_operand = m_memory.read(m_instAddr);
                (this->*operation)();
                m_cpuState = CpuState::READ_NEXT_OPCODE;
                m_instructionState = 0;
            }
        } break;
        case 3: {
            // perform operation
            m_operand = m_memory.read(m_instAddr);
            (this->*operation)();
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::rmwAbsIdx(opFunc operation, const uint8_t idx)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch high-byte of address, add idx, increment PC
            m_instAddr += idx;
            m_instAddr &= 0xFF;
            m_instAddr |= m_memory.read(m_programCounter++) << 8;
            ++m_instructionState;
        } break;
        case 2: {
            // read from address and fix if needed
            m_memory.read(m_instAddr);
            if(idx > (m_instAddr & 0xFF)) {
                m_instAddr += 0x100;
            }
            ++m_instructionState;
        } break;
        case 3: {
            // re-read from address
            m_operand = m_memory.read(m_instAddr);
            ++m_instructionState;
        } break;
        case 4: {
            // dummy write and perform operation
            m_memory.write(m_instAddr, m_operand, 0);
            (this->*operation)();
            ++m_instructionState;
        } break;
        case 5: {
            // proper write
            m_memory.write(m_instAddr, m_operand, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

void Cpu::wrAbsIdx(const uint8_t val, const uint8_t idx)
{
    switch (m_instructionState) {
        case 0: {
            // fetch low-byte of address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch high-byte of address, add idx, increment PC
            m_instAddr += idx;
            m_instAddr &= 0xFF;
            m_instAddr |= m_memory.read(m_programCounter++) << 8;
            ++m_instructionState;
        } break;
        case 2: {
            // read from address and fix if needed
            m_memory.read(m_instAddr);
            if(idx > (m_instAddr & 0xFF)) {
                m_instAddr += 0x100;
            }
            ++m_instructionState;
        } break;
        case 3: {
            // proper write
            m_memory.write(m_instAddr, val, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    };
}

// Indexed indirect addressing
void Cpu::rdIdxInd(opFunc operation)
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // read from the address, add X to it
            m_instAddr = m_memory.read(m_instAddr) + m_xIndex;
            ++m_instructionState;
        } break;
        case 2: {
            // fetch effective address low
            m_operand = m_memory.read(m_instAddr++);
            ++m_instructionState;
        } break;
        case 3: {
            // fetch effective address high
            m_instAddr = m_memory.read(m_instAddr) << 8;
            m_instAddr |= m_operand;
            ++m_instructionState;
        } break;
        case 4: {
            // read from effective address
            m_operand = m_memory.read(m_instAddr);
            (this->*operation)();
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

void Cpu::rmwIdxInd(opFunc operation)
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // read from the address, add X to it
            m_instAddr = m_memory.read(m_instAddr) + m_xIndex;
            ++m_instructionState;
        } break;
        case 2: {
            // fetch effective address low
            m_operand = m_memory.read(m_instAddr++);
            ++m_instructionState;
        } break;
        case 3: {
            // fetch effective address high
            m_instAddr = m_memory.read(m_instAddr) << 8;
            m_instAddr |= m_operand;
            ++m_instructionState;
        } break;
        case 4: {
            // read from effective address
            m_operand = m_memory.read(m_instAddr);
            ++m_instructionState;
        } break;
        case 5: {
            // write the value back to effective address, and do the operation on it
            m_memory.write(m_instAddr, m_operand, 0);
            (this->*operation)();
            ++m_instructionState;
        } break;
        case 6: {
            // write the new value to effective address
            m_memory.write(m_instAddr, m_operand, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

void Cpu::wrIdxInd(const uint8_t val)
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // read from the address, add X to it
            m_instAddr = m_memory.read(m_instAddr) + m_xIndex;
            ++m_instructionState;
        } break;
        case 2: {
            // fetch effective address low
            m_operand = m_memory.read(m_instAddr++);
            ++m_instructionState;
        } break;
        case 3: {
            // fetch effective address high
            m_instAddr = m_memory.read(m_instAddr) << 8;
            m_instAddr |= m_operand;
            ++m_instructionState;
        } break;
        case 4: {
            // write to effective address
            m_memory.write(m_instAddr, val, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

// Indirect indexed addressing
void Cpu::rdIndIdx(opFunc operation)
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch effective address low
            m_operand = m_memory.read(m_instAddr++);
            ++m_instructionState;
        } break;
        case 2: {
            // fetch effective address high and add Y to the low byte
            m_instAddr = m_memory.read(m_instAddr) << 8;
            m_instAddr |= (m_operand + m_yIndex);
            ++m_instructionState;
        } break;
        case 3: {
            // read from address and fix if needed
            m_operand = m_memory.read(m_instAddr);
            if(m_yIndex > (m_instAddr & 0xFF)) {
                m_instAddr += 0x100;
                ++m_instructionState;
            } else {
                // perform operation
                m_operand = m_memory.read(m_instAddr);
                (this->*operation)();
                m_cpuState = CpuState::READ_NEXT_OPCODE;
                m_instructionState = 0;
            }
        } break;
        case 4: {
            // read from effective address
            m_operand = m_memory.read(m_instAddr);
            (this->*operation)();
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

void Cpu::rmwIndIdx(opFunc operation)
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch effective address low
            m_operand = m_memory.read(m_instAddr++);
            ++m_instructionState;
        } break;
        case 2: {
            // fetch effective address high and add Y to the low byte
            m_instAddr = m_memory.read(m_instAddr) << 8;
            m_instAddr |= (m_operand + m_yIndex);
            ++m_instructionState;
        } break;
        case 3: {
            // read from address and fix if needed
            m_operand = m_memory.read(m_instAddr);
            if(m_yIndex > (m_instAddr & 0xFF)) {
                m_instAddr += 0x100;
            }
            ++m_instructionState;
        } break;
        case 4: {
            // read from effective address
            m_operand = m_memory.read(m_instAddr);
            ++m_instructionState;
        } break;
        case 5: {
            // write the value back to effective address, and do the operation on it
            m_memory.write(m_instAddr, m_operand, 0);
            (this->*operation)();
            ++m_instructionState;
        } break;
        case 6: {
            // write the new value to effective address
            m_memory.write(m_instAddr, m_operand, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

void Cpu::wrIndIdx(const uint8_t val)
{
    bool debug = m_opcodeReadFrom == 0xEA0C;
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address, increment PC
            m_instAddr = m_memory.read(m_programCounter++);
            if(debug) {
                printf("0: m_instAddr = 0x%04X\n", m_instAddr);
            }
            ++m_instructionState;
        } break;
        case 1: {
            // fetch effective address low
            m_operand = m_memory.read(m_instAddr++);
            if(debug) {
                printf("1: m_operand = 0x%02X, m_instAddr = 0x%04X\n", m_operand, m_instAddr);
            }
            ++m_instructionState;
        } break;
        case 2: {
            // fetch effective address high and add Y to the low byte
            m_instAddr = m_memory.read(m_instAddr) << 8;
            if(debug) {
                printf("2a: m_instAddr = 0x%02X\n", m_instAddr);
            }
            m_instAddr |= ((m_operand + m_yIndex) & 0xFF);
            if(debug) {
                printf("2b: m_instAddr = 0x%04X\n", m_instAddr);
            }
            ++m_instructionState;
        } break;
        case 3: {
            // read from address and fix if needed
            m_operand = m_memory.read(m_instAddr);
            if(debug) {
                printf("3a: m_operand = 0x%02X, m_instAddr = 0x%04X\n", m_operand, m_instAddr);
            }
            if(m_yIndex > (m_instAddr & 0xFF)) {
                m_instAddr += 0x100;
            }
            if(debug) {
                printf("3b: m_operand = 0x%02X, m_instAddr = 0x%04X\n", m_operand, m_instAddr);
            }
            ++m_instructionState;
        } break;
        case 4: {
            // write to effective address
            if(debug) {
                printf("4: val = 0x%02X, m_instAddr = 0x%04X\n", val, m_instAddr);
            }
            m_memory.write(m_instAddr, val, 0);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

void Cpu::jmpInd()
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address low, increment PC
            m_operand = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // fetch pointer address high, increment PC
            m_instAddr = m_memory.read(m_programCounter++) << 8;
            m_instAddr |= m_operand;
            ++m_instructionState;
        } break;
        case 2: {
            m_operand = m_memory.read(m_instAddr++);
            ++m_instructionState;
        } break;
        case 3: {
            m_programCounter = m_memory.read(m_instAddr) << 8;
            m_programCounter |= m_operand;
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}  

void Cpu::adc()
{
    uint16_t result = ((uint16_t)m_accumulator) + m_operand + m_status.bits.carryFlag;
    m_status.bits.carryFlag = (result > 0xFF);
    m_accumulator = result & 0xFF;
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::andi()
{
    m_accumulator &= m_operand;
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::asl()
{
    bool newCarry = (m_operand & 0x80) > 0;
    m_operand <<= 1;
    m_status.bits.negativeFlag = (m_operand & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_operand);
    m_status.bits.carryFlag = newCarry;
}

void Cpu::bit()
{
    m_status.bits.negativeFlag = (m_operand & 0x80) > 0;
    m_status.bits.overflowFlag = (m_operand & 0x40) > 0;
    m_status.bits.zeroFlag = (0 == (m_accumulator & m_operand));
}

void Cpu::br(uint8_t flag, uint8_t condition)
{
    switch(m_instructionState) {
        case 0: {
            // fetch relative jump offset operand
            m_operand = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            if(flag == condition) {
                int8_t signedOperand = (int8_t)m_operand;
                m_programCounter += signedOperand;
                ++m_instructionState;
            } else {
                m_cpuState = CpuState::READ_NEXT_OPCODE;
                m_instructionState = 0;
            }
        } break;
        case 2: {
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
        } break;
    }
}

void Cpu::clc()
{
    m_status.bits.carryFlag = 0;
}

void Cpu::cld()
{
    m_status.bits.decimalModeFlag = 0;
}

void Cpu::cli()
{
    m_status.bits.interruptDisableFlag = 0;
}

void Cpu::clv()
{
    m_status.bits.overflowFlag = 0;
}

void Cpu::cmp()
{
    m_status.bits.carryFlag = (m_operand <= m_accumulator);
    m_status.bits.negativeFlag = ((m_accumulator - m_operand) & 0x80) > 0;
    m_status.bits.zeroFlag = (m_operand == m_accumulator);
}

void Cpu::cpy()
{
    m_status.bits.carryFlag = (m_operand <= m_yIndex);
    m_status.bits.negativeFlag = ((m_yIndex - m_operand) & 0x80) > 0;
    m_status.bits.zeroFlag = (m_operand == m_yIndex);
}

void Cpu::cpx()
{
    m_status.bits.carryFlag = (m_operand <= m_xIndex);
    m_status.bits.negativeFlag = ((m_xIndex - m_operand) & 0x80) > 0;
    m_status.bits.zeroFlag = (m_operand == m_xIndex);
}

void Cpu::dec()
{
    --m_operand;
    m_status.bits.negativeFlag = (m_operand & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_operand);
}

void Cpu::dex()
{
    --m_xIndex;
    m_status.bits.negativeFlag = (m_xIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_xIndex);
}

void Cpu::dey()
{
    --m_yIndex;
    m_status.bits.negativeFlag = (m_yIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_yIndex);
}

void Cpu::eor()
{
    m_accumulator ^= m_operand;
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::inc()
{
    ++m_operand;
    m_status.bits.negativeFlag = (m_operand & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_operand);
}

void Cpu::inx()
{
    ++m_xIndex;
    m_status.bits.negativeFlag = (m_xIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_xIndex);
}

void Cpu::iny()
{
    ++m_yIndex;
    m_status.bits.negativeFlag = (m_yIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_yIndex);
}

void Cpu::isr()
{
    m_pendingIrq = false;
    m_memory.write(m_stackPointer--, ((m_programCounter >> 8) & 0xFF), m_programCounter);
    m_memory.write(m_stackPointer--, (m_programCounter & 0xFF), m_programCounter);
    m_memory.write(m_stackPointer--, m_status.all, m_programCounter);
    m_programCounter = m_memory.readWord(0xFFFE); // read the location of the main ISR from ROM
    //printf("isr triggering ");
    //printStack();
}

void Cpu::jsr()
{
    switch(m_instructionState) {
        case 0: {
            // fetch subroutine address low, increment PC
            m_operand = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            // --m_stackPointer; not sure if this is right?
            ++m_instructionState;
        } break;
        case 2: {
            m_memory.write(m_stackPointer--, (m_programCounter >> 8), m_programCounter);
            ++m_instructionState;
        } break;
        case 3: {
            m_memory.write(m_stackPointer--, (m_programCounter & 0xFF), m_programCounter);
            ++m_instructionState;
        } break;
        case 4: {
            m_programCounter = (m_memory.read(m_programCounter) << 8 | m_operand);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
            //printf("jsr triggering ");
            //printStack();
        } break;
    }
}

void Cpu::lda()
{
    m_accumulator = m_operand;
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::ldx()
{
    m_xIndex = m_operand;
    m_status.bits.negativeFlag = (m_xIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_xIndex);
}

void Cpu::ldy()
{
    m_yIndex = m_operand;
    m_status.bits.negativeFlag = (m_yIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_yIndex);
}

void Cpu::lsr()
{
    bool newCarry = (m_operand & 0x01) > 0;
    m_operand >>= 1;
    m_status.bits.negativeFlag = (m_operand & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_operand);
    m_status.bits.carryFlag = newCarry;
}

void Cpu::nop()
{
    ++m_programCounter;
    m_cpuState = CpuState::READ_NEXT_OPCODE;
    m_instructionState = 0;
}

void Cpu::ora()
{
    m_accumulator |= m_operand;
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::pha()
{
    switch(m_instructionState) {
        case 0: {
            // fetch pointer address low, increment PC
            m_operand = m_memory.read(m_programCounter);
            ++m_instructionState;
        } break;
        case 1: {
            m_memory.write(m_stackPointer--, m_accumulator, m_programCounter);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
            //printf("pha triggering ");
            //printStack();
        } break;
    }
}

void Cpu::php()
{
    switch(m_instructionState) {
        case 0: {
            m_operand = m_memory.read(m_programCounter);
            ++m_instructionState;
        } break;
        case 1: {
            m_memory.write(m_stackPointer--, m_status.all, m_programCounter);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
            //printf("php triggering ");
            //printStack();
        } break;
    }
}

void Cpu::pla()
{
    switch(m_instructionState) {
        case 0: {
            m_operand = m_memory.read(m_programCounter);
            ++m_instructionState;
        } break;
        case 1: {
            ++m_stackPointer;
            ++m_instructionState;
        } break;
        case 2: {
            m_accumulator = m_memory.read(m_stackPointer);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
            //printf("pla triggering ");
            //printStack();
        } break;
    }
}

void Cpu::plp()
{
    switch(m_instructionState) {
        case 0: {
            m_operand = m_memory.read(m_programCounter);
            ++m_instructionState;
        } break;
        case 1: {
            ++m_stackPointer;
            ++m_instructionState;
        } break;
        case 2: {
            m_status.all = m_memory.read(m_stackPointer);
            m_cpuState = CpuState::READ_NEXT_OPCODE;
            m_instructionState = 0;
            //printf("plp triggering ");
            //printStack();
        } break;
    }
}

void Cpu::rol()
{
    bool newCarry = (m_operand & 0x80) > 0;
    m_operand <<= 1;
    m_operand += m_status.bits.carryFlag;
    m_status.bits.negativeFlag = (m_operand & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_operand);
    m_status.bits.carryFlag = newCarry;
}

void Cpu::ror()
{
    bool newCarry = (m_operand & 0x01) > 0;
    m_operand >>= 1;
    m_operand += m_status.bits.carryFlag ? 0x80 : 0x00;
    m_status.bits.negativeFlag = (m_operand & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_operand);
    m_status.bits.carryFlag = newCarry;
}

void Cpu::rti()
{
    switch(m_instructionState) {
        case 0: {
            m_operand = m_memory.read(m_programCounter++);
            ++m_instructionState;
        } break;
        case 1: {
            ++m_stackPointer;
            ++m_instructionState;
        } break;
        case 2: {
            m_status.all = m_memory.read(m_stackPointer++);
            ++m_instructionState;
        } break;
        case 3: {
            m_programCounter = m_memory.read(m_stackPointer++);
            ++m_instructionState;
        } break;
        case 4: {
            m_programCounter |= (m_memory.read(m_stackPointer) << 8);
            m_instructionState = 0;
            m_cpuState = READ_NEXT_OPCODE;
            //printf("rti triggering ");
            //printStack();
        } break;
    }
}

void Cpu::rts()
{
    switch(m_instructionState) {
        case 0: {
            m_operand = m_memory.read(m_programCounter);
            ++m_instructionState;
        } break;
        case 1: {
            ++m_stackPointer;
            ++m_instructionState;
        } break;
        case 2: {
            m_programCounter = m_memory.read(m_stackPointer++);
            ++m_instructionState;
        } break;
        case 3: {
            m_programCounter |= (m_memory.read(m_stackPointer) << 8);
            ++m_instructionState;
        } break;
        case 4: {
            ++m_programCounter;
            m_instructionState = 0;
            m_cpuState = READ_NEXT_OPCODE;
            //printf("rts triggering ");
            //printStack();
        } break;
    }
}

void Cpu::sbc()
{
    uint16_t op0 = m_operand;
    uint16_t op1 = m_accumulator;
    uint16_t tmp = 0;

    op0 = (~op0 & 0xFF) + m_status.bits.carryFlag;
    tmp = op0 + op1;

    m_status.bits.carryFlag = ((tmp & 0x100) == 0x100);
    m_status.bits.overflowFlag = (((op0 ^ tmp) & (op1 ^ tmp) & 0x80) == 0x80);

    m_accumulator = tmp & 0xFF;
    m_status.bits.negativeFlag = (m_accumulator & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_accumulator);
}

void Cpu::sec()
{
    m_status.bits.carryFlag = 1;
}

void Cpu::sed()
{
    m_status.bits.decimalModeFlag = 1;
}

void Cpu::sei()
{
    m_status.bits.interruptDisableFlag = 1;
}

void Cpu::tax()
{
    m_xIndex = m_accumulator;
    m_status.bits.negativeFlag = (m_xIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_xIndex);
}

void Cpu::tay()
{
    m_yIndex = m_accumulator;
    m_status.bits.negativeFlag = (m_yIndex & 0x80) > 0;
    m_status.bits.zeroFlag = (0 == m_yIndex);
}

void Cpu::tsx()
{
    m_xIndex = m_stackPointer & 0xFF;
}

void Cpu::txa()
{
    m_accumulator = m_xIndex;
}

void Cpu::txs()
{
    m_stackPointer = 0x100 | m_xIndex;
    //printf("txs triggering ");
    //printStack();
}

void Cpu::tya()
{
    m_accumulator = m_yIndex;
}

void Cpu::init()
{
    m_memory.write(0x0000, 0xFF, 0);
    m_memory.write(0x0001, 0x07, 0);
    canary1  = 0xAAAA;
    canary2  = 0xAAAA;
    canary3  = 0xAAAA;
    canary4  = 0xAAAA;
    canary5  = 0xAAAA;
    canary6  = 0xAAAA;
    canary7  = 0xAAAA;
    canary8  = 0xAAAA;
    canary9  = 0xAAAA;
    canary10 = 0xAAAA;
    canary11 = 0xAAAA;
    canary12 = 0xAAAA;
    canary13 = 0xAAAA;
    canary14 = 0xAAAA;
    canary15 = 0xAAAA;
    canary16 = 0xAAAA;
    canary17 = 0xAAAA;
    canary18 = 0xAAAA;
    canary19 = 0xAAAA;
    canary20 = 0xAAAA;
    canary21 = 0xAAAA;
    canary22 = 0xAAAA;
    canary23 = 0xAAAA;
    canary24 = 0xAAAA;
    m_programCounter = m_memory.readWord(0xFFFC);
    m_stackPointer = 0x1FF;

    m_cmdMap["read"] = &Cpu::dbgRead;
    m_cmdMap["rd"]   = &Cpu::dbgRead;
    m_cmdMap["r"]    = &Cpu::dbgRead;
    m_cmdMap["writ"] = &Cpu::dbgWrit;
    m_cmdMap["wr"]   = &Cpu::dbgWrit;
    m_cmdMap["w"]    = &Cpu::dbgWrit;
    m_cmdMap["sdmp"] = &Cpu::dbgSdmp;
    m_cmdMap["step"] = &Cpu::dbgStep;
    m_cmdMap["brka"] = &Cpu::dbgBrka;
    m_cmdMap["brkd"] = &Cpu::dbgBrkd;
    m_cmdMap["lsbp"] = &Cpu::dbgLsbp;
    m_cmdMap["seti"] = &Cpu::dbgSeti;
    m_cmdMap["clri"] = &Cpu::dbgClri;
    m_cmdMap["sreg"] = &Cpu::dbgSreg;
    m_cmdMap["pstk"] = &Cpu::dbgPstk;

    m_cpuState = CpuState::READ_NEXT_OPCODE;
}

Cpu::Cpu(MemoryController& memory)
    : m_memory(memory)
    , m_videoTimer(0)
    , m_pendingIrq(false)
    , m_totalCycleCount(0)
    , m_debugMode(false)
    , m_stepping(false)
{
    init();
}

Cpu::~Cpu()
{

}

int Cpu::addBreakpoint(uint16_t bpAddr)
{
    m_debugMode = true;
    m_breakpointSet.insert(bpAddr);
    return m_breakpointSet.size();
}

int Cpu::removeBreakpoint(uint16_t bpAddr)
{
    m_breakpointSet.erase(bpAddr);
    return m_breakpointSet.size();
}

void Cpu::setDebugState(bool mode)
{
    m_debugMode = mode;
}

MemoryController& Cpu::getMemory()
{
    return m_memory;
}

std::string Cpu::decodeInstruction(uint16_t addr)
{
    uint8_t op0 = m_memory.read(addr);
    uint8_t op1 = m_memory.read(addr + 1);
    uint8_t op2 = m_memory.read(addr + 2);
    int8_t sop1 = (int8_t)op1;
    char output[64] = { 0 };
    switch (op0) {
        case ADC_abs: sprintf(output, "ADC $%02X%02X", op2, op1);       break;
        case ADC_abx: sprintf(output, "ADC $%02X%02X,X", op2, op1);     break;
        case ADC_aby: sprintf(output, "ADC $%02X%02X,Y", op2, op1);     break;
        case ADC_imm: sprintf(output, "ADC #$%02X", op1);               break;
        case ADC_izx: sprintf(output, "ADC ($%02X,X)", op1);            break;
        case ADC_izy: sprintf(output, "ADC ($%02X),Y", op1);            break;
        case ADC_zp:  sprintf(output, "ADC $%02X", op1);                break;
        case ADC_zpx: sprintf(output, "ADC $%02X,X", op1);              break;
        case AHX_aby: sprintf(output, "AHX $%02X%02X,Y", op2, op1);     break;
        case AHX_izy: sprintf(output, "AHX ($%02X),Y", op1);            break;
        case ALR_imm: sprintf(output, "ALR #$%02X", op1);               break;
        case ANC_im2: sprintf(output, "ANC_im2");                       break;
        case ANC_imm: sprintf(output, "ANC #$%02X", op1);               break;
        case AND_abs: sprintf(output, "AND $%02X%02X", op2, op1);       break;
        case AND_abx: sprintf(output, "AND $%02X%02X,X", op2, op1);     break;
        case AND_aby: sprintf(output, "AND $%02X%02X,Y", op2, op1);     break;
        case AND_imm: sprintf(output, "AND #$%02X", op1);               break;
        case AND_izx: sprintf(output, "AND ($%02X,X)", op1);            break;
        case AND_izy: sprintf(output, "AND ($%02X),Y", op1);            break;
        case AND_zp:  sprintf(output, "AND $%02X", op1);                break;
        case AND_zpx: sprintf(output, "AND $%02X,X", op1);              break;
        case ARR_imm: sprintf(output, "ARR #$%02X", op1);               break;
        case ASL:     sprintf(output, "ASL");                           break;
        case ASL_abs: sprintf(output, "ASL $%02X%02X", op2, op1);       break;
        case ASL_abx: sprintf(output, "ASL $%02X%02X,X", op2, op1);     break;
        case ASL_zp:  sprintf(output, "ASL $%02X", op1);                break;
        case ASL_zpx: sprintf(output, "ASL $%02X,X", op1);              break;
        case AXS_imm: sprintf(output, "AXS #$%02X", op1);               break;
        case BCC_rel: sprintf(output, "BCC $%04X", (addr + 2 + sop1));  break;
        case BCS_rel: sprintf(output, "BCS $%04X", (addr + 2 + sop1));  break;
        case BEQ_rel: sprintf(output, "BEQ $%04X", (addr + 2 + sop1));  break;
        case BIT_abs: sprintf(output, "BIT $%02X%02X", op2, op1);       break;
        case BIT_zp:  sprintf(output, "BIT $%02X", op1);                break;
        case BMI_rel: sprintf(output, "BMI $%04X", (addr + 2 + sop1));  break;
        case BNE_rel: sprintf(output, "BNE $%04X", (addr + 2 + sop1));  break;
        case BPL_rel: sprintf(output, "BPL $%04X", (addr + 2 + sop1));  break;
        case BRK:     sprintf(output, "BRK");                           break;
        case BVC_rel: sprintf(output, "BVC $%04X", (addr + 2 + sop1));  break;
        case BVS_rel: sprintf(output, "BVS $%04X", (addr + 2 + sop1));  break;
        case CLC:     sprintf(output, "CLC");                           break;
        case CLD:     sprintf(output, "CLD");                           break;
        case CLI:     sprintf(output, "CLI");                           break;
        case CLV:     sprintf(output, "CLV");                           break;
        case CMP_abs: sprintf(output, "CMP $%02X%02X", op2, op1);       break;
        case CMP_abx: sprintf(output, "CMP $%02X%02X,X", op2, op1);     break;
        case CMP_aby: sprintf(output, "CMP $%02X%02X,Y", op2, op1);     break;
        case CMP_imm: sprintf(output, "CMP #$%02X", op1);               break;
        case CMP_izx: sprintf(output, "CMP ($%02X,X)", op1);            break;
        case CMP_izy: sprintf(output, "CMP ($%02X),Y", op1);            break;
        case CMP_zp:  sprintf(output, "CMP $%02X", op1);                break;
        case CMP_zpx: sprintf(output, "CMP $%02X,X", op1);              break;
        case CPX_abs: sprintf(output, "CPX $%02X%02X", op2, op1);       break;
        case CPX_imm: sprintf(output, "CPX #$%02X", op1);               break;
        case CPX_zp:  sprintf(output, "CPX $%02X", op1);                break;
        case CPY_abs: sprintf(output, "CPY $%02X%02X", op2, op1);       break;
        case CPY_imm: sprintf(output, "CPY #$%02X", op1); break;
        case CPY_zp:  sprintf(output, "CPY $%02X", op1); break;
        case DCP_abs: sprintf(output, "DCP $%02X%02X", op2, op1); break;
        case DCP_abx: sprintf(output, "DCP $%02X%02X,X", op2, op1); break;
        case DCP_aby: sprintf(output, "DCP $%02X%02X,Y", op2, op1); break;
        case DCP_izx: sprintf(output, "DCP ($%02X,X)", op1); break;
        case DCP_izy: sprintf(output, "DCP ($%02X),Y", op1); break;
        case DCP_zp:  sprintf(output, "DCP $%02X", op1); break;
        case DCP_zpx: sprintf(output, "DCP $%02X,X", op1); break;
        case DEC_abs: sprintf(output, "DEC $%02X%02X", op2, op1); break;
        case DEC_abx: sprintf(output, "DEC $%02X%02X,X", op2, op1); break;
        case DEC_zp:  sprintf(output, "DEC $%02X", op1); break;
        case DEC_zpx: sprintf(output, "DEC $%02X,X", op1); break;
        case DEX:     sprintf(output, "DEX"); break;
        case DEY:     sprintf(output, "DEY"); break;
        case EOR_abs: sprintf(output, "EOR $%02X%02X", op2, op1); break;
        case EOR_abx: sprintf(output, "EOR $%02X%02X,X", op2, op1); break;
        case EOR_aby: sprintf(output, "EOR $%02X%02X,Y", op2, op1); break;
        case EOR_imm: sprintf(output, "EOR #$%02X", op1); break;
        case EOR_izx: sprintf(output, "EOR ($%02X,X)", op1); break;
        case EOR_izy: sprintf(output, "EOR ($%02X),Y", op1); break;
        case EOR_zp:  sprintf(output, "EOR $%02X", op1); break;
        case EOR_zpx: sprintf(output, "EOR $%02X,X", op1); break;
        case INC_abs: sprintf(output, "INC $%02X%02X", op2, op1); break;
        case INC_abx: sprintf(output, "INC $%02X%02X,X", op2, op1); break;
        case INC_zp:  sprintf(output, "INC $%02X", op1); break;
        case INC_zpx: sprintf(output, "INC $%02X,X", op1); break;
        case INX:     sprintf(output, "INX"); break;
        case INY:     sprintf(output, "INY"); break;
        case ISC_abs: sprintf(output, "ISC $%02X%02X", op2, op1); break;
        case ISC_abx: sprintf(output, "ISC $%02X%02X,X", op2, op1); break;
        case ISC_aby: sprintf(output, "ISC $%02X%02X,Y", op2, op1); break;
        case ISC_izx: sprintf(output, "ISC ($%02X,X)", op1); break;
        case ISC_izy: sprintf(output, "ISC ($%02X),Y", op1); break;
        case ISC_zp:  sprintf(output, "ISC $%02X", op1); break;
        case ISC_zpx: sprintf(output, "ISC $%02X,X", op1); break;
        case JMP_abs: sprintf(output, "JMP $%02X%02X", op2, op1); break;
        case JMP_ind: sprintf(output, "JMP_ind"); break;
        case JSR:     sprintf(output, "JSR $%02X%02X", op2, op1); break;
        case KIL0:    sprintf(output, "KIL0"); break;
        case KIL1:    sprintf(output, "KIL1"); break;
        case KIL2:    sprintf(output, "KIL2"); break;
        case KIL3:    sprintf(output, "KIL3"); break;
        case KIL4:    sprintf(output, "KIL4"); break;
        case KIL5:    sprintf(output, "KIL5"); break;
        case KIL6:    sprintf(output, "KIL6"); break;
        case KIL7:    sprintf(output, "KIL7"); break;
        case KIL9:    sprintf(output, "KIL9"); break;
        case KILB:    sprintf(output, "KILB"); break;
        case KILD:    sprintf(output, "KILD"); break;
        case KILF:    sprintf(output, "KILF"); break;
        case LAS_aby: sprintf(output, "LAS $%02X%02X,Y", op2, op1); break;
        case LAX_abs: sprintf(output, "LAX $%02X%02X", op2, op1); break;
        case LAX_aby: sprintf(output, "LAX $%02X%02X,Y", op2, op1); break;
        case LAX_imm: sprintf(output, "LAX #$%02X", op1); break;
        case LAX_izx: sprintf(output, "LAX ($%02X,X)", op1); break;
        case LAX_izy: sprintf(output, "LAX ($%02X),Y", op1); break;
        case LAX_zp:  sprintf(output, "LAX $%02X", op1); break;
        case LAX_zpy: sprintf(output, "LAX $%02X,Y", op1); break;
        case LDA_abs: sprintf(output, "LDA $%02X%02X", op2, op1); break;
        case LDA_abx: sprintf(output, "LDA $%02X%02X,X", op2, op1); break;
        case LDA_aby: sprintf(output, "LDA $%02X%02X,Y", op2, op1); break;
        case LDA_imm: sprintf(output, "LDA #$%02X", op1); break;
        case LDA_izx: sprintf(output, "LDA ($%02X,X)", op1); break;
        case LDA_izy: sprintf(output, "LDA ($%02X),Y", op1); break;
        case LDA_zp:  sprintf(output, "LDA $%02X", op1); break;
        case LDA_zpx: sprintf(output, "LDA $%02X,X", op1); break;
        case LDX_abs: sprintf(output, "LDX $%02X%02X", op2, op1); break;
        case LDX_aby: sprintf(output, "LDX $%02X%02X,Y", op2, op1); break;
        case LDX_imm: sprintf(output, "LDX #$%02X", op1); break;
        case LDX_zp:  sprintf(output, "LDX $%02X", op1); break;
        case LDX_zpy: sprintf(output, "LDX $%02X,Y", op1); break;
        case LDY_abs: sprintf(output, "LDY $%02X%02X", op2, op1); break;
        case LDY_abx: sprintf(output, "LDY $%02X%02X,X", op2, op1); break;
        case LDY_imm: sprintf(output, "LDY #$%02X", op1); break;
        case LDY_zp:  sprintf(output, "LDY $%02X", op1); break;
        case LDY_zpx: sprintf(output, "LDY $%02X,X", op1); break;
        case LSR:     sprintf(output, "LSR"); break;
        case LSR_abs: sprintf(output, "LSR $%02X%02X", op2, op1); break;
        case LSR_abx: sprintf(output, "LSR $%02X%02X,X", op2, op1); break;
        case LSR_zp:  sprintf(output, "LSR $%02X", op1); break;
        case LSR_zpx: sprintf(output, "LSR $%02X,X", op1); break;
        case NOP1:    sprintf(output, "NOP1"); break;
        case NOP3:    sprintf(output, "NOP3"); break;
        case NOP5:    sprintf(output, "NOP5"); break;
        case NOP7:    sprintf(output, "NOP7"); break;
        case NOPD:    sprintf(output, "NOPD"); break;
        case NOPE:    sprintf(output, "NOPE"); break;
        case NOPF:    sprintf(output, "NOPF"); break;
        case NOP_ab1: sprintf(output, "NOP_ab1"); break;
        case NOP_ab3: sprintf(output, "NOP_ab3"); break;
        case NOP_ab5: sprintf(output, "NOP_ab5"); break;
        case NOP_ab7: sprintf(output, "NOP_ab7"); break;
        case NOP_abD: sprintf(output, "NOP_abD"); break;
        case NOP_abF: sprintf(output, "NOP_abF"); break;
        case NOP_abs: sprintf(output, "NOP $%02X%02X", op2, op1); break;
        case NOP_im2: sprintf(output, "NOP_im2"); break;
        case NOP_im3: sprintf(output, "NOP_im3"); break;
        case NOP_im4: sprintf(output, "NOP_im4"); break;
        case NOP_im5: sprintf(output, "NOP_im5"); break;
        case NOP_imm: sprintf(output, "NOP #$%02X", op1); break;
        case NOP_zp0: sprintf(output, "NOP_zp0"); break;
        case NOP_zp1: sprintf(output, "NOP_zp1"); break;
        case NOP_zp3: sprintf(output, "NOP_zp3"); break;
        case NOP_zp4: sprintf(output, "NOP_zp4"); break;
        case NOP_zp5: sprintf(output, "NOP_zp5"); break;
        case NOP_zp6: sprintf(output, "NOP_zp6"); break;
        case NOP_zp7: sprintf(output, "NOP_zp7"); break;
        case NOP_zpD: sprintf(output, "NOP_zpD"); break;
        case NOP_zpF: sprintf(output, "NOP_zpF"); break;
        case ORA_abs: sprintf(output, "ORA $%02X%02X", op2, op1); break;
        case ORA_abx: sprintf(output, "ORA $%02X%02X,X", op2, op1); break;
        case ORA_aby: sprintf(output, "ORA $%02X%02X,Y", op2, op1); break;
        case ORA_imm: sprintf(output, "ORA #$%02X", op1); break;
        case ORA_izx: sprintf(output, "ORA ($%02X,X)", op1); break;
        case ORA_izy: sprintf(output, "ORA ($%02X),Y", op1); break;
        case ORA_zp:  sprintf(output, "ORA $%02X", op1); break;
        case ORA_zpx: sprintf(output, "ORA $%02X,X", op1); break;
        case PHA:     sprintf(output, "PHA"); break;
        case PHP:     sprintf(output, "PHP"); break;
        case PLA:     sprintf(output, "PLA"); break;
        case PLP:     sprintf(output, "PLP"); break;
        case RLA_abs: sprintf(output, "RLA $%02X%02X", op2, op1); break;
        case RLA_abx: sprintf(output, "RLA $%02X%02X,X", op2, op1); break;
        case RLA_aby: sprintf(output, "RLA $%02X%02X,Y", op2, op1); break;
        case RLA_izx: sprintf(output, "RLA ($%02X,X)", op1); break;
        case RLA_izy: sprintf(output, "RLA ($%02X),Y", op1); break;
        case RLA_zp:  sprintf(output, "RLA $%02X", op1); break;
        case RLA_zpx: sprintf(output, "RLA $%02X,X", op1); break;
        case ROL:     sprintf(output, "ROL"); break;
        case ROL_abs: sprintf(output, "ROL $%02X%02X", op2, op1); break;
        case ROL_abx: sprintf(output, "ROL $%02X%02X,X", op2, op1); break;
        case ROL_zp:  sprintf(output, "ROL $%02X", op1); break;
        case ROL_zpx: sprintf(output, "ROL $%02X,X", op1); break;
        case ROR:     sprintf(output, "ROR"); break;
        case ROR_abs: sprintf(output, "ROR $%02X%02X", op2, op1); break;
        case ROR_abx: sprintf(output, "ROR $%02X%02X,X", op2, op1); break;
        case ROR_zp:  sprintf(output, "ROR $%02X", op1); break;
        case ROR_zpx: sprintf(output, "ROR $%02X,X", op1); break;
        case RRA_abs: sprintf(output, "RRA $%02X%02X", op2, op1); break;
        case RRA_abx: sprintf(output, "RRA $%02X%02X,X", op2, op1); break;
        case RRA_aby: sprintf(output, "RRA $%02X%02X,Y", op2, op1); break;
        case RRA_izx: sprintf(output, "RRA ($%02X,X)", op1); break;
        case RRA_izy: sprintf(output, "RRA ($%02X),Y", op1); break;
        case RRA_zp:  sprintf(output, "RRA $%02X", op1); break;
        case RRA_zpx: sprintf(output, "RRA $%02X,X", op1); break;
        case RTI:     sprintf(output, "RTI"); break;
        case RTS:     sprintf(output, "RTS"); break;
        case SAX_abs: sprintf(output, "SAX $%02X%02X", op2, op1); break;
        case SAX_izx: sprintf(output, "SAX ($%02X,X)", op1); break;
        case SAX_zpy: sprintf(output, "SAX $%02X,Y", op1); break;
        case SBC_abs: sprintf(output, "SBC $%02X%02X", op2, op1); break;
        case SBC_abx: sprintf(output, "SBC $%02X%02X,X", op2, op1); break;
        case SBC_aby: sprintf(output, "SBC $%02X%02X,Y", op2, op1); break;
        case SBC_im2: sprintf(output, "SBC_im2"); break;
        case SBC_imm: sprintf(output, "SBC #$%02X", op1); break;
        case SBC_izx: sprintf(output, "SBC ($%02X,X)", op1); break;
        case SBC_izy: sprintf(output, "SBC ($%02X),Y", op1); break;
        case SBC_zp:  sprintf(output, "SBC $%02X", op1); break;
        case SBC_zpx: sprintf(output, "SBC $%02X,X", op1); break;
        case SEC:     sprintf(output, "SEC"); break;
        case SED:     sprintf(output, "SED"); break;
        case SEI:     sprintf(output, "SEI"); break;
        case SHX_aby: sprintf(output, "SHX $%02X%02X,Y", op2, op1); break;
        case SHY_abx: sprintf(output, "SHY $%02X%02X,X", op2, op1); break;
        case SLO_abs: sprintf(output, "SLO $%02X%02X", op2, op1); break;
        case SLO_abx: sprintf(output, "SLO $%02X%02X,X", op2, op1); break;
        case SLO_aby: sprintf(output, "SLO $%02X%02X,Y", op2, op1); break;
        case SLO_izx: sprintf(output, "SLO ($%02X,X)", op1); break;
        case SLO_izy: sprintf(output, "SLO ($%02X),Y", op1); break;
        case SLO_zp:  sprintf(output, "SLO $%02X", op1); break;
        case SLO_zpx: sprintf(output, "SLO $%02X,X", op1); break;
        case SRE_abs: sprintf(output, "SRE $%02X%02X", op2, op1); break;
        case SRE_abx: sprintf(output, "SRE $%02X%02X,X", op2, op1); break;
        case SRE_aby: sprintf(output, "SRE $%02X%02X,Y", op2, op1); break;
        case SRE_izx: sprintf(output, "SRE ($%02X,X)", op1); break;
        case SRE_izy: sprintf(output, "SRE ($%02X),Y", op1); break;
        case SRE_zp4: sprintf(output, "SRE_zp4"); break;
        case SRE_zp8: sprintf(output, "SRE_zp8"); break;
        case SRE_zpx: sprintf(output, "SRE $%02X,X", op1); break;
        case STA_abs: sprintf(output, "STA $%02X%02X", op2, op1); break;
        case STA_abx: sprintf(output, "STA $%02X%02X,X", op2, op1); break;
        case STA_aby: sprintf(output, "STA $%02X%02X,Y", op2, op1); break;
        case STA_izx: sprintf(output, "STA ($%02X,X)", op1); break;
        case STA_izy: sprintf(output, "STA ($%02X),Y", op1); break;
        case STA_zp3: sprintf(output, "STA_zp3"); break;
        case STA_zpx: sprintf(output, "STA $%02X,X", op1); break;
        case STX_abs: sprintf(output, "STX $%02X%02X", op2, op1); break;
        case STX_zp3: sprintf(output, "STX_zp3"); break;
        case STX_zpx: sprintf(output, "STX $%02X,X", op1); break;
        case STY_abs: sprintf(output, "STY $%02X%02X", op2, op1); break;
        case STY_zp3: sprintf(output, "STY_zp3"); break;
        case STY_zpx: sprintf(output, "STY $%02X,X", op1); break;
        case TAS_aby: sprintf(output, "TAS $%02X%02X,Y", op2, op1); break;
        case TAX:     sprintf(output, "TAX"); break;
        case TAY:     sprintf(output, "TAY"); break;
        case TSX:     sprintf(output, "TSX"); break;
        case TXA:     sprintf(output, "TXA"); break;
        case TXS:     sprintf(output, "TXS"); break;
        case TYA:     sprintf(output, "TYA"); break;
        case XAA_imm: sprintf(output, "XAA #$%02X", op1); break;
        default:      sprintf(output, "unknown"); break;
    }
    return std::string(output);
}

void Cpu::printStack()
{
    printf("stack dump (PC: 0x%04X)\n======================\n", m_opcodeReadFrom);
    for(uint16_t idx = 0x1FF; idx > m_stackPointer; --idx) {
        printf("0x%04X: %02X\n", idx, m_memory.read(idx));
    }
}

} // namespace MOS6510
