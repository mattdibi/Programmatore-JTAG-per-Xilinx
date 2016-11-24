/*  
	Project:
		JTAG controller for Xilinx Spartan-6 on Arduino Uno32 board
	Group:
		Mattia Dal Ben     (Matricola 102806)
		Mattia Marson      (Matricola 103248)
		Manuel Guglielmini (Matricola  91575)
	
	This project was made for the course: "Calcolatori Elettronici e Sistemi Operativi"
	held in University of Udine by Prof. Mirko Loghi.
*/

// Configuration
#define ACTIVE HIGH     // Level of logic 1 on the JTAG lines
#define QUIET 1        // Set to 1 to remove startup banner
#define XONXOFF 0       // Send software flow control characters?
#define TCKWAIT 1       // ms delay between clock edges
#define VERBOSITY 2 	    // Set to increase verbosity [0 = min, 2 = max]

//#define DEBUG

// Buffer Size
#define BUFSIZE 128

// Pinout
#define LED 13

#define TMS 8
#define TDI 9
#define TDO 36 //Pin 10 broken
#define TCK 11

// Optional hardware flow control
#define CTS 2 // NB: this is assumed active-low


// JTAG pin functions
void tms(boolean value);
boolean tdi(boolean value);
uint8_t tdin(int n, uint8_t bits);
void tck(void);

void set(uint8_t pin, boolean value);
boolean get(uint8_t pin);

// Output encoding
char out1(boolean value);
char out4(uint8_t value);

// Flow control
void xon(void);
void xoff(void);
void waitforinput(void);

// Subfunctions
boolean exit_r(boolean value);
void idcode(void);

#ifdef DEBUG
const char *help =
    "***********************************************************\r\n"
    "Available commands:\r\n"
    "! set TMS=1 then pulse TCK       . pulse TCK then set TDI=0\r\n"
    "* set TMS=0 then pulse TCK       , pulse TCK then set TDI=1\r\n"
	": set TMS=1, pulse TCK then set TDI=0\r\n"
	"; set TMS=1, pulse TCK then set TDI=1\r\n"
    "[0-f] 4-bit shorthand (TDI)\r\n"
    "? print this banner              X Get device ID code \r\n"
    "\r\n"
    "NOTE: Set your serial monitor to <Newline mode>\r\n"
    "***********************************************************\r\n";
#else
const char *help =
    "***********************************************************\t"
    "Available commands:\t"
    "! set TMS=1 then pulse TCK       . pulse TCK then set TDI=0\t"
    "* set TMS=0 then pulse TCK       , pulse TCK then set TDI=1\t"
	": set TMS=1, pulse TCK then set TDI=0\t"
	"; set TMS=1, pulse TCK then set TDI=1\t"
    "[0-f] 4-bit shorthand (TDI)\t"
    "? print this banner              X Get device ID code \t"
    "\t"
    "NOTE: Set your serial monitor to <Newline mode>\t"
    "***********************************************************\t";
#endif
void banner(void)
{
    #ifdef DEBUG
    Serial.println();
    Serial.print(help);
    Serial.print("\r\n\n");
    #else
    Serial.print("\t");
    Serial.print(help);
    //Serial.println();
    #endif
    Serial.flush();
}

void setup()
{
    pinMode(LED, OUTPUT);

    pinMode(TMS, OUTPUT);
    pinMode(TDI, OUTPUT);
    pinMode(TDO, INPUT);
    pinMode(TCK, OUTPUT);

    pinMode(CTS, OUTPUT);

    Serial.begin(9600);

#if QUIET == 0
	// Startup banner
    banner();
#endif
}

char buf[BUFSIZE];

void loop()
{
    int i, n=0;
    char c='\0';

    // Read input until buffer full or end of line
    while(n<BUFSIZE && c!='\n') 
	{
        waitforinput();
        c = Serial.read();
        if(c == '\r')
            c = '\n';   // Normalize input

        if(c == '\b' && n > 0)
            n--;
        else
            buf[n++] = c;
    }
    
#if VERBOSITY == 1 || VERBOSITY == 2
	// Write input on serial port
   #ifdef DEBUG
	Serial.println("Input :");
    #else
    Serial.print("Input :\t");
    #endif
    for(i=0; i<n; i++) 
	{
        char c = buf[i];
        if(c == '\n')
            Serial.print('\t');
        else
            Serial.print(c);
    }
    Serial.flush();
#endif

	// Translates every character with corresponding command and meanwhile 
	// it substitute the content of the buffer with the output (LOGGING)
    for(i=0; i<n; i++) 
	{
        char c = buf[i];
        switch(c) 
		{
        case '!':   tms(1); c=' ';      break;
        case '*':   tms(0); c=' ';      break;
		/************************************/
        case '.':   c=out1(tdi(0));     break;
        case ',':   c=out1(tdi(1));     break;
		/************************************/
		case ':':	c=out1(exit_r(0));  break;
		case ';':	c=out1(exit_r(1));  break;
		/************************************/
        case 'X':   idcode(); c='\r'; break;
        case '?':   banner(); c='\r';   break;
        default:
            if(isdigit(c) || (c>='a' && c<='f')) 
			{
                // Hex notation
                uint8_t v;
                v = isdigit(c)? (c - '0') : (10 + c - 'a');
                c = out4(tdin(4, v));
            } 
			else if(isspace(c)) 
			{
                // Pass to keep visual alignment
				// between input and output
            } 
			else 
			{
                c = '-';
            }
        }
        buf[i] = c;
    }

    // Write output on serial port
#if VERBOSITY == 1 || VERBOSITY == 2
   #ifdef DEBUG
	Serial.println("Output:");
   #else
  Serial.print("Output:\t");
  #endif
#endif
    for(i=0; i<n; i++) 
	{
        char c = buf[i];
        if(c == '\n')
            Serial.print('\t');
        else
            Serial.print(c);
    }
    Serial.println();
    Serial.flush();
}



// Subfunction: return ID Code
void idcode(void)
{
    uint8_t n7, n6, n5, n4, n3, n2, lsb;
	
	// Acknowlegding commands
#if VERBOSITY == 1 || VERBOSITY == 2
  #ifdef DEBUG
    Serial.println("ID Code call.");
  #else
    Serial.print("ID Code call.\t");
  #endif
#endif

    // Switching to Test-Logic-Reset state
    tms(1); tms(1); tms(1); tms(1); tms(1);

    // Switching to Capture-DR state
    tms(0); tms(1); tms(0);
	
	// Reading ID Code
    lsb = tdi(1);
	
    if(lsb == 0) 
	{
        Serial.println("0         [device has no idcode]");
    } 
	else 
	{
        lsb |= tdin(7, 0xff) << 1;
        n2 = tdin(4, 0xf);

        if(lsb == 0xff && n2 == 0xf) 
		{
            // No such manufacturer, must be our own 1s
#ifdef DEBUG
            Serial.println("No more data to collect.");
#else
Serial.print("No more data to collect.\t");
#endif
        }

        n3 = tdin(4, 0xf);
        n4 = tdin(4, 0xf);
        n5 = tdin(4, 0xf);
        n6 = tdin(4, 0xf);
        n7 = tdin(4, 0xf);

        // Printing the ID Code
        Serial.print("ID Code: ");
        Serial.print(out4(n7));
        Serial.print(out4(n6));
        Serial.print(out4(n5));
        Serial.print(out4(n4));
        Serial.print(out4(n3));
        Serial.print(out4(n2));
        Serial.print(out4(lsb >> 4));
        Serial.print(out4(lsb & 0xf));
        #ifdef DEBUG
        Serial.println();
        #else
        Serial.print('\t');
        #endif
        }

    // Switch back to Run-Test/Idle state
    tms(1); tms(1); tms(0);
    
#if VERBOSITY == 1 || VERBOSITY == 2
#ifdef DEBUG
    Serial.println("End ID code.");
#else
Serial.print("End ID code.\t");
#endif
#endif

}

// Set TMS to 1 , cycle TCK , set TDI to value, sample TDO
// this function is needed because the IEEE standard says that the last bit
// must be read while exiting from the SHIFT-XR state
boolean exit_r(boolean value)
{
    set(TMS, 1);
    set(TDI, value);
    tck();
    return get(TDO);
}


// Set TMS to value, cycle TCK
void tms(boolean value)
{
#if VERBOSITY == 2
	// Printing every tms call to debug
	// Verbosity level is over 9000!
#ifdef DEBUG
    if( value )
      Serial.println("tms(1)");
    else
      Serial.println("tms(0)");
#else
if( value )
      Serial.print("tms(1)\t");
    else
      Serial.print("tms(0)\t");
      #endif
    Serial.flush();
#endif
    set(TMS, value);
    tck();
}

// Cycle TCK, set TDI to value, sample TDO
boolean tdi(boolean value)
{
    set(TDI, value);
    tck();
    return get(TDO);
}

// Multi-bit version of tdi()
uint8_t tdin(int n, uint8_t bits)
{
    uint8_t tmp=0, res=0;
    int i;

    // Shift bits and push into tmp lifo-order
    for(i=0; i<n; i++) 
	{
        tmp = tmp<<1 | tdi(bits & 1);
		// It transmits through tdi() only the last bit inside vector bits[]. But tdi() also
		// RETURNS the output inside tmp.
		bits >>= 1;
		// Shift right the content of bits[] so that at the next loop we transmit by tdi()
		// the next value inside bits[]
    }

    // Reverse bit order tmp->res, so we must reorder output
    for(i=0; i<n; i++) 
	{
        res = res<<1 | tmp&1;
        tmp >>= 1;
    }

    return res;
}

// Pulse CLK function
void tck(void)
{
    set(TCK, 1);
    delay(TCKWAIT);
    set(TCK, 0);
    delay(TCKWAIT);
}

void set(uint8_t pin, boolean value)
{
    digitalWrite(pin, value? ACTIVE : !ACTIVE);
	// If value == 1 => digitalWrite(pin,  ACTIVE)
	// If value == 0 => digitalWrite(pin, !ACTIVE)
	// It's needed to select active high or active low
}

boolean get(uint8_t pin)
{
    return (digitalRead(pin) == ACTIVE);
}

char out1(boolean value)
{
	// If value == 1 => ","
	// If value == 0 => "."
    return (value? ',' : '.');
}

char out4(uint8_t value)
{
	// Hex format
    if(value<10)    
        return ('0' + value);
    else
        return ('a' + value - 10);
}

void printbits(int n, uint8_t bits)
{
    boolean b;

    for(n--; n>=0; n--) 
	{
        b = (bits >> n) & 1;
        Serial.print(b? '1' : '0');
    }
}

void xon(void)
{
#if XONXOFF == 1
    Serial.print('\x11');           // DC1
#endif
    digitalWrite(CTS, LOW);         // Clear to send
    digitalWrite(LED, HIGH);
}

void xoff(void)
{
#if XONXOFF == 1
    Serial.print('\x13');           // DC3
#endif
    digitalWrite(CTS, HIGH);        // Not clear to send
    digitalWrite(LED, LOW);
}

void waitforinput(void)
{
    if(!Serial.available()) 
	{
        //xon();
        while(!Serial.available()) {}
        //xoff();
    }
}
