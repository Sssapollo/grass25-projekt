#ifndef PC_HPP
#define PC_HPP

#include <systemc>
#include "systemc.h"
using namespace sc_core;

SC_MODULE(PC) {

  sc_in<bool> clk;
  sc_in<bool> enable;
  sc_in<sc_bv<2>> j;
  sc_in<uint32_t> next;
  sc_in<sc_bv<12>> offset;

  
  

  sc_out<uint32_t> pc;

  uint32_t pcValue;

  SC_CTOR(PC) {
    pcValue = 0x00001000;
    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  void behaviour() {

    while(true) {
      wait();
      if (enable.read()) {
        switch (j.read().to_uint()) {
          case 0:
            pcValue = pcValue + 4;
            break;
          case 1:
            pcValue = next.read();
            break;
          case 2:
            pcValue = pcValue + offset.read().to_uint();
            break;
          default:
            break;
        }
      }
      pc.write(pcValue);
    }
  }
};

#endif // PC_HPP