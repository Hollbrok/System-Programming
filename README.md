# My Projects for Lunev course

- [X] Warm-up tasks
    - [X] __Show numbers__ from _1_ to _X_, where X is argument of the cmd line.
    - [X] __Copy__ _f1_ data to _f2_, where is f1 and f2 are argument of the cmd line.
    - [X] Learn about __strace__, his syntax and output.
    - [X] __Forker__ (need to add error processing after close)
    - [X] __Threader__
    - [X] __Executor__
- [X] __FIFO__
- [X] __Message's queue__
- [X] __Shared memory__
- [X] __Signals__
- [X] __Pipes (send file between all children via parent)__s

# How to build

U should to write the next commands
*  make
*  ./progName [argument/arguments] or (`if u need to run more than 1 prog`) u should do this in 2 independent consoles

# Progress


| __1st sem__   |       __Test programms__      |   __FIFO__            | __Message Queue__ |   __Shared Memory__    |  __Signals__          |
|:------------- |:---------------:              |:--------------------:      |:-------------:    |:-------------:         |   :-------------:     |
|   status      | __all passed successfully__   | `question remains`    |  `+`              |    __wrote__  |                       |

## TODO

### __SHM + SEMs__
    [ ] atexit function to rm ipc's
    [ ] or add 1 more sem to indicate that we remove all successfully__ (val 1 after init and 0 after rm). And after
    not EXCL create check this!
    [ ] critical sections
