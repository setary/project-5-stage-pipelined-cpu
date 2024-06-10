#ifndef __ALU_H__
#define __ALU_H__

#include "DigitalCircuit.h"

#include <cassert>

class ALU : public DigitalCircuit {

  public:

    ALU(const Wire<4> *iALUControl,
        const Wire<32> *iInput0,
        const Wire<32> *iInput1,
        Wire<32> *oOutput,
        Wire<1> *oZero) : DigitalCircuit("ALU") {
      _iALUControl = iALUControl;
      _iInput0 = iInput0;
      _iInput1 = iInput1;
      _oOutput = oOutput;
      _oZero = oZero;
    }

    virtual void advanceCycle() {
      uint32_t input0 = _iInput0->to_ulong();
      uint32_t input1 = _iInput1->to_ulong();
      uint8_t aluControl = _iALUControl->to_ulong();

      uint32_t result = 0;

      switch (aluControl) {
        case 0x0: // AND
          result = input0 & input1;
          break;
        case 0x1: // OR
          result = input0 | input1;
          break;
        case 0x2: // ADD
          result = input0 + input1;
          break;
        case 0x6: // SUBTRACT
          result = input0 - input1;
          break;
        case 0x7: // SET ON LESS THAN
          result = (input0 < input1) ? 1 : 0;
          break;
        case 0xC: // NOR
          result = ~(input0 | input1);
          break;
        default:
          result = 0;
          break;
      }

      *_oOutput = result;
      *_oZero = (result == 0) ? 1 : 0;
    }

  private:

    const Wire<4> *_iALUControl;
    const Wire<32> *_iInput0;
    const Wire<32> *_iInput1;
    Wire<32> *_oOutput;
    Wire<1> *_oZero;

};

#endif

