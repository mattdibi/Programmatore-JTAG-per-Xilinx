interface: serial.o svftoserial.o main.o
	g++ -o interface main.o serial.o svftoserial.o

serial.o: serial.cpp serial.hpp
	g++ -c serial.cpp

svftoserial.o: svftoserial.cpp svftoserial.hpp
	g++ -c svftoserial.cpp

main.o: main.cpp
	g++ -c main.cpp

clean:
	rm -rf *.o interface
	
