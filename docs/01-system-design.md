# High-level architecture

## Multithreading

Different systems of the application can be split into different threads, or be kept on the same thread, depending on the specifications of the computer running the game. Therefore, the implementation of the multithreading system must allow for modular systems. To achieve this, the following should be accounted for:
 - each system that can be split into a different thread must consist of:
   - a base API containing all the low-level functionality of the system (all functions / classes etc. provided by its API)
   - a "Command" function type that represents a function using the base API
   - an interface defining an "Execute" method taking as parameter a "Command" that must be executed
   - an implementation of the interface that immediately executes the given command
   - an implementation of the interface that schedules the execution of the given command on another thread
   - an implementation of the separate thread main loop, command queue etc.
 - each system that requires the use of another system can then do the following:
   - call the "Execute" method of the system object of the other system with the command needed to execute
   - (optional) wait for synchronization

