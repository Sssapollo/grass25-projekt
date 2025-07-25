// TODO
#ifndef SECURE_MEMORY_UNIT_HPP
#define SECURE_MEMORY_UNIT_HPP

#include <systemc>
#include <systemc.h>
#include <map>
using namespace sc_core;

struct Request
{
  uint32_t addr;
  uint32_t data;
  uint8_t r;
  uint8_t w;
  uint32_t fault;
  uint8_t faultBit;
};

struct Result
{
  uint32_t cycles;
  uint32_t errors;
};

extern "C" struct Result run_simulation(
    uint32_t max_cycles,
    const char *tracefile,
    uint8_t endianness,
    uint32_t latency_scrambling,
    uint32_t latency_encryption,
    uint32_t latency_memory_access,
    uint32_t seed,
    uint32_t numRequests,
    struct Request *requests);

SC_MODULE(SecureMemoryUnit)
{

  SC_HAS_PROCESS(SecureMemoryUnit);

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
  bool error_flag;
  uint32_t latency_scrambling;
  uint32_t latency_encryption;
  uint32_t latency_memory_access;
  uint32_t real_latency;
  uint32_t seed;

  sc_out<uint32_t> rdata;
  sc_out<bool> ready;
  sc_out<bool> error;

  // TODO

  SecureMemoryUnit(sc_module_name name,
                   uint8_t endianness, uint32_t latency_scrambling, uint32_t latency_encryption,
                   uint32_t latency_memory_access, uint32_t seed)
      : sc_module(name),
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
        state(seed),
        scrambler_key(generate(seed, state)),
        encryptor_key(generate(seed, state)),
        isBigEndian(endianness),
        latency_scrambling(latency_scrambling),
        latency_encryption(latency_encryption),
        latency_memory_access(latency_memory_access),
        seed(seed),
        error_flag(false)

  {

    SC_THREAD(process);
    sensitive << clk.pos();
  }

  void process()
  {
    real_latency = std::max(latency_scrambling, latency_memory_access);
    while (true)
    {
      wait();

      // print inner state for debugging

      // directly use as physical address, so only one single byte is modified

      if (fault.read() != UINT32_MAX)
      {
        uint32_t index = faultbit.read().to_uint();
        uint32_t fault_address = fault.read();
        if (index == 8)
        {
          parity_memory[fault_address] = !parity_memory[fault_address];
        }
        else
        {
          memory[fault_address] = memory[fault_address] ^ (1 << index);
        }
      }

      uint32_t addresses[4] = {0, 0, 0, 0};
      uint8_t encrypted_data_bytes[4] = {0, 0, 0, 0};

      if (r.read() || w.read())
      {
        // Reset error flag and outputs
        error_flag = false;
        ready.write(false);
        rdata.write(0);

        uint32_t address = addr.read();
        uint32_t p0, p1, p2, p3;
        scramble(address, scrambler_key, p0, p1, p2, p3);
        for (uint32_t i = 0; i < real_latency; ++i)
          wait();
        addresses[0] = p0;
        addresses[1] = p1;
        addresses[2] = p2;
        addresses[3] = p3;
      }
      if (w.read())
      {
        uint32_t data_w = wdata.read();
        uint32_t encrypted_data = encrypt(data_w, encryptor_key);
        for (int i = 0; i < 4; i++)
        {
          uint8_t value = 0;
          uint8_t data_w_value = 0;
          int byteIndex = isBigEndian ? (3 - i) : i;
          value = (encrypted_data >> (i * 8)) & 0xFF;
          encrypted_data_bytes[byteIndex] = value;
        }
        for (int i = 0; i < 4; i++)
        {
          int parity = calculate_parity(encrypted_data_bytes[i]);
          // Debugging output for parity

          printf("address %d: %02X, Parity: %d\n", i, addresses[i], parity);

          parity_memory[addresses[i]] = parity;
          write_memory(addresses[i], encrypted_data_bytes[i]);
        }
        for (uint32_t j = 0; j < latency_memory_access; ++j)
          wait();
        error.write(false);
      }
      if (r.read())
      {
        error_flag = false;
        for (int i = 0; i < 4; i++)
        {
          uint8_t value = 0;
          int byteIndex = isBigEndian ? (3 - i) : i;
          value = read_memory(addresses[i]) & 0xFF;

          encrypted_data_bytes[byteIndex] = value;
        }
        for (uint32_t j = 0; j < latency_memory_access; ++j)
          wait();
        uint32_t data_r = 0;
        for (int i = 0; i < 4; i++)
        {
          int parity = calculate_parity(encrypted_data_bytes[i]);

          if (parity_memory.find(addresses[i]) != parity_memory.end())
          {
            // Check if the address exists in the parity memory
            // If it exists, compare the stored parity with the calculated parity
            int byteIndex = isBigEndian ? (3 - i) : i;

            if (parity_memory[addresses[byteIndex]] != parity)
            {
              error_flag = true;
            }
          }
          data_r |= (encrypted_data_bytes[i] << (i * 8));
        }
        if (data_r != 0)
        {
          data_r = encrypt(data_r, encryptor_key); // Decrypt the data
        } // skip the decryption if data_r unmodified
        if (error_flag)
        {
          error.write(true);
          rdata.write(0);
        }
        
        {
          error.write(false);
          rdata.write(data_r);
        }
      }

      ready.write(true);
    }
  }

  // encryptor
  uint32_t encrypt(uint32_t data, uint32_t key)
  {
    uint32_t encrypted_data = 0;
    encrypted_data = data ^ key;
    return encrypted_data;
  }

  // parity checker
  int calculate_parity(uint8_t data)
  {
    int parity_bit = 0;
    for (int i = 0; i < 8; ++i)
    {
      parity_bit += (data >> i) & 1;
    }

    return parity_bit & 1;
  }

  // address scrambler
  void scramble(uint32_t address, uint32_t key, uint32_t &p0, uint32_t &p1, uint32_t &p2, uint32_t &p3)
  {
    p0 = (address + 0) ^ key;
    p1 = (address + 1) ^ key;
    p2 = (address + 2) ^ key;
    p3 = (address + 3) ^ key;
  }

  // PRNG generator
  uint32_t generate(uint32_t seed, uint32_t &state)
  {

    state = (state * 950706376) % 2147483647;

    return state;
  }

  // read memory
  uint8_t read_memory(uint32_t address)
  {
    return memory[address];
  }

  // write memory
  void write_memory(uint32_t address, uint8_t value)
  {
    memory[address] = value & 0xFF;
  }

  void setScramblingKey(uint32_t key)
  {
    scrambler_key = key;
  }

  void setEncryptionKey(uint32_t key)
  {
    encryptor_key = key;
  }

  uint8_t getByteAt(uint32_t physicalAddress)
  {
    return memory[physicalAddress];
  }
};

#endif // SECURE_MEMORY_UNIT_HPP