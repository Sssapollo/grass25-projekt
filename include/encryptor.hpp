//TODO
#ifndef ENCRYPTOR_HPP
#define ENCRYPTOR_HPP
#include <systemc.h>

SC_MODULE(Encryptor){
    sc_in<bool> clk;
    sc_in<uint8_t> key;  
    sc_in<uint8_t> data_in;
    sc_out<uint8_t> data_out;


    SC_CTOR(Encryptor) {
        SC_THREAD(encrypt);
        sensitive << clk.pos();
    }




    void encrypt() {
       while(true){
        wait();
        uint8_t input = data_in.read();
        uint8_t encryption_key = key.read();
        uint8_t encrypted_data = input ^ encryption_key; 
        data_out.write(encrypted_data);
       }
    }







};
#endif 
