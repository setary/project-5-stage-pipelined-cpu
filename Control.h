#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "DigitalCircuit.h"

class Control : public DigitalCircuit {

  public:

    Control(const Wire<6> *iOpcode,
            Wire<1> *oRegDst,
            Wire<1> *oALUSrc,
            Wire<1> *oMemToReg,
            Wire<1> *oRegWrite,
            Wire<1> *oMemRead,
            Wire<1> *oMemWrite,
            Wire<1> *oBranch,
            Wire<2> *oALUOp) : DigitalCircuit("Control") {
      _iOpcode = iOpcode;
      _oRegDst = oRegDst;
      _oALUSrc = oALUSrc;
      _oMemToReg = oMemToReg;
      _oRegWrite = oRegWrite;
      _oMemRead = oMemRead;
      _oMemWrite = oMemWrite;
      _oBranch = oBranch;
      _oALUOp = oALUOp;
    }

    virtual void advanceCycle() {
      uint8_t opcode = _iOpcode->to_ulong();

      switch (opcode) {
        case 0b000000: // R-format
        //case 0b000001:
        //case 0b000010:
        //case 0b000110:
        //case 0b001111:
          *_oRegDst = 1;
          *_oALUSrc = 0;
          *_oMemToReg = 0;
          *_oRegWrite = 1;
          *_oMemRead = 0;
          *_oMemWrite = 0;
          *_oBranch = 0;
          *_oALUOp = 0b10;
          break;
        case 0b100011: // lw
          *_oRegDst = 0;
          *_oALUSrc = 1;
          *_oMemToReg = 1;
          *_oRegWrite = 1;
          *_oMemRead = 1;
          *_oMemWrite = 0;
          *_oBranch = 0;
          *_oALUOp = 0b00;
          break;
        case 0b101011: // sw
          *_oRegDst = 0; 
          *_oALUSrc = 1;
          *_oMemToReg = 0; 
          *_oRegWrite = 0;
          *_oMemRead = 0;
          *_oMemWrite = 1;
          *_oBranch = 0;
          *_oALUOp = 0b00;
          break;
        case 0b000100: // beq
          *_oRegDst = 0; 
          *_oALUSrc = 0;
          *_oMemToReg = 0; 
          *_oRegWrite = 0;
          *_oMemRead = 0;
          *_oMemWrite = 0;
          *_oBranch = 1;
          *_oALUOp = 0b01;
          break;
        default: 
          *_oRegDst = 0;
          *_oALUSrc = 0;
          *_oMemToReg = 0;
          *_oRegWrite = 0;
          *_oMemRead = 0;
          *_oMemWrite = 0;
          *_oBranch = 0;
          *_oALUOp = 0b00;
          break;
      }
    }

  private:

    const Wire<6> *_iOpcode;
    Wire<1> *_oRegDst;
    Wire<1> *_oALUSrc;
    Wire<1> *_oMemToReg;
    Wire<1> *_oRegWrite;
    Wire<1> *_oMemRead;
    Wire<1> *_oMemWrite;
    Wire<1> *_oBranch;
    Wire<2> *_oALUOp;

};

#endif

