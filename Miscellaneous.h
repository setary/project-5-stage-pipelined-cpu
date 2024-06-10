#ifndef __MISCELLANEOUS_H__
#define __MISCELLANEOUS_H__

#include "DigitalCircuit.h"

template<size_t N>
class Adder : public DigitalCircuit {

  public:

    Adder(
      const std::string &name,
      const Wire<N> *input0,
      const Wire<N> *input1,
      Wire<N> *output
    ) : DigitalCircuit(name) {
      _input0 = input0;
      _input1 = input1;
      _output = output;
    }

    virtual void advanceCycle() {
      _output->reset();

      unsigned int tmp = 0;
      for (size_t i = 0; i < N; i++) {
        if (_input0->test(i)) { tmp++; }
        if (_input1->test(i)) { tmp++; }
        if ((tmp % 2) == 1) { _output->set(i); }
        tmp /= 2;
      }
    }

  private:

    const Wire<N> *_input0, *_input1;
    Wire<N> *_output;

};

template<size_t N, size_t M>
class SignExtend : public DigitalCircuit {

  public:

    SignExtend(
      const std::string &name,
      const Wire<N> *input,
      Wire<M> *output
    ) : DigitalCircuit(name) {
      _input = input;
      _output = output;
    }

    virtual void advanceCycle() {
      _output->reset();

      for (size_t i = 0; i < N; i++) {
        if (_input->test(i)) {
          _output->set(i);
        }
      }

      for (size_t i = N; i < M; i++) {
        if (_input->test(N - 1)) {
          _output->set(i);
        }
      }
    }

  private:

    const Wire<N> *_input;
    Wire<M> *_output;

};

template<size_t N>
class MUX2 : public DigitalCircuit {

  public:

    MUX2(
      const std::string &name,
      const Wire<N> *input0,
      const Wire<N> *input1,
      const Wire<1> *select,
      Wire<N> *output
    ) : DigitalCircuit(name) {
      _input0 = input0;
      _input1 = input1;
      _select = select;
      _output = output;
    }

    virtual void advanceCycle() {
      _output->reset();
      switch (_select->to_ulong()) {
        case 0: {
          (*_output) = (*_input0);
          break;
        }
        case 1: {
          (*_output) = (*_input1);
          break;
        }
        default: {
          fprintf(stderr, "[%s] ERROR: unsupported _select %lu\n", __func__, _select->to_ulong());
          assert(false);
          break;
        }
      }
    }

  private:

    const Wire<N> *_input0, *_input1;
    const Wire<1> *_select;
    Wire<N> *_output;

};

template<size_t N>
class MUX3 : public DigitalCircuit {

  public:

    MUX3(
      const std::string &name,
      const Wire<N> *input0,
      const Wire<N> *input1,
      const Wire<N> *input2,
      const Wire<2> *select,
      Wire<N> *output
    ) : DigitalCircuit(name) {
      _input0 = input0;
      _input1 = input1;
      _input2 = input2;
      _select = select;
      _output = output;
    }

    virtual void advanceCycle() {
      _output->reset();
      switch (_select->to_ulong()) {
        case 0: {
          (*_output) = (*_input0);
          break;
        }
        case 1: {
          (*_output) = (*_input1);
          break;
        }
        case 2: {
          (*_output) = (*_input2);
          break;
        }
        default: {
          fprintf(stderr, "[%s] ERROR: unsupported _select %lu\n", __func__, _select->to_ulong());
          assert(false);
          break;
        }
      }
    }

  private:

    const Wire<N> *_input0, *_input1, *_input2;
    const Wire<2> *_select;
    Wire<N> *_output;

};

#endif

