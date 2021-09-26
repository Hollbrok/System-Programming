#  Client-Server Application Using FIFOs

## Server

* Client has one FIFO to read const-size blocks of data from all clients


## Client
* Every client has its own FIFO to get response from server
* This FIFO has specific name generation algorithm

## TODO

- [X] if server is already created -- unlink it and create new
- [ ] if server is sleeping for a __X__ time then I can delete it.
- [ ] Create new file for general stuff between server and client.
