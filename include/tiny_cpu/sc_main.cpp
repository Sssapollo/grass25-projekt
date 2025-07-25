#include <systemc>
#include "tinyRISC2.hpp"
#include <iostream>

// use namespace sc_core;


int sc_main(int argc, char* argv[]) {
    


    sc_signal<bool> reset;

    std::map<unsigned int, unsigned int> program;

    
    program[0x1000] = 0b00000000000000000110010100110011; // INC x10, x0
    program[0x1004] = 0b00000000101001010100010110110011; // ADD x11, x10, x10
    program[0x1008] = 0b00000000101101011100011000110011; // ADD x12, x11, x11
    program[0x100C] = 0b00000000110001100100011010110011; // ADD x13, x12, x12
    program[0x1010] = 0b00000000110101101100011100110011; // ADD x14, x13, x13
    program[0x1014] = 0b00000000111001110100011110110011; // ADD x15, x14, x14
    program[0x1018] = 0b00000000111101111100100000110011; // ADD x16, x15, x15
    program[0x101C] = 0b00000001000010000100100010110011; // ADD x17, x16, x16
    program[0x1020] = 0b00000001000110001100100100110011; // ADD x18, x17, x17
    program[0x1024] = 0b00000001001010010100100110110011; // ADD x19, x18, x18
    program[0x1028] = 0b00000001001110011100101000110011; // ADD x20, x19, x19
    program[0x102C] = 0b00000001010010100100101010110011; // ADD x21, x20, x20
    program[0x1030] = 0b00000001010110101100101100110011; // ADD x22, x21, x21
    program[0x1034] = 0b00000001000010110100001010110011; // ADD x5 , x22, x16
    program[0x1038] = 0b00000000110101011100000010110011; // ADD x1 , x11, x13
    program[0x103C] = 0b00000000000000000110000110110011; // INC x3 , x0 
    program[0x1040] = 0b00000000000100100100001000110011; // ADD x4 , x4 , x1
    program[0x1044] = 0b00000000001100001101000010110011; // SUB x1, x1, x3
    program[0x1048] = 0b00000000000000001011010001100011; // BLT x1, x0, 8
    program[0x104C] = 0b00000000000000101000000001100011; // J x5 
    program[0x1050] = 0b00000000010010010001000000100011; // SW  x18 , x4

    const unsigned int period = 1;


    TINY_RISC cpu("cpu", program, period);
    // cpu.clk(clk);
    cpu.reset(reset);

    // reset.write(true);
    // sc_start(100,SC_US);   // 1 cycle
    reset.write(false);

    sc_start(250,SC_NS);   // 10 cycles

    for (size_t i = 10; i < 23; i++)
    {
      std::cout << cpu.rf.registers_data[i] << ' ';
    }
    std::cout << cpu.rf.registers_data[5] << " x5 ";
    std::cout << cpu.rf.registers_data[4] << " x4 ";
    std::cout << cpu.rf.registers_data[3] << " x3 ";
    std::cout << cpu.rf.registers_data[1] << " x1 ";
    std::cout << cpu.getMemoryValue(256) << ' ';
    std::cout << std::endl;

    return 0;
}