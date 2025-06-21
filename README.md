1. LCG-based PRNG
how to choose A,B,M in the formular RandSeed = (A * RandSeed + B) % M
The period of PRNG is less than or equal to M 
requirements to make the period maximum:
 1.1 B and M are relatively prime
 1.2 All prime factors of M are divisible by A-1;
 1.3 If M is a multiple of 4, so is A-1;
 1.4 A, B, N[0] ARE SMALLER THAN M
 