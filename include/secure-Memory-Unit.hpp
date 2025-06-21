//TODO
#ifndef SECURE_MEMORY_UNIT_HPP
#define SECURE_MEMORY_UNIT_HPP

#include <systemc>
#include <systemc.h>
#include <map>
using namespace sc_core;

#include "address_Scrambler.hpp"
#include "encryptor.hpp"
#include "fault-Injection.hpp"
#include "memory-Access.hpp"
#include "Parity-Checker.hpp"
#include "prng.hpp"

SC_MODULE(TINY_RISC) {


  SC_HAS_PROCESS(TINY_RISC);

    ADDRESS_SCRAMBLER address_scrambler;
    ENCRYPTOR encryptor;
    FAULT_INJECTION fault_injection;
    MEMORY_ACCESS memory_access;
    PARITY_CHECKER parity_checker;
    PRNG prng;



  std::map<unsigned int, unsigned int>& init_mem;


    sc_clock clk;
    sc_in<uint32_t> addr;
    sc_in<uint32_t> wdata;
    sc_in<bool> r;
    sc_in<bool> w;
    sc_in<uint32_t> fault;
    sc_in<sc_bv<4>> faultbit;


    sc_out<uint32_t> rdata;
    sc_out<bool> ready;
    sc_in<bool> error;



  
  //TODO

  SMU(sc_module_name name,
          uint8_t endianness, uint32_t latency_scrambling, uint32_t latency_encryption,
          uint32_t latency_memory_access,uint32_t seed)
    : 
    sc_module(name),
    
    {
    

   
}

};

#endif // TINYRISC_HPP