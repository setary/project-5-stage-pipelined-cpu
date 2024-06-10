#ifndef __DIGITAL_CIRCUIT_H__
#define __DIGITAL_CIRCUIT_H__

#include <string>
#include <bitset>

#include <cstdio>

class DigitalCircuit {

  public:

    // DigitalCircuit::DigitalCircuit()
    //   - initializes the internal state of a digital circuit
    DigitalCircuit(const std::string &name) {
      _name = name;
    }

    void printName() {
      fprintf(stderr, "[%s] this = %p, this->_name = %s\n", __func__, this,
        _name.c_str());
    }

    // void DigitalCircuit::advanceCycle()
    //   - simulates one clock cycle of a digital circuit by updating the
    //     internal state and output signals using the provided input signal
    //     values
    virtual void advanceCycle() = 0;

  private:

    std::string _name;

};

template<size_t N>
using Register = std::bitset<N>;

template<size_t N>
using Wire = std::bitset<N>;

template<size_t N>
void printWire(const std::string &name, const Wire<N> &wire) {
  fprintf(stderr, "[%s] wireName = %s, wireValue = 0x", __func__, name.c_str());

  unsigned tmp = 0;
  for (size_t i = 0; i < ((N + 3) / 4) * 4; i++) {
    tmp = tmp << 1;
    if (i < N) {
      size_t j = (N - 1) - i;
      if (wire.test(j)) { tmp++; }
      if (j % 4 == 0) {
        fprintf(stderr, "%1x", tmp);
        tmp = 0;
      }
    }
  }
  fprintf(stderr, "\n");
}

template<size_t N>
void printWire(const std::string &name, const Wire<N> *wire) {
  assert(wire != nullptr);
  printWire(name, *wire);
}

#endif

