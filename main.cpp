#include <fstream>
#include <string>
#include "serial.hpp"
#include "svftoserial.hpp"

#define DEVICE_PORT "/dev/ttyUSB0"

const unsigned BUFFER_SIZE=4096;
const unsigned long BUFFER_MAX_SIZE=200000000;

bool isHexDigit(char ch);
string ExtractInstruction(ifstream& is);
void SanitizeInput(string& s);

using namespace std;

int main(int argc, char* argv[])
{
   Serial serialPort;
   ifstream svf_file("blink_led.svf", fstream::in);
   unsigned ret;
   
   char buffer[BUFFER_SIZE];
   char* instruction_buffer;
   long int ib_length;
   bool manual_mode=false;
   string s;
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
         while(svf_file.good())
         {
            cout<<"Entrato nel while"<<endl;
            s=ExtractInstruction(svf_file);
            cout<<"Stringa da file: "<<s<<endl;
            DecodeInstruction(s, instruction_buffer, ib_length);
            s=string(instruction_buffer);
            cout<<"Stringa decodificata: "<<s<<endl;
            s+='\n'; //perché sennò la seriale si incazza
            serialPort.WriteString(s.c_str());
            sleep(1);
            /*ret=serialPort.ReadString(buffer,'\n', BUFFER_MAX_SIZE, 5000);
            s_tmp=string(buffer);
            SanitizeInput(s_tmp);
            cout<<s_tmp;*/
         }
         cout<<"Update successful"<<endl;
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

bool isHexDigit(char ch)
{
   return isdigit(ch) || (ch>='a' && ch<='f');
}