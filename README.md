OS: Linux or UNIX compliant system (its testet on linux)

Build with qmake, cmake or whatever you prefer

Copy the binary to any location you want

Run with:
	ECGProtocol [--V] var  


Here follows a list of arguments (V) and their inputs (var):
	
	--PORT-DST <port> : Set the destination port
	--PORT-SRC <port> : Set the source port. If listening mode, interface listen on this port for incomming packets
	--MESSAGE "<message>" <lenght>
	--ROLE-CLIENT : The default role.i
	--ROLE-SERVER : Sets the interface in listening mode
	--SET-TIMEOUT <val> : Set the timeout value in milliseconds

Escape with 'ctrl+c' while in listening mode
