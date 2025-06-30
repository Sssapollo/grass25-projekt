#ifndef PRNG_H
#define PRNG_H

#include <systemc.h>

// LCG-based Pseudo-Random Number Generator (PRNG) module
// the generation function is X_{n+1} = (A * X_n + C) mod M
// X_{0} is the seed, which is given from the in_port seed;
//Generators recommended by Fishman (1990) modul M = 2^31 - 1 = 2147483647 A = 950706376 B = 0
//Quelle:https://statmath.wu.ac.at/software/src/prng-3.0.2/doc/prng.html/Table_LCG.html

SC_MODULE(PRNG){

    sc_in<bool> clk;
    sc_in<uint32_t> seed;
    sc_out<uint32_t> out;
    uint32_t state;
    SC_CTOR(PRNG) {
        state = 0; 
        SC_THREAD(generate);
        sensitive << clk.pos();
    }
    
    
    void generate() {
       while(true){
        wait();
         
        if (seed.read() != 0 && state == 0) {
            state = seed.read();
        } else {
            state = (state * 950706376) % 2147483647; 
        }
        out.write(state);
       }
    }

};

#endif // PRNG_HPP
