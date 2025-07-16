#ifndef TINYRISC_HPP
#define TINYRISC_HPP

#include <systemc>
#include <systemc.h>
#include <map>
using namespace sc_core;

#include "cu.hpp"
#include "main_memory.hpp"
#include "register_file.hpp"
#include "alu.hpp"
#include "multiplexer.hpp"
#include "pc.hpp"

SC_MODULE(TINY_RISC) {
  sc_in<bool> reset;
  sc_clock clk;

  std::map<uint32_t, uint32_t> initialMemory;

  PC pc;
  REGISTER_FILE rf;
  ALU alu;
  MAIN_MEMORY mm;
  CU cu;

  MULTIPLEXER_I32 muxReg1;
  MULTIPLEXER_I32 muxReg2;
  MULTIPLEXER_I32 muxRegW;
  MULTIPLEXER_I32 muxMemAddr;
  MULTIPLEXER_I32 muxMemR;
  MULTIPLEXER_I32 muxAluRo;

  sc_signal<uint32_t> rf_r1_muxReg1;
  sc_signal<uint32_t> muxReg1_r1_alu;
  sc_signal<uint32_t> muxReg1_next_pc;
  sc_signal<uint32_t> rf_r2_muxReg2;
  sc_signal<uint32_t> muxReg2_r2_alu;
  sc_signal<uint32_t> muxReg2_wdata_mm;
  sc_signal<uint32_t> muxRegW_wdata_rf;
  sc_signal<uint32_t> alu_ro_muxAluRo;
  sc_signal<uint32_t> muxAluRo_r0_muxRegW;
  sc_signal<uint32_t> muxAluRo_r0_cu;

  sc_signal<bool> alu_overflow_cu;
  sc_signal<sc_bv<3>> cu_op_alu;

  sc_signal<uint32_t> pc_pc_cu;
  sc_signal<bool> pc_enable_cu;
  sc_signal<sc_bv<2>> pc_j_cu;
  sc_signal<sc_bv<12>> cu_offset_pc;

  sc_signal<uint32_t> mm_rdata_muxMemR;
  sc_signal<uint32_t> muxMemR_rdata_cu;
  sc_signal<uint32_t> muxMemR_muxRegW;
  
  sc_signal<bool> mm_ready_cu;
  sc_signal<bool> cu_r_mm;
  sc_signal<bool> cu_w_mm;

  sc_signal<uint32_t> muxReg1_muxMemAddr;
  sc_signal<uint32_t> cu_addr_muxMemAddr;
  sc_signal<uint32_t> muxMemAddr_addr_mm;

  sc_signal<sc_bv<5>> cu_a1_rf;
  sc_signal<sc_bv<5>> cu_a2_rf;
  sc_signal<sc_bv<5>> cu_a3_rf;
  sc_signal<bool> cu_we_rf;

  sc_signal<uint8_t> muxReg1_select;
  sc_signal<uint8_t> muxReg2_select;
  sc_signal<uint8_t> muxMemR_select;
  sc_signal<uint8_t> muxAluRo_select;
  sc_signal<uint8_t> cu_select_muxRegW;
  sc_signal<uint8_t> cu_select_muxMemAddr;


  SC_CTOR(TINY_RISC);
  TINY_RISC(sc_module_name name, std::map<uint32_t, uint32_t> initialMemory, uint32_t period) : 
      sc_module(name), clk("clk", period, SC_NS), initialMemory(initialMemory),
      pc("pc"), rf("rf"), alu("alu"), mm("mm"), cu("cu"),
      muxReg1("muxReg1", 1, 3), muxReg2("muxReg2", 1, 2), muxRegW("muxRegW", 2, 1),
      muxMemAddr("muxMemAddr", 2, 1), muxMemR("muxMemR", 1, 2), muxAluRo("muxAluRo", 1, 2) {
    rf.clk(clk);
    mm.clk(clk);
    cu.clk(clk);
    pc.clk(clk);
    // Set the wires
    // 1.1
    rf.r1.bind(rf_r1_muxReg1);
    muxReg1.in[0](rf_r1_muxReg1);
    alu.r1.bind(muxReg1_r1_alu);
    muxReg1.out[0](muxReg1_r1_alu);
    pc.next.bind(muxReg1_next_pc);
    muxReg1.out[1](muxReg1_next_pc);
    rf.r2.bind(rf_r2_muxReg2);
    muxReg2.in[0](rf_r2_muxReg2);
    alu.r2.bind(muxReg2_r2_alu);
    muxReg2.out[0](muxReg2_r2_alu);
    mm.wdata.bind(muxReg2_wdata_mm);
    muxReg2.out[1](muxReg2_wdata_mm);
    rf.wdata.bind(muxRegW_wdata_rf);
    muxRegW.out[0](muxRegW_wdata_rf);
    alu.ro.bind(alu_ro_muxAluRo);
    muxAluRo.in[0](alu_ro_muxAluRo);
    muxRegW.in[0](muxAluRo_r0_muxRegW);
    muxAluRo.out[0](muxAluRo_r0_muxRegW);
    // 1.2
    alu.overflow.bind(alu_overflow_cu);
    cu.overflow.bind(alu_overflow_cu);
    alu.op.bind(cu_op_alu);
    cu.op.bind(cu_op_alu);
    muxAluRo.out[1](muxAluRo_r0_cu);
    cu.ro.bind(muxAluRo_r0_cu);
    // 1.3
    pc.pc.bind(pc_pc_cu);
    cu.pcin.bind(pc_pc_cu);
    pc.enable.bind(pc_enable_cu);
    cu.pcenable.bind(pc_enable_cu);
    pc.j.bind(pc_j_cu);
    cu.pcj.bind(pc_j_cu);
    pc.offset.bind(cu_offset_pc);
    cu.pcoffset.bind(cu_offset_pc);
    // 1.4
    mm.rdata.bind(mm_rdata_muxMemR);
    muxMemR.in[0](mm_rdata_muxMemR);
    cu.datain.bind(muxMemR_rdata_cu);
    muxMemR.out[0](muxMemR_rdata_cu);
    muxRegW.in[1](muxMemR_muxRegW);
    muxMemR.out[1](muxMemR_muxRegW);
    // 1.5
    mm.ready.bind(mm_ready_cu);
    cu.memready.bind(mm_ready_cu);
    mm.r.bind(cu_r_mm);
    cu.rmem.bind(cu_r_mm);
    mm.w.bind(cu_w_mm);
    cu.wmem.bind(cu_w_mm);
    // 1.6
    muxMemAddr.in[0](muxReg1_muxMemAddr);
    muxReg1.out[2](muxReg1_muxMemAddr);
    muxMemAddr.in[1](cu_addr_muxMemAddr);
    cu.addr.bind(cu_addr_muxMemAddr);
    mm.addr.bind(muxMemAddr_addr_mm);
    muxMemAddr.out[0](muxMemAddr_addr_mm);
    // 1.7
    rf.a1.bind(cu_a1_rf);
    rf.a2.bind(cu_a2_rf);
    rf.a3.bind(cu_a3_rf);
    rf.we.bind(cu_we_rf);
    cu.a1.bind(cu_a1_rf);
    cu.a2.bind(cu_a2_rf);
    cu.a3.bind(cu_a3_rf);
    cu.we.bind(cu_we_rf);
    // 1.8
    muxReg1.select.bind(muxReg1_select);
    muxReg2.select.bind(muxReg2_select);
    muxMemR.select.bind(muxMemR_select);
    muxAluRo.select.bind(muxAluRo_select);
    muxReg1_select.write(0);
    muxReg2_select.write(0);
    muxMemR_select.write(0);
    muxAluRo_select.write(0);
    cu.rsource.bind(cu_select_muxRegW);
    muxRegW.select.bind(cu_select_muxRegW);
    cu.msource.bind(cu_select_muxMemAddr);
    muxMemAddr.select.bind(cu_select_muxMemAddr);


    
    doReset();
    SC_THREAD(behaviour);
    sensitive << clk.posedge_event();
  }

  void behaviour() {
    while(true) {
      wait();
      if(reset.read()) {
        doReset();
      }
    }
  }

  void doReset() {
    pc.pcValue = 0x00001000;
    for(int i = 0; i < 32; i++) {
      rf.registers_data[i] = 0;
    }
    mm.memory.clear();
    for(auto it = initialMemory.begin(); it != initialMemory.end(); it++) {
      mm.set(it->first, it->second);
    }
  }

  uint32_t getMemoryValue(uint32_t address) {
    return mm.get(address);
  }
};


#endif // TINYRISC_HPP