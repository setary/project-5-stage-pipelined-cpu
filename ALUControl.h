#ifndef __ALU_CONTROL_H__
#define __ALU_CONTROL_H__

#include "DigitalCircuit.h"

#include <cassert>

class ALUControl : public DigitalCircuit {

  public:

    ALUControl(const Wire<2> *iALUOp,
               const Wire<6> *iFunct,
               Wire<4> *oOperation) : DigitalCircuit("ALUControl") {
      _iALUOp = iALUOp;
      _iFunct = iFunct;
      _oOperation = oOperation;
    }

    virtual void advanceCycle() {
      uint8_t aluOp = _iALUOp->to_ulong();
      uint8_t funct = _iFunct->to_ulong();
      uint8_t controlSignal = 0b0000; // Default to AND

      switch (aluOp) {
          case 0x0: // lw, sw
            controlSignal = 0b0010; // ADD
            break;
          case 0x1: // beq
            controlSignal = 0b0110; // SUB
            break;
          case 0x2: // R-type instructions
            if ((funct & 0xF) == 0x0) {
              controlSignal = 0b0010; // ADD
            } else if ((funct & 0xF) == 0x2) {
              controlSignal = 0b0110; // SUB
            } else if ((funct & 0xF) == 0x4) {
              controlSignal = 0b0000; // AND
            } else if ((funct & 0xF) == 0x5) {
              controlSignal = 0b0001; // OR
            } else if ((funct & 0xF) == 0xa) {
              controlSignal = 0b0111; // SLT
            } else {
              controlSignal = 0b0000; // Default to AND for all other cases
            }
            break;
          default:
            controlSignal = 0b0000; // Default to AND for all other cases
            break;
      }

      *_oOperation = std::bitset<4>(controlSignal);
    }

  private:

    const Wire<2> *_iALUOp;
    const Wire<6> *_iFunct;
    Wire<4> *_oOperation;

};

#endif

