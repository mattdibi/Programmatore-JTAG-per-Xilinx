# Progetto programmatore JTAG per scheda Xilinx
L'obiettivo del progetto è quello di sostituire il programmatore USB della scheda Xilinx Spartan 6 con una board Arduino-like e un software da noi realizzato per effettuare, tramite protocollo JTAG, la programmazione della scheda FPGA.

### Gruppo di lavoro:
Questo progetto è stato realizzato per il corso di Calcolatori Elettronici e Sistemi Operativi nel corso di Laurea Magistrale in Ingegneria Elettronica offerto dall'Università degli Studi di Udine. Componenti del gruppo:
* Mattia Marson
* Manuel Guglielmini
* Mattia Dal Ben

### Documentazione:
Per la documentazione completa consultare il [pdf contenuto nella repository](https://github.com/MattiaDalBen/Programmatore-JTAG-per-Xilinx/blob/master/Relazione%20Progetto%20Programmatore%20JTAG.pdf). Qui di seguito è riportata una guida rapida.
Avvertenza: come riportato nella relazione questo codice è stato testato solamente sulle schede Spartan 6. Non c'è alcuna garanzia che il codice funzioni per altre schede Xilinx.

### Compilazione:
Dopo aver collegato la board Arduino-like e acceso la scheda Xilinx

```sh
$ cd ProgettoProgrammatoreJTAG
$ make
$ ./interface
```

### Modalità di esecuzione:
Modalità di esecuzione standard:
```sh
$ ./interface
```

Modalità manuale
```sh
$ ./interface -m
```

Modalità con output dettagliato
```sh
$ ./interface -v
```

Modalità con output su file
```sh
$ ./interface -f [nomeFile]
```

Help
```sh
$ ./interface -h
```

