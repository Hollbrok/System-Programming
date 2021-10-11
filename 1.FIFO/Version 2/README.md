#  1'st project in Lunev's course

## How to compile and start

* make file=<file_name>
i.e.
    - [X] make file=server
    - [X] make file=client

To start you've two options:

1) Use 2 consoles and do this:

* ./server * IN 1st CONSOLE *

* ./client [file_name] * IN 2nd CONSOLE *

2) Or use this method:

* ./server &

And then you can be client:

* ./client [file_name]

Note: to terminate server you must call __./server__ again and generate Interrupt or Terminate.

TODO: create terminater of server

## Server

* Server has 2 FIFO's to read access from clients and to read data (file bytes) from client


## Client

* Every client has its own FIFO to get response(ACCESS) from server. This FIFO has specific name generation algorithm

## TODO
- [X] Full working programm (according to Lunev criterias)
- [X] if server is already created -- unlink it and create new
- [X] Add loging (backup file) (get info at the start of server works and update log file at the end of work). (O_SYNC)
- [X] If server receives signals SIGTERM or SIGINT, it removes the FIFO and  terminates.
- [X] Protection from DOS-attacks
- [ ] Now I can only delete server by using ./server + INT. So I need to add __serdel__ stuff.
- [X] Function separation
