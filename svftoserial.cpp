#ifndef SVFTOSERIAL_CPP
#define SVFTOSERIAL_CPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "svftoserial.hpp"

// Funzione di decodifica istruzione: prende come argomento la singola linea del file
// (o pi� linee riassunte in un'unica stringa nel caso del bitstream); esclude automaticamente
// commenti, istruzioni TIR, HIR, TDR, HDR e altre istruzioni che non devono generare uscite.
// Viene passato anche bufdim per eventuali controlli di "sforamento" del buffer, non ancora
// implementati ma implementabili come miglioria.
// Come indicato nel file svf specifico "blink_led.svf" si � assunto che lo stato a cui si passa
// al termine delle SIR e SDR sia quello di IDLE (vederee ENDIR e ENDDR).

void DecodeInstruction (string& line, char*& buffer, long int& bufdim) 
{
	if (isalpha(line[0]) && isupper(line[0])) 	// Serve per non considerare commenti
    {
	    istringstream iss(line); 				// Necessario per analizzare linea componente per componente
        string sub;
        iss >> sub;					// Vengono chiamate diverse funzioni a seconda del valore di sub
		if (sub == "SDR")			GenerateSDROutput (line, buffer, bufdim);
		else if (sub == "SIR") 		GenerateSIROutput (line, buffer, bufdim);
		else if (sub == "STATE")	GenerateSTATEOutput (line, buffer, bufdim);
		else if (sub == "RUNTEST")	GenerateSTATEOutput (line, buffer, bufdim);
		else 
		{
		// Eventuale handler per altri casi
		// costruisce stringa vuota
		   bufdim = 1;
		   buffer = new char[bufdim];
		   buffer[0] = '\0';
		}
	}
}

void GenerateSDROutput (string& line, char*& buffer, long int& bufdim) // Genera output per le istruzioni SDR
{
		istringstream iss(line);
		string sub;
		long int n, k;											// n assunto divisibile per 4, altrimenti ulteriori controlli da implementare
		int i;
		iss >> sub; 											// Scarta nome comando
		iss >> n;												// Memorizza dimensione dati, utilizzabile per verifica
		bufdim = n + 3 + 3 + 3;									// Condidera dati, ultimo carattere e caratteri iniziali e finali
		buffer = new char [bufdim];
		iss >> sub; 											// Scarta parametro
		iss >> sub;												// Prende dati in esadecimale tra parentesi
		buffer[0] = '!'; buffer[1] = '*'; buffer[2] = '*';		// Parte iniziale SDR. 1 o 2 asterischi? se 1 diminuire di 1 conteggi buffer succ.
		for (k = sub.length()-2; k>1; k--)						// Conteggio da ultimo carattere escludendo parentesi di chiusura
		{														// e di apertura
			buffer[sub.length()-2-k+3] = sub[k];				// Riempie buffer partendo da ultimo carattere
		}
		string binval = hexCharToBin(sub[k]);					// Converte ultimo valore esadecimale in stringa binaria
		for (int i=binval.length()-1; i>0; i--)					// Sostituisce '.' e ',' a 0 e 1
		{
			if (binval[i]=='0') buffer[(sub.length()-2-k+3) + (binval.length()-1-i)] = '.';
			else if(binval[i]=='1') buffer[(sub.length()-2-k+3) + (binval.length()-1-i)] = ',';
		}
		if (binval[i]=='0') buffer[(sub.length()-2-k+3) + (binval.length()-1-i)] = ':';		// Sostituisce l'ultimo 0 o 1 con ':' 
        else if (binval[i]=='1') buffer[(sub.length()-2-k+3) + (binval.length()-1-i)]= ';';	// o ';'
		buffer[(sub.length()-2-k+3) + (binval.length()-1-i) + 1] = '!';		// Parte finale comune a tutti gli SDR
		buffer[(sub.length()-2-k+3) + (binval.length()-1-i) + 2] = '*';
		buffer[(sub.length()-2-k+3) + (binval.length()-1-i) + 3] = '\n';	// Delimitatore buffer
}

void GenerateSIROutput (string& line, char*& buffer, long int& bufdim) // Genera output per le istruzioni SIR
{
		istringstream iss(line);
		string sub, str;
		int n, k;
		iss >> sub; 											// Scarta nome comando
		iss >> n;												// Memorizza lunghezza istruzione
		bufdim = n + 4 + 3;										// Considera istruzione e caratteri iniziali e finali
		buffer = new char [bufdim];
		iss >> sub; 											// Scarta parametro
		iss >> sub;												// Prende istruzione in esadecimale tra parentesi
		string hexval = sub.substr(1,2);						// Preleva parte tra parentesi
		string binval = hexstrToBinstr(hexval);					// Converte stringa esadecimale in stringa binaria
		str += "!!**";											// Inserisce parte iniziale comune per tutte le SIR
		for (k=binval.length()-1; k>binval.length()-n; k--)	// Sostituisce '.' e ',' a 0 e 1
		{
			if (binval[k]=='0') str += '.';
			else if(binval[k]=='1') str += ',';
		}
		if (binval[k]=='0') str += ':';							// Sostituisce l'ultimo 0 o 1 con ':' 
        else if (binval[k]=='1') str += ';';					// o ';'
		str += "!*";											// Inserisce parte finale comune per tutte le SIR
		str.copy(buffer,str.length(),0);
		buffer[str.length()] = '\n';							// Carattere di delimitazione
}

void GenerateSTATEOutput (string& line, char*& buffer, long int& bufdim) // Genera output per le istruzioni STATE
{
		istringstream iss(line);
		string sub;
		iss >> sub; 				// Scarta nome comando
		iss >> sub; 				// Prende nome stato
		if (sub == "RESET;")
		{
			bufdim = 7;
			buffer = new char [bufdim];
			string str = "!!!!!*";
			str.copy(buffer,6,0);	// Copia stringa comando nel buffer
			buffer[6] = '\n';		// Carattere di delimitazione
		}
		else if (sub == "IDLE;") 
		{
			bufdim = 2;
			buffer = new char [bufdim];
			string str = "*";
			str.copy(buffer,1,0);
			buffer[1] = '\n';
		}
}

void GenerateRUNTESTOutput (string& line, char*& buffer, long int& bufdim) // Genera output per le istruzioni RUNTEST
{
		istringstream iss(line);
		string sub, str;
		int n, i;
		iss >> sub; 					// Scarta nome comando
		iss >> n;
		bufdim = n/4 + 1;
		buffer = new char [bufdim];
		for (i=0; i < n/4; i++)			// Assumendo che i cicli di clock indicati siano divisibili per 4
		{
			str += '0';					// Tanti '0' quanti sono i cicli di clock richiesti divisi per 4
		}
		str.copy(buffer,str.length(),0);
		buffer[str.length()] = '\n';	// Carattere di delimitazione
}

string hexstrToBinstr(const string& hex)	// Funzione di conversione stringa esadecimale in stringa binaria
{
		string bin;
		for(unsigned k = 0; k != hex.length(); ++k)
			bin += hexCharToBin(hex[k]);
		return bin;
}

const char* hexCharToBin(char c)	// Funzione di conversione valore esadecimale in valore binario
{
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
    }
}

#endif

// Modifiche:
// 1. In tutte le funzioni l'argomento long int bufdim diventa long int& bufdim
// 2. Aggiunta inizializzazione bufdim in tutte e 4 le funzioni
// 3. Aggiunto buffer = new char [bufdim]; in ogni funzione
