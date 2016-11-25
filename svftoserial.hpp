#ifndef SVFTOSERIAL_HPP
#define SVFTOSERIAL_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

void DecodeInstruction(string& line, char*& buffer, long int& bufdim);
void GenerateSDROutput(string& line, char*& buffer, long int& bufdim); // Genera output per le istruzioni SDR
void GenerateSIROutput(string& line, char*& buffer, long int& bufdim); // Genera output per le istruzioni SIR
void GenerateSTATEOutput(string& line, char*& buffer, long int& bufdim); // Genera output per le istruzioni STATE
void GenerateRUNTESTOutput(string& line, char*& buffer, long int& bufdim); // Genera output per le istruzioni RUNTEST
string hexstrToBinstr(const string& hex);	// Funzione di conversione stringa esadecimale in stringa binaria
const char* hexCharToBin(char c);	// Funzione di conversione valore esadecimale in valore binario

#endif
