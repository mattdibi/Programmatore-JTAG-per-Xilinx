#ifndef SVFTOSERIAL_CPP
#define SVFTOSERIAL_CPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "svftoserial.hpp"

// Funzione di decodifica istruzione: prende come argomento la singola linea del file
// (o più linee riassunte in un'unica stringa nel caso del bitstream); esclude automaticamente
// commenti, istruzioni TIR, HIR, TDR, HDR e altre istruzioni che non devono generare uscite.
// Come indicato nel file svf specifico "blink_led.svf" si è assunto che lo stato a cui si passa
// al termine delle SIR e SDR sia quello di IDLE (vederee ENDIR e ENDDR).

string DecodeInstruction (string line) 
{
	if (isalpha(line[0]) && isupper(line[0])) 	// Serve per non considerare commenti
    {
	    istringstream iss(line); 				// Necessario per analizzare linea componente per componente
        string sub;
        iss >> sub;								// Vengono chiamate diverse funzioni a seconda del valore di sub
		if (sub == "SDR")			return GenerateSDROutput (line);
		else if (sub == "SIR") 		return GenerateSIROutput (line);
		else if (sub == "STATE")	return GenerateSTATEOutput (line);
		else if (sub == "RUNTEST")	return GenerateRUNTESTOutput (line);
		else 
		{
			// Handler per altri casi
			return "";
		}
	}
}

string GenerateSDROutput (string line) // Genera output per le istruzioni SDR
{
		istringstream iss(line);
		string sub, str;
		long int n, k;											// n assunto divisibile per 4, altrimenti ulteriori controlli da implementare
		int i;

		iss >> sub; 											// Scarta nome comando
		iss >> n;												// Memorizza dimensione dati, utilizzabile per verifica
		iss >> sub; 											// Scarta parametro
		iss >> sub;												// Prende dati in esadecimale tra parentesi
		
		str += "!*";
		
		for (k = sub.length()-2; k>1; k--)						// Conteggio da ultimo carattere escludendo parentesi di chiusura
		{														// e di apertura
			str += sub[k];
		}

		string binval = hexCharToBin(sub[k]);					// Converte ultimo valore esadecimale in stringa binaria

		for (i=binval.length()-1; i>0; i--)						// Sostituisce '.' e ',' a 0 e 1
		{
			if (binval[i]=='0') 
				str += ".";
			else if(binval[i]=='1') 
				str += ",";
		}

		if (binval[i]=='0') 
			str += ":";
        else if (binval[i]=='1') 
			str += ";";

		str += "!*";

		return str;
}

string GenerateSIROutput (string line) // Genera output per le istruzioni SIR
{
		istringstream iss(line);
		string sub, str;
		int n, k;

		iss >> sub; 											// Scarta nome comando
		iss >> n;												// Memorizza lunghezza istruzione
		iss >> sub; 											// Scarta parametro
		iss >> sub;												// Prende istruzione in esadecimale tra parentesi

		string hexval = sub.substr(1,2);						// Preleva parte tra parentesi
		string binval = hexstrToBinstr(hexval);					// Converte stringa esadecimale in stringa binaria

		str += "!!**";											// Inserisce parte iniziale comune per tutte le SIR
		
		for (k=binval.length()-1; k>binval.length()-n; k--)		// Sostituisce '.' e ',' a 0 e 1
		{
			if (binval[k]=='0') 
				str += '.';
			else if(binval[k]=='1') 
				str += ',';
		}

		// Sostituisce l'ultimo bit con ':' o con ';'
		if (binval[k]=='0') 
			str += ':';
        else if (binval[k]=='1') 
			str += ';';

		str += "!*";											// Inserisce parte finale comune per tutte le SIR
		
		return str;
}

string GenerateSTATEOutput (string line) // Genera output per le istruzioni STATE
{
		istringstream iss(line);
		string sub;
		iss >> sub; 				// Scarta nome comando
		iss >> sub; 				// Prende nome stato
		if (sub == "RESET;")
		{
			return "!!!!!*";
		}
		else if (sub == "IDLE;") 
		{
			return "*";
		}
}

string GenerateRUNTESTOutput (string line) // Genera output per le istruzioni RUNTEST
{
		istringstream iss(line);
		string sub, str;
		long int n;
		int i;

		iss >> sub; 					// Scarta nome comando
		iss >> n;

		// Impostiamo come massimo il numero di caratteri che possiamo inviare sulla seriale
		if(n > 1000)
			n = 1000;

		for (i=0; i < n/4; i++)			// Assumendo che i cicli di clock indicati siano divisibili per 4
		{
			str += '0';					// Tanti '0' quanti sono i cicli di clock richiesti divisi per 4
		}

		return str;
}

vector<string> GenerateBITSTREAMOutput(vector<string> bitstream)
{
	vector<string> decBitstream;
	int indexRow, indexCol, initialCol;
	bool found = false;

	// Ignoriamo tutti i bit della SMASK
	indexRow  = bitstream.size() - 1;

	// Ciclo di ignore fino al carattere ')' che segnala la fine della SMASK
	while(indexRow >= 0 && found == false)
	{
		indexCol = bitstream[indexRow].length();

		while(indexCol >= 0 && found == false)
		{
			if(bitstream[indexRow][indexCol] == ')')
				found = true;

			indexCol--;
		}

		indexRow--;
	}

	indexRow++;

	// Il bitstream va inserito al contrario: dall'ultima riga alla prima
	for(int i = indexRow; i >= 0; i--)
	{
		string str = "";

		// Se è la prima riga dell'istruzione inseriamo la parte iniziale comune a tutte le istruzioni SDR
		// e partiamo con la copia dell'istruzione cominciando dalla posizione della ')'
		if(i == indexRow)
		{
			str += "!*";
			initialCol = indexCol;
		}
		else
		{
			// Se non è la prima riga partiamo dall'ultimo carattere della stringa
			initialCol = bitstream[i].length()-1;
		}

		for (int k = initialCol; k >= 0; k--)
		{
			if(i == 0 && k == 0)
			{
				string binval = hexCharToBin(bitstream[i][k]);

				for (i=binval.length()-1; i>0; i--)
				{
					if (binval[i]=='0') 
						str += ".";
					else if(binval[i]=='1') 
						str += ",";
				}

				if (binval[i]=='0') 
					str += ":";
				else if (binval[i]=='1') 
					str += ";";

				continue;
			}

			if(bitstream[i][k] != '\n' && bitstream[i][k] != '\r')
				str += bitstream[i][k];
		}

		// Se è l'ultima riga immetto la parte finale comune a tutte le istruzioni SDR
		if(i == 0)
			str+= "!*";

		decBitstream.push_back(str);
	}

	return decBitstream;
}

string hexstrToBinstr(string hex)	// Funzione di conversione stringa esadecimale in stringa binaria
{
		string bin;

		for(unsigned k = 0; k != hex.length(); ++k)
		{
			bin += hexCharToBin(hex[k]);
		}

		return bin;
}

string hexCharToBin(char c)	// Funzione di conversione valore esadecimale in valore binario
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
