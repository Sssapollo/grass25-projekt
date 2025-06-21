#include <systemc>
#include <iostream>
#include "../include/prng.hpp"

uint32_t run_simulation(uint32_t seed) {
    sc_clock clk("clk", 1, SC_NS);
    sc_signal<uint32_t> result;
    sc_signal<uint32_t> seed_signal;

    PRNG prng("prng");
    prng.clk(clk);
    prng.seed(seed_signal);
    prng.out(result);

    seed_signal.write(seed);
    sc_start(1, SC_SEC);
    return result.read();
}

// Note that we need this default sc_main implementation.
int sc_main(int argc, char* argv[])
{
    std::cout << "ERROR" << std::endl;
    return 1;
}