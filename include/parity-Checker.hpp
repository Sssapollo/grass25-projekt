//TODO
#ifndef PARITY_CHECKER_HPP
#define PARITY_CHECKER_HPP
#include <systemc.h>


SC_MODULE(PARITY_CHECKER) {
    sc_in<bool> clk;
    sc_in<uint8_t> data_in;  
    sc_in<bool> we;
    sc_in<bool> re;
    sc_in<uint32_t> addr;

    std::map<uint32_t, sc_bv<1>> parity_memory;
    sc_out<bool> parity_out;  

    SC_CTOR(PARITY_CHECKER) {
        SC_THREAD(process_parity);
        sensitive << clk.pos();
    }
    int calculate_parity(uint32_t data) {
        
        int parity_bit = 0;
        for (int i = 0; i < 8; ++i) {
            parity_bit += (data >> i) & 1;  
        }

        return parity_bit%2;
    }

    void process_parity() {
       while(true){
        wait();
        if (we.read()){
            uint32_t address = addr.read();
            uint8_t data = data_in.read();
            int parity_bit = calculate_parity(data);
            parity_memory[address] = sc_bv<1>(parity_bit);          
        }
        if (re.read()) {
            uint32_t address = addr.read();
            uint8_t data = data_in.read();
            if(parity_memory[addr]!= sc_bv<1>(calculate_parity(data))) {
                parity_out.write(false);  
            } else {
                parity_out.write(true); 
            }
        }

        


       }
    }
};

#endif 