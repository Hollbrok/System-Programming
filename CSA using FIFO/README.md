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
- [X] Add loging (backup file) (get info at the start of server works and update log file at the end of work). (O_SYNC)
- [X] If server receives signals SIGTERM or SIGINT, it removes the FIFO and  terminates.
- [ ] Protection from DOS-attacks
- [ ] Now I can only delete server by using ./server + INT. So I need to add __serdel__ stuff.
