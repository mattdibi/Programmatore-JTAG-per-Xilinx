#ifndef SERIAL_CPP
#define SERIAL_CPP

#include "serial.hpp"

Serial::Serial()
{}

Serial::~Serial()
{
    Close();
}

void Serial::Open(const char *Device,const unsigned Bauds)
{
   
   struct termios options;

   fd = open(Device, O_RDWR | O_NOCTTY | O_NDELAY);                    
   if (fd == -1)
      throw ErrorOpeningException();

   fcntl(fd, F_SETFL, FNDELAY);
   tcgetattr(fd, &options);
   bzero(&options, sizeof(options));
   speed_t Speed;
   switch (Bauds)
   {
      case 110:
         Speed=B110; 
         break;
      case 300:
         Speed=B300;
         break;
      case 600:
         Speed=B600;
         break;
      case 1200:
         Speed=B1200;
         break;
      case 2400:
         Speed=B2400;
         break;
      case 4800:
         Speed=B4800;
         break;
      case 9600:
         Speed=B9600;
         break;
      case 19200:
         Speed=B19200;
         break;
      case 38400:
         Speed=B38400;
         break;
      case 57600:
         Speed=B57600;
         break;
      case 115200:
         Speed=B115200;
         break;
      default:
         throw ErrorSpeedException();
   }
   cfsetispeed(&options, Speed);
   cfsetospeed(&options, Speed);
   options.c_cflag |= ( CLOCAL | CREAD |  CS8);
   options.c_iflag |= ( IGNPAR | IGNBRK );
   options.c_cc[VTIME]=0;
   options.c_cc[VMIN]=0;
   tcsetattr(fd, TCSANOW, &options);
   return;
}

void Serial::Close()
{
   close (fd);
}

void Serial::WriteChar(const char Byte)
{
   if (write(fd,&Byte,1)!=1)
      throw ErrorWriteException();
   return;
}

void Serial::WriteString(const char *String)
{
   int length=strlen(String);
   if (write(fd,String,length)!=length)
      throw ErrorWriteException();
   return;
}

void Serial::Write(const void *Buffer, const unsigned NbBytes)
{
   if (write (fd,Buffer,NbBytes)!=(ssize_t)NbBytes)
      throw ErrorWriteException();
   return;
}

unsigned Serial::ReadChar(char *pByte,unsigned TimeOut_ms)
{
   TimeOut Timer;
   Timer.InitTimer();
   while (Timer.ElapsedTime_ms()<TimeOut_ms || TimeOut_ms==0)
   {
      switch (read(fd,pByte,1)) 
      {
         case -1: throw ErrorReadException();
         case 1 : return 1;
      }
   }
   throw TimeoutException();
}

unsigned Serial::ReadStringNoTimeOut(char *String, char FinalChar, unsigned MaxNbBytes)
{
   unsigned NbBytes=0;
   unsigned ret;
   while (NbBytes<MaxNbBytes)
   {
      ret=ReadChar(&String[NbBytes]);
      if (ret==1)
      {
         if (String[NbBytes]==FinalChar)
         {
            String[NbBytes]='\0';
            return NbBytes;
         }
         NbBytes++;
      }
      else 
         throw ErrorReadException(); // ramo inutile, tanto l'eccezione verrebbe lanciata da ReadChar(...)
   }
   throw ErrorBufferFullException();
}

unsigned Serial::ReadString(char *String,char FinalChar,unsigned MaxNbBytes,unsigned TimeOut_ms)
{
   if (TimeOut_ms==0)
      return ReadStringNoTimeOut(String,FinalChar,MaxNbBytes);
   unsigned NbBytes=0;
   unsigned ret;
   TimeOut Timer;
   long int TimeOutParam;
   Timer.InitTimer();

   while (NbBytes<MaxNbBytes)
   {
      TimeOutParam=TimeOut_ms-Timer.ElapsedTime_ms();
      if (TimeOutParam>0)
      {
         ret=ReadChar(&String[NbBytes],TimeOutParam);           
         if (ret==1)
         {
            if (String[NbBytes]==FinalChar)
            {
               String[NbBytes]='\0';
               return NbBytes;
            }
            NbBytes++;
         }
         else 
          throw ErrorReadException();
      }
      if (Timer.ElapsedTime_ms()>TimeOut_ms) 
      {
         String[NbBytes]='\0';
         throw TimeoutException();
      }
   }
   throw ErrorBufferFullException();
}


unsigned Serial::Read (void *Buffer,unsigned MaxNbBytes,unsigned TimeOut_ms)
{
   TimeOut Timer;
   Timer.InitTimer();
   unsigned NbByteRead=0;
   while (Timer.ElapsedTime_ms()<TimeOut_ms || TimeOut_ms==0)
   {
      unsigned char* Ptr=(unsigned char*)Buffer+NbByteRead;
      int Ret=read(fd,(void*)Ptr,MaxNbBytes-NbByteRead);
      if (Ret==-1) 
         throw ErrorReadException();
      if (Ret>0) 
      {
         NbByteRead+=Ret;
         if (NbByteRead>=MaxNbBytes)
            return NbByteRead;
      }
   }
   throw TimeoutException();
}

void Serial::FlushReceiver()
{
   tcflush(fd,TCIFLUSH);
}

int Serial::Peek()
{
   int Nbytes=0;
   ioctl(fd, FIONREAD, &Nbytes);
   return Nbytes;
}

TimeOut::TimeOut()
{}

void TimeOut::InitTimer()
{
   gettimeofday(&PreviousTime, NULL);
}

unsigned long int TimeOut::ElapsedTime_ms()
{
   struct timeval CurrentTime;
   int sec,usec;
   gettimeofday(&CurrentTime, NULL);
   sec=CurrentTime.tv_sec-PreviousTime.tv_sec;
   usec=CurrentTime.tv_usec-PreviousTime.tv_usec;
   if (usec<0)
   {
      usec=1000000-PreviousTime.tv_usec+CurrentTime.tv_usec;
      sec--;
   }
   return sec*1000+usec/1000;
}

#endif

