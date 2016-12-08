#include <fstream>
#include <string>
#include <vector>

#include "serial.hpp"
#include "svftoserial.hpp"

// Impostazioni per il corretto funzionamento del programma
#define DEVICE_PORT "/dev/ttyUSB0"
#define INPUT_FILE "blink_led.svf"

#define INSTR_LENGTH 250
const unsigned BUFFER_SIZE=4096;
const unsigned long BUFFER_MAX_SIZE=200000000;

bool isHexDigit(char ch);
bool isBitStreamInstruction(string instr);
vector<string> ExtractInstruction(ifstream& is);
vector<string> ExtractBitstream(vector<string> instructionsFile, long int currentIndex);
string ExtractAnswer(string line);
void SanitizeInput(string& s);

using namespace std;

int main(int argc, char* argv[])
{
   Serial serialPort;
   ifstream svf_file(INPUT_FILE, fstream::in);
   unsigned ret;
   
   char buffer[BUFFER_SIZE];
   char* instruction_buffer;
   long int ib_length;
   bool manual_mode=false;

   string instruction, decodedInstruction, inputArduino, s, s_tmp;
   vector<string> instructionsFile, bitstream, decodedBitstream;

   if(argc == 2)
   {
      if(!strcmp(argv[1], "-m"))
         manual_mode = true;
   }
   try
   {
      serialPort.Open(DEVICE_PORT,115200);
      cout << "Porta seriale aperta con successo!\n";
      serialPort.FlushReceiver();

      if(manual_mode)
      {
         cout << "Modalità manuale.\n";
         cout << "Inserire comando da inviare (digitare 'quit' per uscire):";
         cin >> s;
         while(s != "quit")
         {
            s += '\n'; //richiesto dalla seriale
            serialPort.WriteString(s.c_str());

            cout << "Comando immesso: " << s;
            sleep(1);

            // Leggiamo da seriale
            serialPort.ReadString(buffer, '\n', BUFFER_SIZE, 5000);
            s = string(buffer);
            SanitizeInput(s);
            // Stampiamo risposta
            cout << endl << s << endl;

            cout << "Nuovo comando:";
            cin >> s;
         }
      }
      else
      {
         instructionsFile = ExtractInstruction(svf_file);

         cout << "Entrato nel ciclo for\n";
         
         for(long int i = 0; i < instructionsFile.size(); i++)
         {
            cout << "Stringa da file: " << instructionsFile[i] << endl;

            // Rileviamo se stiamo prendendo il bitstream
            if( isBitStreamInstruction(instructionsFile[i]) )
            {
                  // Estraiamo vettore di stringhe
                  bitstream = ExtractBitstream(instructionsFile,i);

                  // Decodifica del bitstream
                  decodedBitstream = GenerateBITSTREAMOutput(bitstream);

                  cout << "Upload bitstream in corso:\n";

                   // Ciclo di stampa sulla seriale
                  for(long int j = 0; j < decodedBitstream.size(); j++)
                  {     
                        decodedBitstream[j] += '\n'; //richiesto dalla seriale

                        // Printa l'istruzione decodificata
                        //cout << "Decoded instruction[" << j << "/" << decodedBitstream.size() << "]: " << decodedBitstream[j];

                        serialPort.WriteString(decodedBitstream[j].c_str());

                        // Leggiamo cosa risponde la seriale se la decodedInstruction è diversa da stringa vuota
                        if (decodedBitstream[j] != "\n")
                        {
                              // Ciclo di sincronizzazione
                              do {
                                    // Leggiamo da seriale
                                    ret = serialPort.ReadString(buffer,'\n', BUFFER_MAX_SIZE, 5000);
                                    s_tmp = string(buffer);
                                    SanitizeInput(s_tmp);

                                    // Ricaviamo la risposta dell'arduino
                                    inputArduino = ExtractAnswer(s_tmp);
                              } while (decodedBitstream[j].compare(inputArduino) != 1); // Continua finchè le stringhe non sono uguali
                        }

                        // Printa il progresso dell'upload
                        cout << " Progresso: " << j << "/" << decodedBitstream.size()-1 << "\r" << flush;

                        // Printa la risposta dell'Arduino
                        //cout << "<arduino> " << endl << s_tmp << "</arduino>\n";
                  }

                  cout << "\nUpload bitstream completato.";

                  // Aggiorniamo l'indice "i" in modo che salti tutto il bitstream nel vettore di stringhe
                  i += (bitstream.size() - 1);
            }
            else
            {
                  // Decodifica dell'istruzione
                  decodedInstruction = DecodeInstruction(instructionsFile[i]);

                  if (decodedInstruction != "")
                        cout << "Istruzione decodificata: " << decodedInstruction << endl;

                  decodedInstruction += '\n'; //richiesto dalla seriale

                  serialPort.WriteString(decodedInstruction.c_str());
                  sleep(1);
                  
                  // Leggiamo cosa risponde la seriale se la decodedInstruction è diversa da stringa vuota
                  if (decodedInstruction != "\n")
                  {
                        // Ciclo di sincronizzazione
                        do {
                              // Leggiamo da seriale
                              ret = serialPort.ReadString(buffer,'\n', BUFFER_MAX_SIZE, 5000);
                              s_tmp = string(buffer);
                              SanitizeInput(s_tmp);

                              // Ricaviamo la risposta dell'arduino
                              inputArduino = ExtractAnswer(s_tmp);
                        } while (decodedInstruction.compare(inputArduino) != 1); // Continua finchè le stringhe non sono uguali

                        // Printa la risposta dell'Arduino
                        cout << "Output seriale: " << endl << s_tmp << endl;
                  }
            }

         }
         cout << "Update completato" << endl;
      }
   }
   catch(TimeoutException& e)
   {
      cout << "Timeout exception: " << e.what() << endl;
      serialPort.FlushReceiver();
      serialPort.Close();
      return EXIT_FAILURE;
   }
   catch(exception& e)
   {
      cout << "Standard exception: " << e.what() << endl;
      return EXIT_FAILURE;
   }
   serialPort.Close();
   return EXIT_SUCCESS;
}

void SanitizeInput(string& s)
{
   // Questa funzione permette la stampa sul terminale corretta dell'output dell'arduino
   for(int i=0; i<s.length(); i++)
      if(s[i]=='\t')
         s[i]='\n';
}

vector<string> ExtractInstruction(ifstream& is)
{
   vector<string> instruction;
   char ch;
   int n=0;

   while(is.good()&&!is.eof())
   {
      n=0;
      string s="";

      ch=is.get();

      if(ch==EOF)
            continue;
      else if(ch=='/') // se è un commento ignoriamo tutto
      {
            is.ignore(100, '\n');
            continue;
      }
      else
      {
            // raccolgo i caratteri della riga finchè non incontro un \n o raggiungo la massima lunghezza della istruzione permessa
            do
            {
                  n++;
                  s+=ch;
                  ch=is.get();
            }while(ch != '\n' && n < INSTR_LENGTH);

            // se raggiungo la massima lunghezza della istruzione inserisco un \n come carattere finale
            if(n >= INSTR_LENGTH)
            {
                  s+=ch;
                  s+='\n';
            }
            
            // carico nel vettore la stringa così costruita
            instruction.push_back(s);
      }
   }
   
   return instruction;
}

vector<string> ExtractBitstream(vector<string> instructionsFile, long int currentIndex)
{
      vector<string> stream;
      string tmp, tmp2;
      int i = 0, k = 0;

      // Prima riga: ignoro tutto ciò che viene prima delle parentesi
      while(instructionsFile[currentIndex][i] != '(')
      {
            i++;
      }

      i++;

      // Arrivato alla parentesi prendo tutto
      while(instructionsFile[currentIndex][i] != '\n' && instructionsFile[currentIndex][i] != '\r')
      {
            tmp += instructionsFile[currentIndex][i];
            i++;
      }

      stream.push_back(tmp);

      currentIndex++;

      // Immetto righe finchè l'ultimo carattere di una riga è ')' si noti che così viene caricata anche l'SMASK
      while(instructionsFile[currentIndex][instructionsFile[currentIndex].size() - 4] != ')')
      {
            stream.push_back(instructionsFile[currentIndex]);
            currentIndex++;
      }

      // Immetto ultima riga fino al carattere di ')'
      while(instructionsFile[currentIndex][k] != ')')
      {
            tmp2 += instructionsFile[currentIndex][k];
            k++;
      }

      stream.push_back(tmp2);

      return stream;
}

string ExtractAnswer(string line)
{
      istringstream iss(line);
      string str;
      iss >> str;
      iss >> str;
      iss >> str;

      return str;
}

bool isHexDigit(char ch)
{
   return isdigit(ch) || (ch>='a' && ch<='f');
}

bool isBitStreamInstruction(string instr)
{
      string s = instr.substr(0, 100);

      istringstream iss(s);
      string tmp;
      long int n;

      iss >> tmp;

      if(tmp != "SDR")
            return false;

      iss >> n;

      if(n < 1000)
            return false;
      
      return true;
}
