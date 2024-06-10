#ifndef __REGISTER_FILE_HPP__
#define __REGISTER_FILE_HPP__

#include "DigitalCircuit.h"

#include <cassert>
#include <cstdio>

class RegisterFile : public DigitalCircuit {

  public:

    RegisterFile(const Wire<5> *iReadRegister1,
                 const Wire<5> *iReadRegister2,
                 const Wire<5> *iWriteRegister,
                 const Wire<32> *iWriteData,
                 const Wire<1> *iRegWrite,
                 Wire<32> *oReadData1,
                 Wire<32> *oReadData2,
                 const char *initFileName = nullptr)
      : DigitalCircuit("RegisterFile") {
      _iReadRegister1 = iReadRegister1;
      _iReadRegister2 = iReadRegister2;
      _iWriteRegister = iWriteRegister;
      _iWriteData = iWriteData;
      _iRegWrite = iRegWrite;
      _oReadData1 = oReadData1;
      _oReadData2 = oReadData2;

      for (size_t i = 0; i < 32; i++) {
        _registers[i].reset();
      }

      if (initFileName != nullptr) {
        // Each line of the memory initialization file consists of:
        //   1) the target register index in decimal value
        //   2) the eight-digit hexadecimal value of the data
        // For example, "10 ABCD1234" stores 0xABCD1234 to register #10.
        FILE *initFile = fopen(initFileName, "r");
        assert(initFile != NULL);
        std::uint32_t reg, val;
        while (fscanf(initFile, " %u %x", &reg, &val) == 2) {
          if (reg != 0) {
            printf("INFO: $%02u <-- 0x%08lx\n", reg, (unsigned long)val);
            _registers[reg] = val;
          }
        }
      }
    }

    void printRegisters() {
      for (size_t i = 0; i < 32; i++) {
        if (_registers[i].any()) {
          printf("  $%02lu = 0x%08lx\n", i, _registers[i].to_ulong());
        }
      }
    }

    virtual void advanceCycle() {
      std::uint8_t readReg1 = _iReadRegister1->to_ulong(); // $rs
      std::uint8_t readReg2 = _iReadRegister2->to_ulong(); // $rt
      *_oReadData1 = _registers[readReg1].to_ulong();
      *_oReadData2 = _registers[readReg2].to_ulong();

      if (_iRegWrite->test(0)) {
        printf("@@zsh RegisterFile regWrite=1\n");
        std::uint8_t writeReg = _iWriteRegister->to_ulong();
        if (writeReg != 0) { 
          _registers[writeReg] = _iWriteData->to_ulong();
        }  
      }
    }

  private:

    const Wire<5> *_iReadRegister1;
    const Wire<5> *_iReadRegister2;
    const Wire<5> *_iWriteRegister;
    const Wire<32> *_iWriteData;
    const Wire<1> *_iRegWrite;
    Wire<32> *_oReadData1;
    Wire<32> *_oReadData2;

    std::bitset<32> _registers[32];

};

#endif

