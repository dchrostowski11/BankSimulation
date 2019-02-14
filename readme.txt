Client-Side
-----------
In the main function we try and connect to the server if we are unable to we will try every 3 seconds to connect to the server. After connecting to the server we will then create 2 threads, one to handle the input from user and another to receive messages from the server. The thread that handles input from users will also be checking for invalid inputs before sending to the server. One of the other functions for this thread is to check for the input quit so it can exit the function.

Server-Side
-----------
The server creates a socket through the socket() call, binds it with bind(), listens for connections with listen(), and ultimately accepts connections from multiple clients with accept().  For every new client that connects, a new thread is created by the clientHandler() function.  Within the clientHandler(), the server waits for input from the client and can create accounts and once created, serve them with the functions:
- end		(ends account service)
- deposit	(allows client to deposit money into account)
- withdraw	(allows client to withdraw money from account)
- query		(allows client to see how much money is in the current account)

Once service for an account is ended, a client has the option to create a new account, start a new service, or quit to program.

All accounts and thread information is stored within linked lists.

Thread-Synchronization
----------------------

All threads were synchronized using mutexes.  For each individual service function, a mutex was locked and unlocked whenever accessing critical data that only one user could access at a time.  A semaphore was used to produce an output every 15 seconds of all the accounts in the system.

Running the Program
-------------------

The program can be run with make, which compiles both server and client files.
To run the server executable, run:

	./bankingServer [portnumber]
	
To run the client executable, run:
	
	./bankingClient [machinenameserverison].cs.rutgers.edu [portnumber]
	
	
