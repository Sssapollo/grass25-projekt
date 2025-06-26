#include <systemc>

#include "../include/secure-Memory-Unit.hpp"

struct Request {
    uint32_t addr;
    uint32_t data;      
    uint8_t  r;         
    uint8_t  w;        
    uint32_t fault;    
    uint8_t  faultBit; 
};

struct Result {
    uint32_t cycles;
    uint32_t errors;
};
struct Result run_simulation(
    const char *file_txt,
    uint32_t max_cycles,
    const char *tracefile,
    uint8_t endianness,
    uint32_t latency_scrambling,
    uint32_t latency_encryption,
    uint32_t latency_memory_access,
    uint32_t seed,
    uint32_t numRequests,
    struct Request *requests) {
    sc_clock clk("clk", 1, SC_SEC);
    sc_signal<uint32_t> addr, wdata, fault;
    sc_signal<bool>     r, w;
    sc_signal<sc_dt::sc_bv<4>> faultbit;
    sc_signal<uint32_t> rdata;
    sc_signal<bool>     ready, error;
    SecureMemoryUnit smu("smu", endianness, latency_scrambling, latency_encryption, latency_memory_access, seed);

    if(tracefile != NULL) {
        sc_trace_file *tf = sc_create_vcd_trace_file(tracefile);
        sc_trace(tf, clk, "clk");
        sc_trace(tf, addr, "addr");
        sc_trace(tf, wdata, "wdata");
        sc_trace(tf, r, "r");
        sc_trace(tf, rdata, "rdata");
        }

    smu.clk(clk); 
    smu.addr(addr); 
    smu.wdata(wdata);
    smu.r(r); 
    smu.w(w);
    smu.fault(fault);
    smu.faultbit(faultbit);
    smu.rdata(rdata); 
    smu.ready(ready); 
    smu.error(error);

    fault.write(UINT32_MAX);
    faultbit.write("0000");
    addr.write(0);
    wdata.write(0);
    r.write(false);
    w.write(false);
    sc_start(1, SC_NS); 
    int err_count = 0;
    int cyc_count = 0;
    for (uint32_t i = 0; i < numRequests && cyc_count < max_cycles; ++i){
        if (requests[i].fault != UINT32_MAX) {
            fault.write(requests[i].fault);
            faultbit.write(requests[i].faultBit & 0xF);
        } else {
            fault.write(UINT32_MAX);
        } 
       if (requests[i].r) {
            r.write(true);  w.write(false);
        } else {                              
            w.write(true);  r.write(false);
            wdata.write(requests[i].data);
        }
        while (!ready.read() && cyc_count < max_cycles){
            sc_start(1, SC_NS);
            cyc_count++;
        }
        if(requests[i].r){
            rdata.write(requests[i].data);
        }
        if (error.read()){
            err_count++;
        }

        r.write(false); 
        w.write(false);
        fault.write(UINT32_MAX);
        sc_start(1, SC_NS); ++cyc_count;
    }


    struct Result res = {
         cyc_count,
         err_count
    };
    return res;

}

int sc_main(int argc, char* argv[])
{
    std::cout << "ERROR" << std::endl;
    return 1;
}