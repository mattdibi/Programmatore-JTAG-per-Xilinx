#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <termios.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <exception>

using namespace std;

//enum SerialError_t {ERROR_BUFFER_FULL, ERROR_READ, ERROR_TIMEOUT, ERROR_WRITE, ERROR_SPEED, ERROR_PORT_PARAMETERS, ERROR_OPENING, DEVICE_NOT_FOUND, SERIAL_SUCCESS};

class Serial
{
  public:
   Serial();
   ~Serial();
   void Open(const char *Device, const unsigned Bauds);
   void Close();
   void WriteChar(char);
   unsigned ReadChar(char *pByte,const unsigned TimeOut_ms=0);
   void WriteString (const char *String);
   unsigned ReadString(char *String, char FinalChar, unsigned MaxNbBytes, const unsigned TimeOut_ms=0);

   void Write(const void *Buffer, const unsigned int NbBytes);
   unsigned Read(void *Buffer,unsigned int MaxNbBytes,const unsigned TimeOut_ms=0);

   void FlushReceiver();
   // Return the number of bytes in the received buffer
   int Peek();
  private:
   unsigned ReadStringNoTimeOut(char *String,char FinalChar,unsigned MaxNbBytes);
   int fd;
};

class TimeOut
{
  public:
   TimeOut();
   void InitTimer();
   unsigned long int ElapsedTime_ms();
  private:    
   struct timeval PreviousTime;
};

class ErrorBufferFullException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: buffer full";
   }
};
class ErrorReadException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error reading";
   }
};
class TimeoutException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Timeout expired";
   }
};
class ErrorWriteException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error writing";
   }
};

class ErrorSpeedException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: baud rate not supported";
   }
};
class ErrorParametersException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: port parameters not valid";
   }
};
class ErrorOpeningException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error opening port";
   }
};
class DeviceNotFoundException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: device not found";
   }
};

#endif

