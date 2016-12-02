#ifndef SVFTOSERIAL_HPP
#define SVFTOSERIAL_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

// Chiama la corretta funzione di generazione output
string DecodeInstruction(string line);
// Genera output per le istruzioni SDR
string GenerateSDROutput(string line);
// Genera output per le istruzioni SIR
string GenerateSIROutput(string line);
// Genera output per le istruzioni STATE
string GenerateSTATEOutput(string line);
// Genera output per le istruzioni RUNTEST
string GenerateRUNTESTOutput(string line);
// Genera output per l'istruzione di caricamento del bitstream
vector<string> GenerateBITSTREAMOutput(vector<string> bitstream);

// Funzione di conversione stringa esadecimale in stringa binaria
string hexstrToBinstr(string hex);
// Funzione di conversione valore esadecimale in valore binario
string hexCharToBin(char c);

#endif
