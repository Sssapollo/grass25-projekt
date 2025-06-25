//TODO
#ifndef SECURE_MEMORY_UNIT_HPP
#define SECURE_MEMORY_UNIT_HPP

#include <systemc>
#include <systemc.h>
#include <map>
using namespace sc_core;

// #include "address_Scrambler.hpp"
// #include "encryptor.hpp"
// #include "fault-Injection.hpp"
// #include "memory-Access.hpp"
// #include "Parity-Checker.hpp"
// #include "prng.hpp"

SC_MODULE(SecureMemoryUnit) {


  SC_HAS_PROCESS(SecureMemoryUnit);

    // AddressScrambler address_scrambler;
    // Encryptor encryptor;
    // MAIN_MEMORY memory_access;
    // PARITY_CHECKER parity_checker;
    // PRNG prng;


    sc_in<bool> clk;
    sc_in<uint32_t> addr;
    sc_in<uint32_t> wdata;
    sc_in<bool> r;
    sc_in<bool> w;
    sc_in<uint32_t> fault;
    sc_in<sc_bv<4>> faultbit;

    std::map<uint32_t, uint8_t> memory;
    std::map<uint32_t, bool> parity_memory;
    uint32_t state;
    uint32_t scrambler_key; 
    uint32_t encryptor_key;
    uint8_t isBigEndian;
    bool error_flag ;
    uint32_t latency_scrambling;
    uint32_t latency_encryption;
    uint32_t latency_memory_access;
    uint32_t seed;


    sc_out<uint32_t> rdata;
    sc_out<bool> ready;
    sc_out<bool> error;



  
  //TODO

  SecureMemoryUnit(sc_module_name name,
          uint8_t endianness, uint32_t latency_scrambling, uint32_t latency_encryption,
          uint32_t latency_memory_access,uint32_t seed)
    : 
    sc_module(name),
    clk("clk"),
    addr("addr"),
    wdata("wdata"),
    r("r"),
    w("w"),
    fault("fault"),
    faultbit("faultbit"),
    rdata("rdata"),
    ready("ready"),
    error("error"),
    state(0),
    scrambler_key(generate(seed,state)), 
    encryptor_key(generate(seed,state)),
    isBigEndian(endianness),
    latency_scrambling(latency_scrambling),
    latency_encryption(latency_encryption),
    latency_memory_access(latency_memory_access),
    seed(seed),
    error_flag(false)

    
    {
    memory.clear();
    parity_memory.clear();
    error_flag = false;
    ready.write(false);
    rdata.write(0);
    error.write(false);
    SC_THREAD(process);
    sensitive << clk.pos();




}


void process() {
    while (true) {
        

      if(fault.read() != UINT32_MAX){
        uint32_t index = faultbit.read().touint();
          if( index == 8){
            parity_memory[fault.read()] = !parity_memory[fault.read()];

          }else{
              memory[fault.read()] = memory[fault.read()] ^ (1 << index);
          }
      }



        if(r.read() || w.read()) {
        // Reset error flag and outputs
        error_flag = false;
        ready.write(false);
        rdata.write(0);
       


          uint32_t address = addr.read();
          uint32_t p0, p1, p2, p3;
          scramble(address, scrambler_key, p0, p1, p2, p3); 
          for (uint32_t i = 0; i < latency_scrambling; ++i) wait();
          uint32_t addresses[4] = {p0, p1, p2, p3};
          uint8_t encypted_data_bytes[4];

        }
        if(w.read()){
          uint32_t data_w = wdata.read();
          uint32_t encrypted_data = encrypt(data_w, encryptor_key);
          for (uint32_t i = 0; i < latency_encryption; ++i) wait();
          for (int i = 0; i < 4; i++) {
          uint8_t value = 0;
          int byteIndex = isBigEndian ? (3 - i) : i;
          value = (encrypted_data >> (i * 8)) & 0xFF;
          encypted_data_bytes[byteIndex] = value;
        }
          for (int i = 0; i < 4; i++)
          {
            int parity = calculate_parity(encypted_data_bytes[i]);
            parity_memory[addresses[i]] = parity;
            write_memory(addresses[i], encypted_data_bytes[i]);
        }
        for(uint32_t j = 0; j < latency_memory_access; ++j) wait();
         error.write(false);
      }
          if(r.read()) {
            error_flag = false;
            for (int i = 0; i < 4; i++) {
              uint8_t value = 0;
              int byteIndex = isBigEndian ? (3-i) : i;
              value = read_memory(addresses[i]) & 0xFF;
              
              encypted_data_bytes[byteIndex] = value;
            }
            for(uint32_t j = 0; j < latency_memory_access; ++j) wait();
            uint32_t data_r = 0;
            for (int i = 0; i < 4; i++)
          {
            int parity = calculate_parity(encypted_data_bytes[i]);
           
            if (parity_memory[addresses[i]] != parity) {
              error_flag = true;
            }
            data_r |= (encypted_data_bytes[i] << (i * 8));
          }
            data_r = encrypt(data_r, encryptor_key);// Decrypt the data
            for (uint32_t i = 0; i < latency_encryption; ++i) wait();
            if(error_flag){
              error.write(true);
              rdata.write(0);
            } else {
              error.write(false);
              rdata.write(data_r);
            }
            
          }

          ready.write(true);

        }


      }

//encryptor
uint32_t encrypt(uint32_t data, uint32_t key) {
    uint32_t encrypted_data = 0;
    encrypted_data = data ^ key;  
    return encrypted_data;
}


//parity checker
int calculate_parity(uint8_t data) {
    int parity_bit = 0;
    for (int i = 0; i < 8; ++i) {
      parity_bit += (data >> i) & 1;  
    }

    return parity_bit & 1;
    }


//address scrambler
void scramble(uint32_t address, uint32_t key, uint32_t &p0, uint32_t &p1, uint32_t &p2, uint32_t &p3) {
    p0 = (address + 0) ^ key;
    p1 = (address + 1) ^ key;
    p2 = (address + 2) ^ key;
    p3 = (address + 3) ^ key;
  }




//PRNG generator
uint32_t generate(uint32_t seed,uint32_t &state) {   
        if (seed != 0 && state == 0) {
            state = seed;
        } else {
            state = (state * 950706376) % 2147483647; 
        }
        return state;
       
    }




// read memory
  uint8_t read_memory(uint32_t address) {
    return memory[address];
}
 
  // write memory
  void write_memory(uint32_t address, uint8_t value) {
      memory[address] = value & 0xFF;
  }

};

#endif // TINYRISC_HPP