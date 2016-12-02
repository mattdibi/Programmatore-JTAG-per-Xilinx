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
   // Apre la connessione seriale
   void Open(const char *Device, const unsigned Bauds);
   // Chiude la seriale
   void Close();
   // Scrive un carattere sulla seriale, lancia un'eccezione in caso di errore
   void WriteChar(const char Byte);
   // Legge un carattere sulla seriale, lancia un'eccezione in caso di errore
   unsigned ReadChar(char *pByte,const unsigned TimeOut_ms=0);
   // Scrive una stringa sulla seriale, lancia un'eccezione in caso di errore
   void WriteString (const char *String);
   // Legge una stringa sulla seriale, lancia un'eccezione in caso di errore
   unsigned ReadString(char *String, char FinalChar, unsigned MaxNbBytes, const unsigned TimeOut_ms=0);
   // Scrive un buffer di caratteri sulla seriale, lancia un'eccezione in caso di errore
   void Write(const void *Buffer, const unsigned int NbBytes);
   // Legge un buffer di caratteri sulla seriale, lancia un'eccezione in caso di errore
   unsigned Read(void *Buffer,unsigned int MaxNbBytes,const unsigned TimeOut_ms=0);
   // Svuota il buffer di lettura
   void FlushReceiver();
   // Restituisce il numero di caratteri nel buffer di lettura
   int Peek();
  private:
   // Scrive una stringa sulla seriale senza considerare un timeout, lancia un'eccezione in caso di errore
   unsigned ReadStringNoTimeOut(char *String,char FinalChar,unsigned MaxNbBytes);
   int fd;
};

// Classe necessaria per gestire i timeout
class TimeOut
{
  public:
   TimeOut();
   // Inizializza il timer
   void InitTimer();
   // Restituisce il tempo trascorso dall'inizializzazione del timer
   unsigned long int ElapsedTime_ms();
  private:
   struct timeval PreviousTime;
};

// Eccezione: buffer pieno
class ErrorBufferFullException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: buffer full";
   }
};
// Eccezione: errore in lettura
class ErrorReadException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error reading";
   }
};
// Eccezione: timeout
class TimeoutException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Timeout expired";
   }
};
// Eccezione: errore in scrittura
class ErrorWriteException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error writing";
   }
};
// Eccezione: velocità non supportata
class ErrorSpeedException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: baud rate not supported";
   }
};
// Eccezione: parametri di connessione non validi
class ErrorParametersException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: port parameters not valid";
   }
};
// Eccezione: errore in apertura della seriale
class ErrorOpeningException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error opening port";
   }
};
// Eccezione: seriale non trovata
class DeviceNotFoundException : public exception
{
  public:
   virtual const char* what() const throw()
   {
      return "Error: device not found";
   }
};

#endif

