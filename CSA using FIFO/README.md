#  Client-Server Application Using FIFOs

## Server

* Client has one FIFO to read const-size blocks of data from all clients



## Client
* Every client has its own FIFO to get response from server
* This FIFO has specific name generation algorithm
