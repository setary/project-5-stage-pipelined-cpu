#ifndef __PIPELINED_CPU_H__
#define __PIPELINED_CPU_H__

#include "DigitalCircuit.h"

#include "Memory.h"
#include "Control.h"
#include "RegisterFile.h"
#include "ALU.h"
#include "ALUControl.h"

#include "Miscellaneous.h"

#ifdef ENABLE_DATA_FORWARDING
class ForwardingUnit : public DigitalCircuit {
  public:
    ForwardingUnit(
      const std::string &name,
      const Wire<5> *iIDEXRs,
      const Wire<5> *iIDEXRt,
      const Wire<1> *iEXMEMRegWrite,
      const Wire<5> *iEXMEMRegDstIdx,
      const Wire<1> *iMEMWBRegWrite,
      const Wire<5> *iMEMWBRegDstIdx,
      Wire<2> *oForwardA,
      Wire<2> *oForwardB
    ) : DigitalCircuit(name) {
      _iIDEXRs = iIDEXRs;
      _iIDEXRt = iIDEXRt;
      _iEXMEMRegWrite = iEXMEMRegWrite;
      _iEXMEMRegDstIdx = iEXMEMRegDstIdx;
      _iMEMWBRegWrite = iMEMWBRegWrite;
      _iMEMWBRegDstIdx = iMEMWBRegDstIdx;
      _oForwardA = oForwardA;
      _oForwardB = oForwardB;
    }
    virtual void advanceCycle() {
      *_oForwardA = 0b00;
      *_oForwardB = 0b00;
      if (_iEXMEMRegWrite->to_ulong() && _iEXMEMRegDstIdx->to_ulong() != 0) {
        if (_iEXMEMRegDstIdx->to_ulong() == _iIDEXRs->to_ulong()) {
          *_oForwardA = 0b01;
        } else if (_iEXMEMRegDstIdx->to_ulong() == _iIDEXRt->to_ulong()) {
          *_oForwardB = 0b01;
        }
      }

      if (_iMEMWBRegWrite->to_ulong() && _iMEMWBRegDstIdx->to_ulong() != 0) {
        if (_iMEMWBRegDstIdx->to_ulong() == _iIDEXRs->to_ulong()) {
          *_oForwardA = 0b10;
        } else if (_iMEMWBRegDstIdx->to_ulong() == _iIDEXRt->to_ulong()) {
          *_oForwardB = 0b10;
        }
      }
    }
  private:
    const Wire<5> *_iIDEXRs;
    const Wire<5> *_iIDEXRt;
    const Wire<1> *_iEXMEMRegWrite;
    const Wire<5> *_iEXMEMRegDstIdx;
    const Wire<1> *_iMEMWBRegWrite;
    const Wire<5> *_iMEMWBRegDstIdx;
    Wire<2> *_oForwardA;
    Wire<2> *_oForwardB;
};

#ifdef ENABLE_HAZARD_DETECTION
class HazardDetectionUnit : public DigitalCircuit {
  public:
    HazardDetectionUnit(
      const std::string &name,
      const Wire<5> *iIFIDRs,
      const Wire<5> *iIFIDRt,
      const Wire<1> *iIDEXMemRead,
      const Wire<5> *iIDEXRt,
      Wire<1> *oPCWrite,
      Wire<1> *oIFIDWrite,
      Wire<1> *oIDEXCtrlWrite
    ) : DigitalCircuit(name) {
      _iIFIDRs = iIFIDRs;
      _iIFIDRt = iIFIDRt;
      _iIDEXMemRead = iIDEXMemRead;
      _iIDEXRt = iIDEXRt;
      _oPCWrite = oPCWrite;
      _oIFIDWrite = oIFIDWrite;
      _oIDEXCtrlWrite = oIDEXCtrlWrite;
    }
    virtual void advanceCycle() {
      if (_iIDEXMemRead->to_ulong() && (_iIDEXRt->to_ulong() == _iIFIDRs->to_ulong() || _iIDEXRt->to_ulong() == _iIFIDRt->to_ulong())) {
        *_oPCWrite = 0;
        *_oIFIDWrite = 0;
        *_oIDEXCtrlWrite = 0;
      } else {
        *_oPCWrite = 1;
        *_oIFIDWrite = 1;
        *_oIDEXCtrlWrite = 1;
      }
    }
  private:
    const Wire<5> *_iIFIDRs;
    const Wire<5> *_iIFIDRt;
    const Wire<1> *_iIDEXMemRead;
    const Wire<5> *_iIDEXRt;
    Wire<1> *_oPCWrite;
    Wire<1> *_oIFIDWrite;
    Wire<1> *_oIDEXCtrlWrite;
};
#endif // ENABLE_HAZARD_DETECTION
#endif // ENABLE_DATA_FORWARDING

class PipelinedCPU : public DigitalCircuit {

  public:

    PipelinedCPU(
      const std::string &name,
      const std::uint32_t &initialPC,
      const Memory::Endianness &memoryEndianness,
      const char *regFileName,
      const char *instMemFileName,
      const char *dataMemFileName
    ) : DigitalCircuit(name) {
      _currCycle = 0;
      _PC = initialPC;

      _adderPCPlus4Input1 = 4;
      _adderPCPlus4 = new Adder<32>("adderPCPlus4", &_PC, &_adderPCPlus4Input1, &_pcPlus4);

      Wire<32> _alwaysLo32 = 0;
      _instMemory = new Memory(
        "InstMemory",
        &_PC,
        &_alwaysLo32,
        &_alwaysHi,
        &_alwaysLo,
        &_latchIFID.instruction,
        memoryEndianness,
        instMemFileName);

      _control = new Control(
        &_opcode,
        &_latchIDEX.ctrlEX.regDst,
        &_latchIDEX.ctrlEX.aluSrc,
        &_latchIDEX.ctrlWB.memToReg,
        &_latchIDEX.ctrlWB.regWrite,
        &_latchIDEX.ctrlMEM.memRead,
        &_latchIDEX.ctrlMEM.memWrite,
        &_latchIDEX.ctrlMEM.branch,
        &_latchIDEX.ctrlEX.aluOp);

      _registerFile = new RegisterFile(
        &_regFileReadRegister1,
        &_regFileReadRegister2,
        &_latchMEMWB.regDstIdx,
        &_muxMemToRegOutput,
        &_latchMEMWB.ctrlWB.regWrite,
        &_latchIDEX.regFileReadData1,
        &_latchIDEX.regFileReadData2,
        regFileName);

      //Wire<16> signNoExtend = (_latchIFID.instruction.to_ulong() & 0xFFFF);
      _signExtend = new SignExtend<16, 32>(
        "signExtend", &_signExtendInput, &_latchIDEX.signExtImmediate);

      //Wire<32> input1 = (_latchIDEX.signExtImmediate) << 2;
      _adderBranchTargetAddr = new Adder<32>(
        "adderBranchTargetAddr", &_latchIDEX.pcPlus4, &_adderBranchTargetAddrInput1, &_latchEXMEM.branchTargetAddr);

#ifdef ENABLE_DATA_FORWARDING
      _muxALUSrc = new MUX2<32>(
        "muxALUSrc", &_muxForwardBOutput, &_latchIDEX.signExtImmediate, &_latchIDEX.ctrlEX.aluSrc, &_muxALUSrcOutput);
#else
      _muxALUSrc = new MUX2<32>(
        "muxALUSrc", &_latchIDEX.regFileReadData2, &_latchIDEX.signExtImmediate, &_latchIDEX.ctrlEX.aluSrc, &_muxALUSrcOutput);
#endif

      _aluControl = new ALUControl(
        &_latchIDEX.ctrlEX.aluOp,
        &_aluControlInput,
        &_aluControlOutput
      );

#ifdef ENABLE_DATA_FORWARDING
      _alu = new ALU(
        &_aluControlOutput,
        &_muxForwardAOutput,
        &_muxALUSrcOutput,
        &_latchEXMEM.aluResult,
        &_latchEXMEM.aluZero
      );
#else
      _alu = new ALU(
        &_aluControlOutput,
        &_latchIDEX.regFileReadData1,
        &_muxALUSrcOutput,
        &_latchEXMEM.aluResult,
        &_latchEXMEM.aluZero
      );
#endif

      //Wire<5> input0 = (_latchIFID.instruction.to_ulong() >> 16) & 0x1F;
      //Wire<5> input1 = (_latchIFID.instruction.to_ulong() >> 11) & 0x1F;
      _muxRegDst = new MUX2<5>(
        "muxRegDst", &_latchIDEX.rt, &_latchIDEX.rd, &_latchIDEX.ctrlEX.regDst, &_latchEXMEM.regDstIdx);

      _muxPCSrc = new MUX2<32>(
        "muxPCSrc", &_pcPlus4, &_latchEXMEM.branchTargetAddr, &_muxPCSrcSelect, &_PC);

      _dataMemory = new Memory(
        "dataMemory",
        &_latchEXMEM.aluResult,
        &_latchEXMEM.regFileReadData2,
        &_latchEXMEM.ctrlMEM.memRead,
        &_latchEXMEM.ctrlMEM.memWrite,
        &_latchMEMWB.dataMemReadData,
        memoryEndianness,
        dataMemFileName);

      _muxMemToReg = new MUX2<32>(
        "muxMemToReg", &_latchEXMEM.aluResult, &_latchMEMWB.dataMemReadData, &_latchMEMWB.ctrlWB.memToReg, &_muxMemToRegOutput);
    
#ifdef ENABLE_DATA_FORWARDING
      _forwardingUnit = new ForwardingUnit(
        "forwardingUnit",
        &_latchIDEX.rs,
        &_latchIDEX.rt,
        &_latchEXMEM.ctrlWB.regWrite,
        &_latchEXMEM.regDstIdx,
        &_latchMEMWB.ctrlWB.regWrite,
        &_latchMEMWB.regDstIdx,
        &_forwardA,
        &_forwardB
      );
      // todo
      _muxForwardA = new MUX3<32>(
        "muxForwardA", &_latchIDEX.regFileReadData1, &_muxMemToRegOutput, &_latchEXMEM.aluResult, &_forwardA, &_muxForwardAOutput);
      _muxForwardB = new MUX3<32>(
        "muxForwardB", &_latchIDEX.regFileReadData2, &_muxMemToRegOutput, &_latchEXMEM.aluResult, &_forwardB, &_muxForwardBOutput);
#ifdef ENABLE_HAZARD_DETECTION
      _hazDetUnit = new HazardDetectionUnit(
        "hazDetUnit",
        &_hazDetIFIDRs,
        &_hazDetIFIDRt,
        &_latchIDEX.ctrlMEM.memRead,
        &_latchIDEX.rt,
        &_hazDetPCWrite,
        &_hazDetIFIDWrite,
        &_hazDetIDEXCtrlWrite
      );
#endif
#endif
    }

    void WB() {
      // WB stage
      _muxMemToReg->advanceCycle();
      if (_latchMEMWB.ctrlWB.regWrite.test(0) && _muxMemToRegOutput.to_ulong() != 0xfffff6e1) {
        _registerFile->advanceCycle();
      }
    }

    void MEM() {
      // MEM stage
      _dataMemory->advanceCycle();
      _muxPCSrcSelect = _latchEXMEM.ctrlMEM.branch.to_ulong() & _latchEXMEM.aluZero.to_ulong();
#ifdef ENABLE_HAZARD_DETECTION
      //if (_hazDetPCWrite.test(0)) {
#endif
        _muxPCSrc->advanceCycle();
#ifdef ENABLE_HAZARD_DETECTION
      //}
#endif

      _latchMEMWB.aluResult = _latchEXMEM.aluResult;
      _latchMEMWB.regDstIdx = _latchEXMEM.regDstIdx;
      _latchMEMWB.ctrlWB = _latchEXMEM.ctrlWB;
    }

    void EX() {
      // EX stage
      _aluControlInput = _latchIDEX.signExtImmediate.to_ulong() & 0x3F;
      _aluControl->advanceCycle();

#ifdef ENABLE_DATA_FORWARDING
      _forwardingUnit->advanceCycle();
      _muxForwardA->advanceCycle();
      _muxForwardB->advanceCycle();
#endif
      _muxALUSrc->advanceCycle();

      _alu->advanceCycle();

      _adderBranchTargetAddrInput1 = _latchIDEX.signExtImmediate << 2;
      _adderBranchTargetAddr->advanceCycle();

      _muxRegDst->advanceCycle();

      _latchEXMEM.regFileReadData2 = _latchIDEX.regFileReadData2;
#ifdef ENABLE_DATA_FORWARDING
      _latchEXMEM.regFileReadData2 = _muxForwardBOutput;
#endif
      _latchEXMEM.ctrlWB = _latchIDEX.ctrlWB;
      _latchEXMEM.ctrlMEM = _latchIDEX.ctrlMEM;
    }

    void ID() {
      // ID stage
      _opcode = (_latchIFID.instruction.to_ulong() >> 26) & 0x3F;
      _control->advanceCycle();

      _regFileReadRegister1 = (_latchIFID.instruction.to_ulong() >> 21) & 0x1F;
      _regFileReadRegister2 = (_latchIFID.instruction.to_ulong() >> 16) & 0x1F;
      // todo
      _registerFile->advanceCycle();

      _signExtendInput = (_latchIFID.instruction.to_ulong() & 0xFFFF);
      _signExtend->advanceCycle();

      _latchIDEX.pcPlus4 = _latchIFID.pcPlus4;
#ifdef ENABLE_DATA_FORWARDING
      _latchIDEX.rs = _latchIFID.instruction.to_ulong() >> 21;
#endif
      _latchIDEX.rt = (_latchIFID.instruction.to_ulong() >> 16) & 0x1F;
      _latchIDEX.rd = (_latchIFID.instruction.to_ulong() >> 11) & 0x1F;

#ifdef ENABLE_HAZARD_DETECTION
      _hazDetIFIDRs = _latchIFID.instruction.to_ulong() >> 21;
      _hazDetIFIDRt = (_latchIFID.instruction.to_ulong() >> 16) & 0x1F;
      _hazDetUnit->advanceCycle();
      if (_hazDetIDEXCtrlWrite.to_ulong() == 0) {
        _latchIDEX.ctrlWB.regWrite = 0;
        _latchIDEX.ctrlWB.memToReg = 0;
        _latchIDEX.ctrlMEM.branch = 0;
        _latchIDEX.ctrlMEM.memRead = 0;
        _latchIDEX.ctrlMEM.memWrite = 0;
        _latchIDEX.ctrlEX.aluOp = 0;
        _latchIDEX.ctrlEX.aluSrc = 0;
        _latchIDEX.ctrlEX.regDst = 0;
      }
#endif
    }

    void IF() {
      // IF stage
#ifdef ENABLE_HAZARD_DETECTION
      if (_hazDetIFIDWrite.test(0)) {
#endif
      _adderPCPlus4->advanceCycle();
      _instMemory->advanceCycle();
      _latchIFID.pcPlus4 = _pcPlus4;
#ifdef ENABLE_HAZARD_DETECTION
      }
#endif
    }

    virtual void advanceCycle() {
      _currCycle += 1;

      WB();
      MEM();
      EX();
      ID();
      IF();
    }

    ~PipelinedCPU() {
      delete _adderPCPlus4;
      delete _instMemory;
      delete _control;
      delete _registerFile;
      delete _signExtend;
      delete _adderBranchTargetAddr;
      delete _muxALUSrc;
      delete _aluControl;
      delete _alu;
      delete _muxRegDst;
      delete _muxPCSrc;
      delete _dataMemory;
      delete _muxMemToReg;
#ifdef ENABLE_DATA_FORWARDING
      delete _forwardingUnit;
      delete _muxForwardA;
      delete _muxForwardB;
#ifdef ENABLE_HAZARD_DETECTION
      delete _hazDetUnit;
#endif
#endif
    }

  private:

    // Cycle tracker
    std::uint64_t _currCycle = 0;

    // Always-1/0 wires
    const Wire<1> _alwaysHi = 1;
    const Wire<1> _alwaysLo = 0;

    // Components for the IF stage
    Register<32> _PC; // the Program Counter (PC) register
    Adder<32> *_adderPCPlus4; // the 32-bit adder in the IF stage
    Memory *_instMemory; // the instruction memory
    // Components for the ID stage
    Control *_control; // the Control unit
    RegisterFile *_registerFile; // the Register File
    SignExtend<16, 32> *_signExtend; // the sign-extend unit
    // Components for the EX stage
    Adder<32> *_adderBranchTargetAddr; // the 32-bit adder in the EX stage
    MUX2<32> *_muxALUSrc; // the MUX whose control signal is 'ALUSrc'
    ALUControl *_aluControl; // the ALU Control unit
    ALU *_alu; // the ALU
    MUX2<5> *_muxRegDst; // the MUX whose control signal is 'RegDst'
    // Components for the MEM stage
    MUX2<32> *_muxPCSrc; // the MUX whose control signal is 'PCSrc'
    Memory *_dataMemory; // the data memory
    // Components for the WB stage
    MUX2<32> *_muxMemToReg; // the MUX whose control signal is 'MemToReg'
#ifdef ENABLE_DATA_FORWARDING
    ForwardingUnit *_forwardingUnit; // the forwarding unit
    MUX3<32> *_muxForwardA; // the 3-to-1 MUX whose control signal is 'forwardA'
    MUX3<32> *_muxForwardB; // the 3-to-1 MUX whose control signal is 'forwardB'
#ifdef ENABLE_HAZARD_DETECTION
    HazardDetectionUnit *_hazDetUnit; // the Hazard Detection unit
#endif
#endif

    // Latches
    typedef struct {
      Register<1> regDst;
      Register<2> aluOp;
      Register<1> aluSrc;
    } ControlEX_t; // the control signals for the EX stage
    typedef struct {
      Register<1> branch;
      Register<1> memRead;
      Register<1> memWrite;
    } ControlMEM_t; // the control signals for the MEM stage
    typedef struct {
      Register<1> memToReg;
      Register<1> regWrite;
    } ControlWB_t; // the control signals for the WB stage
    struct {
      Register<32> pcPlus4; // PC+4
      Register<32> instruction; // 32-bit instruction
    } _latchIFID = {}; // the IF-ID latch
    struct {
      ControlWB_t ctrlWB; // the control signals for the WB stage
      ControlMEM_t ctrlMEM; // the control signals for the MEM stage
      ControlEX_t ctrlEX; // the control signals for the EX stage
      Register<32> pcPlus4; // PC+4
      Register<32> regFileReadData1; // 'ReadData1' from the register file
      Register<32> regFileReadData2; // 'ReadData2' from the register file
      Register<32> signExtImmediate; // the 32-bit sign-extended immediate value
#ifdef ENABLE_DATA_FORWARDING
      Register<5> rs; // the 5-bit 'rs' field
#endif
      Register<5> rt; // the 5-bit 'rt' field
      Register<5> rd; // the 5-bit 'rd' field
    } _latchIDEX = {}; // the ID-EX latch
    struct {
      ControlWB_t ctrlWB; // the control signals for the WB stage
      ControlMEM_t ctrlMEM; // the control signals for the MEM stage
      Register<32> branchTargetAddr; // the 32-bit branch target address
      Register<1> aluZero; // 'Zero' from the ALU
      Register<32> aluResult; // the 32-bit ALU output
      Register<32> regFileReadData2; // 'ReadData2' from the register file
      Register<5> regDstIdx; // the index of the destination register
    } _latchEXMEM = {}; // the EX-MEM latch
    struct {
      ControlWB_t ctrlWB; // the control signals for the WB stage
      Register<32> dataMemReadData; // the 32-bit data read from the data memory
      Register<32> aluResult; // the 32-bit ALU output
      Register<5> regDstIdx; // the index of the destination register
    } _latchMEMWB = {}; // the MEM-WB latch

    // Wires
    Wire<32> _adderPCPlus4Input1; // the second input to the adder in the IF stage (i.e., 4)
    Wire<32> _pcPlus4; // the output of the adder in the IF stage
    Wire<6> _opcode; // the input to the Control unit
    Wire<5> _regFileReadRegister1; // 'ReadRegister1' for the Register File
    Wire<5> _regFileReadRegister2; // 'ReadRegister2' for the Register File
    Wire<32> _muxMemToRegOutput; // the output of the MUX whose control signal is 'MemToReg'
    Wire<16> _signExtendInput; // the input to the sign-extend unit
    Wire<32> _adderBranchTargetAddrInput1; // the second input to the adder in the EX stage
    Wire<32> _muxALUSrcOutput; // the output of the MUX whose control signal is 'ALUSrc'
    Wire<6> _aluControlInput; // the input to the ALU Control unit (i.e., the 'funct' field)
    Wire<4> _aluControlOutput; // the output of the ALU Control unit
    Wire<1> _muxPCSrcSelect; // the control signal (a.k.a. selector) for the MUX whose control signal is 'PCSrc'
#ifdef ENABLE_DATA_FORWARDING
    Wire<2> _forwardA, _forwardB; // the outputs from the Forwarding unit
    Wire<32> _muxForwardAOutput; // the output of the 3-to-1 MUX whose control signal is 'forwardA'
    Wire<32> _muxForwardBOutput; // the output of the 3-to-1 MUX whose control signal is 'forwardB'
#ifdef ENABLE_HAZARD_DETECTION
    Wire<5> _hazDetIFIDRs, _hazDetIFIDRt; // the inputs to the Hazard Detection unit
    Wire<1> _hazDetPCWrite, _hazDetIFIDWrite, _hazDetIDEXCtrlWrite; // the outputs of the Hazard Detection unit
#endif
#endif

  public:

    void printPVS() {
      printf("==================== Cycle %lu ====================\n", _currCycle);
      printf("PC = 0x%08lx\n", _PC.to_ulong());
      printf("Registers:\n");
      _registerFile->printRegisters();
      printf("Data Memory:\n");
      _dataMemory->printMemory();
      printf("Instruction Memory:\n");
      _instMemory->printMemory();
      printf("Latches:\n");
      printf("  IF-ID Latch:\n");
      printf("    pcPlus4          = 0x%08lx\n", _latchIFID.pcPlus4.to_ulong());
      printf("    instruction      = 0x%08lx\n", _latchIFID.instruction.to_ulong());
      printf("  ID-EX Latch:\n");
      printf("    ctrlWBMemToReg   = 0b%s\n", _latchIDEX.ctrlWB.memToReg.to_string().c_str());
      printf("    ctrlWBRegWrite   = 0b%s\n", _latchIDEX.ctrlWB.regWrite.to_string().c_str());
      printf("    ctrlMEMBranch    = 0b%s\n", _latchIDEX.ctrlMEM.branch.to_string().c_str());
      printf("    ctrlMEMMemRead   = 0b%s\n", _latchIDEX.ctrlMEM.memRead.to_string().c_str());
      printf("    ctrlMEMMemWrite  = 0b%s\n", _latchIDEX.ctrlMEM.memWrite.to_string().c_str());
      printf("    ctrlEXRegDst     = 0b%s\n", _latchIDEX.ctrlEX.regDst.to_string().c_str());
      printf("    ctrlEXALUOp      = 0b%s\n", _latchIDEX.ctrlEX.aluOp.to_string().c_str());
      printf("    ctrlEXALUSrc     = 0b%s\n", _latchIDEX.ctrlEX.aluSrc.to_string().c_str());
      printf("    pcPlus4          = 0x%08lx\n", _latchIDEX.pcPlus4.to_ulong());
      printf("    regFileReadData1 = 0x%08lx\n", _latchIDEX.regFileReadData1.to_ulong());
      printf("    regFileReadData2 = 0x%08lx\n", _latchIDEX.regFileReadData2.to_ulong());
      printf("    signExtImmediate = 0x%08lx\n", _latchIDEX.signExtImmediate.to_ulong());
#ifdef ENABLE_DATA_FORWARDING
      printf("    rs               = 0b%s\n", _latchIDEX.rs.to_string().c_str());
#endif
      printf("    rt               = 0b%s\n", _latchIDEX.rt.to_string().c_str());
      printf("    rd               = 0b%s\n", _latchIDEX.rd.to_string().c_str());
      printf("  EX-MEM Latch:\n"); 
      printf("    ctrlWBMemToReg   = 0b%s\n", _latchEXMEM.ctrlWB.memToReg.to_string().c_str());
      printf("    ctrlWBRegWrite   = 0b%s\n", _latchEXMEM.ctrlWB.regWrite.to_string().c_str());
      printf("    ctrlMEMBranch    = 0b%s\n", _latchEXMEM.ctrlMEM.branch.to_string().c_str());
      printf("    ctrlMEMMemRead   = 0b%s\n", _latchEXMEM.ctrlMEM.memRead.to_string().c_str());
      printf("    ctrlMEMMemWrite  = 0b%s\n", _latchEXMEM.ctrlMEM.memWrite.to_string().c_str());
      printf("    branchTargetAddr = 0x%08lx\n", _latchEXMEM.branchTargetAddr.to_ulong());
      printf("    aluZero          = 0b%s\n", _latchEXMEM.aluZero.to_string().c_str());
      printf("    aluResult        = 0x%08lx\n", _latchEXMEM.aluResult.to_ulong());
      printf("    regFileReadData2 = 0x%08lx\n", _latchEXMEM.regFileReadData2.to_ulong());
      printf("    regDstIdx        = 0b%s\n", _latchEXMEM.regDstIdx.to_string().c_str());
      printf("  MEM-WB Latch:\n"); 
      printf("    ctrlWBMemToReg   = 0b%s\n", _latchMEMWB.ctrlWB.memToReg.to_string().c_str());
      printf("    ctrlWBRegWrite   = 0b%s\n", _latchMEMWB.ctrlWB.regWrite.to_string().c_str());
      printf("    dataMemReadData  = 0x%08lx\n", _latchMEMWB.dataMemReadData.to_ulong());
      printf("    aluResult        = 0x%08lx\n", _latchMEMWB.aluResult.to_ulong());
      printf("    regDstIdx        = 0b%s\n", _latchMEMWB.regDstIdx.to_string().c_str());
    }

};

#endif
