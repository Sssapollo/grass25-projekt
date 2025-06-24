//TODO
#ifndef ADDRESS_SCRAMBLER
#define ADDRESS_SCRAMBLER
#include <systemc.h>


SC_MODULE(AddressScramblerSC) {

    sc_in<sc_uint<32>> logical_addr;   
    sc_in<sc_uint<32>> scrambling_key;  
    sc_in<bool> clk;

    sc_out<sc_uint<32>> phys_addr0;      
    sc_out<sc_uint<32>> phys_addr1;      
    sc_out<sc_uint<32>> phys_addr2;     
    sc_out<sc_uint<32>> phys_addr3;     


    SC_CTOR(AddressScramblerSC) {

        SC_METHOD(scramble_proc);
        sensitive << clk.pos();
    }


    void scramble_proc() {
        sc_uint<32> addr = logical_addr.read();
        sc_uint<32> key  = scrambling_key.read();


        phys_addr0.write( (addr + 0) ^ key );
        phys_addr1.write( (addr + 1) ^ key );
        phys_addr2.write( (addr + 2) ^ key );
        phys_addr3.write( (addr + 3) ^ key );
    }
};

#endif  // ADDRESS_SCRAMBLER_SC_H
