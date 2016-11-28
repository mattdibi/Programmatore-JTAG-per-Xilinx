#ifndef SVFTOSERIAL_HPP
#define SVFTOSERIAL_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

string DecodeInstruction(string line);
string GenerateSDROutput(string line); // Genera output per le istruzioni SDR
string GenerateSIROutput(string line); // Genera output per le istruzioni SIR
string GenerateSTATEOutput(string line); // Genera output per le istruzioni STATE
string GenerateRUNTESTOutput(string line); // Genera output per le istruzioni RUNTEST
string hexstrToBinstr(string hex);	// Funzione di conversione stringa esadecimale in stringa binaria
string hexCharToBin(char c);	// Funzione di conversione valore esadecimale in valore binario

#endif
