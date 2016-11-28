#include <fstream>
#include <string>

#include "serial.hpp"
#include "svftoserial.hpp"

#define DEVICE_PORT "/dev/ttyACM0"

const unsigned BUFFER_SIZE=4096;
const unsigned long BUFFER_MAX_SIZE=200000000;

bool isHexDigit(char ch);
string ExtractInstruction(ifstream& is);
string ExtractAnswer(string line);
void SanitizeInput(string& s);

using namespace std;

int main(int argc, char* argv[])
{
   Serial serialPort;
   //ifstream svf_file("blink_led.svf", fstream::in);
   //ifstream svf_file("id_code.svf", fstream::in);
   ifstream svf_file("myidcode.svf", fstream::in);
   unsigned ret;
   
   char buffer[BUFFER_SIZE];
   char* instruction_buffer;
   long int ib_length;
   bool manual_mode=false;
   string instruction, decodedInstruction, inputArduino, s, s_tmp;
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
         
         while(svf_file.good())
         {
            instruction = ExtractInstruction(svf_file);
            cout << "Stringa da file: " << instruction << endl;

            // Rileviamo se stiamo prendendo il bitstream
            if( instruction.size() > 1024 )
            {
                  // have fun
                  cout << "LOL\n";
            }
            else
            {
                  decodedInstruction = DecodeInstruction(instruction);
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

string ExtractInstruction(ifstream& is)
{
   string s, s_tmp;
   while(is.good())
   {
      getline(is, s);
      if(s[0]!= '/' && s[1]!='/' && s!="")
      {
         while(isHexDigit(is.peek()))
         {
            getline(is, s_tmp);
            s+=s_tmp;
         }
         return s;
      }
   }
   return "";
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
