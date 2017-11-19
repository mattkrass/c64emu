#ifndef INCLUDED_MOS6510_H
#define INCLUDED_MOS6510_H

#include <map>
#include <set>
#include <vector>
#include "memorycontroller.h"

namespace MOS6510 {
const int SCREEN_WIDTH  = 960;//768;
const int SCREEN_HEIGHT = 680;// 544;
const uint32_t BG_COLOR = 0xFF9083EC;
const uint32_t FG_COLOR = 0xFFAAFFEE;

struct StatusRegister {
    union {
        struct {
            uint8_t carryFlag:1;
            uint8_t zeroFlag:1;
            uint8_t interruptDisableFlag:1;
            uint8_t decimalModeFlag:1;
            uint8_t breakFlag:1;
            uint8_t unusedFlag:1;
            uint8_t overflowFlag:1;
            uint8_t negativeFlag:1;
        } bits;
        uint8_t all;
    };
};

enum AddrMode
{
    IMP,
    IMM,
    REL,
    ZP,
    ZPX,
    ZPY,
    ABS,
    ABX,
    ABY,
    IND,
    IZX,
    IZY
};

enum Instructions
{
    // 0x00
    BRK     = 0x00, ORA_izx = 0x01, KIL0    = 0x02, SLO_izx = 0x03,
    NOP_zp0 = 0x04, ORA_zp  = 0x05, ASL_zp  = 0x06, SLO_zp  = 0x07,
    PHP     = 0x08, ORA_imm = 0x09, ASL     = 0x0A, ANC_imm = 0x0B,
    NOP_abs = 0x0C, ORA_abs = 0x0D, ASL_abs = 0x0E, SLO_abs = 0x0F,

    // 0x10
    BPL_rel = 0x10, ORA_izy = 0x11, KIL1    = 0x12, SLO_izy = 0x13,
    NOP_zp1 = 0x14, ORA_zpx = 0x15, ASL_zpx = 0x16, SLO_zpx = 0x17,
    CLC     = 0x18, ORA_aby = 0x19, NOP1    = 0x1A, SLO_aby = 0x1B,
    NOP_ab1 = 0x1C, ORA_abx = 0x1D, ASL_abx = 0x1E, SLO_abx = 0x1F,

    // 0x20
    JSR     = 0x20, AND_izx = 0x21, KIL2    = 0x22, RLA_izx = 0x23,
    BIT_zp  = 0x24, AND_zp  = 0x25, ROL_zp  = 0x26, RLA_zp  = 0x27,
    PLP     = 0x28, AND_imm = 0x29, ROL     = 0x2A, ANC_im2 = 0x2B,
    BIT_abs = 0x2C, AND_abs = 0x2D, ROL_abs = 0x2E, RLA_abs = 0x2F,

    // 0x30
    BMI_rel = 0x30, AND_izy = 0x31, KIL3    = 0x32, RLA_izy = 0x33,
    NOP_zp3 = 0x34, AND_zpx = 0x35, ROL_zpx = 0x36, RLA_zpx = 0x37,
    SEC     = 0x38, AND_aby = 0x39, NOP3    = 0x3A, RLA_aby = 0x3B,
    NOP_ab3 = 0x3C, AND_abx = 0x3D, ROL_abx = 0x3E, RLA_abx = 0x3F, 

    // 0x40
    RTI     = 0x40, EOR_izx = 0x41, KIL4    = 0x42, SRE_izx = 0x43,
    NOP_zp4 = 0x44, EOR_zp  = 0x45, LSR_zp  = 0x46, SRE_zp4 = 0x47,
    PHA     = 0x48, EOR_imm = 0x49, LSR     = 0x4A, ALR_imm = 0x4B,
    JMP_abs = 0x4C, EOR_abs = 0x4D, LSR_abs = 0x4E, SRE_abs = 0x4F,

    // 0x50
    BVC_rel = 0x50, EOR_izy = 0x51, KIL5    = 0x52, SRE_izy = 0x53,
    NOP_zp5 = 0x54, EOR_zpx = 0x55, LSR_zpx = 0x56, SRE_zpx = 0x57,
    CLI     = 0x58, EOR_aby = 0x59, NOP5    = 0x5A, SRE_aby = 0x5B,
    NOP_ab5 = 0x5C, EOR_abx = 0x5D, LSR_abx = 0x5E, SRE_abx = 0x5F,
    
    // 0x60
    RTS     = 0x60, ADC_izx = 0x61, KIL6    = 0x62, RRA_izx = 0x63,
    NOP_zp6 = 0x64, ADC_zp  = 0x65, ROR_zp  = 0x66, RRA_zp  = 0x67,
    PLA     = 0x68, ADC_imm = 0x69, ROR     = 0x6A, ARR_imm = 0x6B,
    JMP_ind = 0x6C, ADC_abs = 0x6D, ROR_abs = 0x6E, RRA_abs = 0x6F,
    
    // 0x70
    BVS_rel = 0x70, ADC_izy = 0x71, KIL7    = 0x72, RRA_izy = 0x73,
    NOP_zp7 = 0x74, ADC_zpx = 0x75, ROR_zpx = 0x76, RRA_zpx = 0x77,
    SEI     = 0x78, ADC_aby = 0x79, NOP7    = 0x7A, RRA_aby = 0x7B,
    NOP_ab7 = 0x7C, ADC_abx = 0x7D, ROR_abx = 0x7E, RRA_abx = 0x7F,
    
    // 0x80
    NOP_imm = 0x80, STA_izx = 0x81, NOP_im2 = 0x82, SAX_izx = 0x83,
    STY_zp3 = 0x84, STA_zp3 = 0x85, STX_zp3 = 0x86, SRE_zp8 = 0x87,
    DEY     = 0x88, NOP_im3 = 0x89, TXA     = 0x8A, XAA_imm = 0x8B,
    STY_abs = 0x8C, STA_abs = 0x8D, STX_abs = 0x8E, SAX_abs = 0x8F,
    
    // 0x90
    BCC_rel = 0x90, STA_izy = 0x91, KIL9    = 0x92, AHX_izy = 0x93,
    STY_zpx = 0x94, STA_zpx = 0x95, STX_zpx = 0x96, SAX_zpy = 0x97,
    TYA     = 0x98, STA_aby = 0x99, TXS     = 0x9A, TAS_aby = 0x9B,
    SHY_abx = 0x9C, STA_abx = 0x9D, SHX_aby = 0x9E, AHX_aby = 0x9F,
    
    // 0xA0
    LDY_imm = 0xA0, LDA_izx = 0xA1, LDX_imm = 0xA2, LAX_izx = 0xA3,
    LDY_zp  = 0xA4, LDA_zp  = 0xA5, LDX_zp  = 0xA6, LAX_zp  = 0xA7,
    TAY     = 0xA8, LDA_imm = 0xA9, TAX     = 0xAA, LAX_imm = 0xAB,
    LDY_abs = 0xAC, LDA_abs = 0xAD, LDX_abs = 0xAE, LAX_abs = 0xAF,
    
    // 0xB0
    BCS_rel = 0xB0, LDA_izy = 0xB1, KILB    = 0xB2, LAX_izy = 0xB3,
    LDY_zpx = 0xB4, LDA_zpx = 0xB5, LDX_zpy = 0xB6, LAX_zpy = 0xB7,
    CLV     = 0xB8, LDA_aby = 0xB9, TSX     = 0xBA, LAS_aby = 0xBB,
    LDY_abx = 0xBC, LDA_abx = 0xBD, LDX_aby = 0xBE, LAX_aby = 0xBF,
    
    // 0xC0
    CPY_imm = 0xC0, CMP_izx = 0xC1, NOP_im4 = 0xC2, DCP_izx = 0xC3,
    CPY_zp  = 0xC4, CMP_zp  = 0xC5, DEC_zp  = 0xC6, DCP_zp  = 0xC7,
    INY     = 0xC8, CMP_imm = 0xC9, DEX     = 0xCA, AXS_imm = 0xCB,
    CPY_abs = 0xCC, CMP_abs = 0xCD, DEC_abs = 0xCE, DCP_abs = 0xCF,
    
    // 0xD0
    BNE_rel = 0xD0, CMP_izy = 0xD1, KILD    = 0xD2, DCP_izy = 0xD3,
    NOP_zpD = 0xD4, CMP_zpx = 0xD5, DEC_zpx = 0xD6, DCP_zpx = 0xD7,
    CLD     = 0xD8, CMP_aby = 0xD9, NOPD    = 0xDA, DCP_aby = 0xDB,
    NOP_abD = 0xDC, CMP_abx = 0xDD, DEC_abx = 0xDE, DCP_abx = 0xDF,
    
    // 0xE0
    CPX_imm = 0xE0, SBC_izx = 0xE1, NOP_im5 = 0xE2, ISC_izx = 0xE3,
    CPX_zp  = 0xE4, SBC_zp  = 0xE5, INC_zp  = 0xE6, ISC_zp  = 0xE7,
    INX     = 0xE8, SBC_imm = 0xE9, NOPE    = 0xEA, SBC_im2 = 0xEB,
    CPX_abs = 0xEC, SBC_abs = 0xED, INC_abs = 0xEE, ISC_abs = 0xEF,
    
    // 0xF0
    BEQ_rel = 0xF0, SBC_izy = 0xF1, KILF    = 0xF2, ISC_izy = 0xF3,
    NOP_zpF = 0xF4, SBC_zpx = 0xF5, INC_zpx = 0xF6, ISC_zpx = 0xF7,
    SED     = 0xF8, SBC_aby = 0xF9, NOPF    = 0xFA, ISC_aby = 0xFB,
    NOP_abF = 0xFC, SBC_abx = 0xFD, INC_abx = 0xFE, ISC_abx = 0xFF,
};

enum CpuState {
    // TODO: fill these in!
    // Read next instruction from memory at the PC (program counter)
    // Process instruction
    // Process Timers on CIA1 and CIA2 (not covered in this article)
    // Update screen via VIC-II (not covered in this article)
    // Calculate cycles (for emulators that aren’t cycle-exact)
};

class Cpu {
private:
    uint16_t                        m_programCounter;
    uint16_t                        m_stackPointer;
    uint8_t                         m_accumulator;
    uint8_t                         m_xIndex;
    uint8_t                         m_yIndex;
    StatusRegister                  m_status;
    MemoryController                m_memory;
    uint16_t                        m_videoTimer;
    uint16_t                        m_sysTimer;
    bool                            m_pendingIrq;

    // debug state
    typedef bool (MOS6510::Cpu::* cmdFunc)(const std::vector<std::string>&);
    bool                            m_debugMode;
    std::set<uint16_t>              m_breakpointSet;
    std::map<std::string, cmdFunc>  m_cmdMap;
    uint16_t                        m_stepCount;
    bool                            m_stepping;
    uint8_t*                        m_cgromPtr;
    SDL_Window *                    m_window;
    SDL_Surface *                   m_surface;

    // internal operations 
    void adc(const AddrMode mode);
    void andi(const AddrMode mode);
    void asl(const AddrMode mode);
    void bit(const AddrMode mode);
    void br(uint8_t flag, uint8_t condition);
    void clc();
    void cld();
    void cli();
    void clv();
    void cmp(uint8_t r, const AddrMode mode);
    void dec(const AddrMode mode);
    void der(uint8_t& r);
    void eor(const AddrMode mode);
    void inc(const AddrMode mode);
    void inr(uint8_t& r);
    void isr();
    void jmp(const AddrMode mode);
    void jsr();
    void ldr(uint8_t & r, const AddrMode mode);
    void lsr(const AddrMode mode);
    void nop();
    void ora(const AddrMode mode);
    void pha();
    void php();
    void pla();
    void plp();
    void rol(const AddrMode mode);
    void ror(const AddrMode mode);
    void rti();
    void rts();
    void sbc(const AddrMode mode);
    void sec();
    void sed();
    void sei();
    void str(const uint8_t &r, const AddrMode mode);
    void tsd(const uint8_t &src, uint8_t &dst);
    void tsx();
    void txs();

    // utility
    void init();
    uint16_t computeAddress(const AddrMode mode);
    void redrawScreen();

    // debug functions
    void debugPrompt();
    bool dbgRead(const std::vector<std::string>& args);
    bool dbgWrit(const std::vector<std::string>& args);
    bool dbgSdmp(const std::vector<std::string>& args);
    bool dbgStep(const std::vector<std::string>& args);
    bool dbgBrka(const std::vector<std::string>& args);
    bool dbgBrkd(const std::vector<std::string>& args);
    bool dbgLsbp(const std::vector<std::string>& args);
    bool dbgSeti(const std::vector<std::string>& args);
    bool dbgClri(const std::vector<std::string>& args);

public:
    Cpu(const Cpu& rhs);
    Cpu(uint8_t *romPtr, uint8_t *cgromPtr);
    ~Cpu();

    int addBreakpoint(uint16_t bpAddr);
    int removeBreakpoint(uint16_t bpAddr);
    void setDebugState(bool mode);

    void execute(bool debugBreak);
    void injectKeycode(uint8_t keycode); // <-- super temporary, I swear
    MemoryController& getMemory();
};
}
#endif // INCLUDED_MOS6510
