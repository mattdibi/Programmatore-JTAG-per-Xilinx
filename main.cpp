#include <fstream>
#include <string>
#include <vector>

#include "serial.hpp"
#include "svftoserial.hpp"

#define DEVICE_PORT "/dev/ttyACM0"

const unsigned BUFFER_SIZE=4096;
const unsigned long BUFFER_MAX_SIZE=200000000;

bool isHexDigit(char ch);
vector<string> ExtractInstruction(ifstream& is);
string ExtractAnswer(string line);
void SanitizeInput(string& s);

using namespace std;

int main(int argc, char* argv[])
{
   Serial serialPort;
   ifstream svf_file("blink_led.svf", fstream::in);
   //ifstream svf_file("myidcode.svf", fstream::in);
   unsigned ret;
   
   char buffer[BUFFER_SIZE];
   char* instruction_buffer;
   long int ib_length;
   bool manual_mode=false;

   string instruction, decodedInstruction, inputArduino, s, s_tmp;
   vector<string> instructionsFile;

   if(argc==2)
   {
      if(!strcmp(argv[1], "-m"))
         manual_mode=true;
   }
   try
   {
      serialPort.Open(DEVICE_PORT,9600);
      cout<<"Serial port opened successfully!"<<endl;
      serialPort.FlushReceiver();
      //serialPort.ReadString(buffer, '\n', BUFFER_SIZE, 10000);
      //s=string(buffer);
      //SanitizeInput(s);
      //cout<<s<<endl;
      if(manual_mode)
      {
         cin>>s;
         while(s!="quit")
         {
            s+='\n'; // Perché sennò la seriale si incazza
            serialPort.WriteString(s.c_str());
            //serialPort.FlushReceiver();
            cout<<"comando da mandare: "<<s<<endl;
            sleep(1);
            serialPort.ReadString(buffer, '\n', BUFFER_SIZE, 5000);
            s=string(buffer);
            SanitizeInput(s);
            cout<<"---------------------------------------------------------"<<endl<<s<<"---------------------------------------------------------"<<endl;
            cin>>s;
         }
      }
      else
      {
         cout << "Entrato nel while" << endl;

         instructionsFile = ExtractInstruction(svf_file);
         
         for(int i=0;i<instructionsFile.size();i++)
         {
            cout << "Stringa da file: " << instructionsFile[i] << endl;

            // Rileviamo se stiamo prendendo il bitstream
            if( instruction.size() > 1024 )
            {
                  // Estraiamo vettore di stringhe

                  // Specchiamo tutte le stringhe

                  // Ciclo di stampa sulla seriale dall'ultima stringa ricevuta fino alla prima
            }
            else
            {
                  decodedInstruction = DecodeInstruction(instructionsFile[i]);
                  if (decodedInstruction != "")
                        cout << "Decoded instruction: " << decodedInstruction << endl;
                  decodedInstruction += '\n'; //perché sennò la seriale si incazza

                  serialPort.WriteString(decodedInstruction.c_str());
                  sleep(1);
                  
                  // Leggiamo cosa risponde la seriale se la decodedInstruction è diversa da stringa vuota
                  if (decodedInstruction != "\n")
                  {
                        // Ciclo di attesa per dare il tempo all'Arduino di rispondere
                        do {
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
         cout << "Update successful" << endl;
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
      else if(ch=='/')
      {
            is.ignore(100, '\n');
            continue;
      }
      else
      {
            do
            {
                  n++;
                  s+=ch;
                  ch=is.get();
            }while(ch!='\n' && n<100);

            if(n >= 100)
            {
                  s+=ch;
                  s+='\n';
            }
            
            instruction.push_back(s);
      }
   }
   instruction.push_back("");
   return instruction;
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
