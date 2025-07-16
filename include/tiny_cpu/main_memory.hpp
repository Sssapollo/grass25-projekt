#ifndef MAIN_MEMORY_HPP
#define MAIN_MEMORY_HPP

#include <systemc>
#include <map>
using namespace sc_core;

#define LATENCY 10

SC_MODULE(MAIN_MEMORY) {
  sc_in<bool> clk;

  sc_in<uint32_t> addr;
  sc_in<uint32_t> wdata;
  sc_in<bool> r;
  sc_in<bool> w;

  sc_out<uint32_t> rdata;
  sc_out<bool> ready;

  std::map<uint32_t, uint8_t> memory;

  SC_CTOR(MAIN_MEMORY) {
    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }

  void behaviour() {
    while(true) {
      wait();

      if (r.read()) {
        doRead(w.read());
      }
      if (w.read()) {
        doWrite();
      }
    }
  }

  void doRead(bool dontSetReady) {
    ready.write(false);

    uint32_t result = get(addr.read());

    for(int i = 0; i < LATENCY; i++) {
      wait();
    }

    rdata.write(result);
    if(!dontSetReady) {
      ready.write(true);
    }
  }

  void doWrite() {
    ready.write(false);
    set(addr.read(), wdata.read());

    for(int i = 0; i < LATENCY; i++) {
      wait();
    }

    ready.write(true);
  }

  uint32_t get(uint32_t address) {
    uint32_t result = 0;

    for (int i = 0; i < 4; i++) {
      uint8_t value = 0;
      if(memory.find(address + i) != memory.end()) {
        value = memory[address + i];
      }
      result |= value << (i * 8);
    }

    return result;
  }

  void set(uint32_t address, uint32_t value) {
    for (int i = 0; i < 4; i++) {
      memory[address + i] = (value >> (i * 8)) & 0xFF;
      if(address + i == UINT32_MAX) {
        break;
      }
    }
  }
};

#endif // MAIN_MEMORY_HPP