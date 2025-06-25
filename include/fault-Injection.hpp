//TODO
#ifndef FAULT_INJECTION_HPP
#define FAULT_INJECTION_HPP
#include <systemc.h>



SC_MODULE(FAULT_INJECTION) {
    sc_in<bool> clk;
    sc_in<uint32_t> fault;
    sc_in<sc_bv<4>> faultbit;
    sc_out<bool> error;

    SC_CTOR(FAULT_INJECTION) {
        SC_METHOD(inject_fault);
        sensitive << clk.pos();
    }

    void inject_fault() {
        if (fault.read() != 0) {
            // Simulate a fault injection based on the fault value and faultbit
            // For example, if fault is 1, we might set error to true
            error.write(true);
        } else {
            error.write(false);
        }
    }
};