#ifndef ALU_HPP
#define ALU_HPP

#include <systemc>
#include <systemc.h>
using namespace sc_core;

SC_MODULE(ALU) {
  sc_in<uint32_t> r1;
  sc_in<uint32_t> r2;
  sc_in<sc_bv<3>> op;

  sc_out<uint32_t> ro;
  sc_out<bool> overflow;

  SC_CTOR(ALU) {
    SC_THREAD(behaviour);
    sensitive << r1 << r2 << op;
  }

  void behaviour() {
    while(true) {
      uint32_t v1 = r1.read();
      uint32_t v2 = r2.read();

      uint8_t opValue = op.read().to_uint();
      uint32_t result = 0;
      bool didOverflow = false;

      switch(opValue) {
        case 0: // NOT
          result = ~v1;
          break;
        case 1: // AND
          result = v1 & v2;
          break;
        case 2: // OR
          result = v1 | v2;
          break;
        case 4: // ADD
          result = v1 + v2;
          if ((v1 < 0x80000000 && v2 < 0x80000000 && result >= 0x80000000) || (v1 >= 0x80000000 && v2 >= 0x80000000 && result < 0x80000000)) {
            didOverflow = true;
          }
          break;
        case 5: // SUB
          result = v1 - v2;
          if ((v1 < 0x80000000 && v2 >= 0x80000000 && result >= 0x80000000) || (v1 >= 0x80000000 && v2 < 0x80000000 && result < 0x80000000)) {
            didOverflow = true;
          }
          break;
        case 6: // INC
          result = v1 + 1;
          if(v1 == INT32_MAX) {
            didOverflow = true;
          }
          break;
      }

      ro.write(result);
      overflow.write(didOverflow);
      wait();
    }
  }
};

#endif // ALU_HPP