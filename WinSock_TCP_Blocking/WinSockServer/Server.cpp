#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "SelectCitanje.h"
#include "SelectUpis.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1025"
#define SERVER_SLEEP_TIME 50
#define DEFAULT_ADDRESS "127.0.0.1"

bool InitializeWindowsSockets();

int RacunajDeterminantu(int** matrica, int brojRedova, int brojKolona,short Port,HANDLE* tredovi);

int** napraviManjuMatricu(int** staraMatrica, int stariRed, int kolonaIzbacivanja);

void OtvoriDrugiProces(short port,int i);


int** Deserijalizacija(char*, int dimenzije);

int OdradiMain(SOCKET* listenSocket);

DWORD WINAPI Paralelno(LPVOID IpParam);

CRITICAL_SECTION cs;

typedef struct ListMatrica
{
	SOCKET acceptedSocket ;
	int dimenzija;
	char* niz;
	int* determinanta;
	int i;
	int VrednostIzReda;
	
} LM;


int  main(void) 
{
	InitializeCriticalSection(&cs);
	

	int* RezultatDeterminante = (int*)calloc(1,sizeof(int));

	short Port = atoi(DEFAULT_PORT);
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data
    char* recvbuf = (char*)calloc(1,DEFAULT_BUFLEN);
	char* recvbuf2 = (char*)calloc(1,4 * sizeof(char));
    
    if(InitializeWindowsSockets() == false)
    {
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
    }
    
    // Prepare address information structures
    addrinfo *resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if ( iResult != 0 )
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
                          SOCK_STREAM,  // stream socket
                          IPPROTO_TCP); // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind( listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

	printf("Server initialized, waiting for clients.\n");
	free(RezultatDeterminante);

	 int k = OdradiMain(&listenSocket);

    // cleanup
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}


bool InitializeWindowsSockets()
{
    WSADATA wsaData;
	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
	return true;
}


int RacunajDeterminantu(int** matrica, int brojRedova, int brojKolona, short Port,HANDLE* Tredovi) // komunikacija sa onim ispod njega
{
	
	

	int *determinanta = (int*)calloc(1,sizeof(int));

	if (brojRedova != brojKolona)
		return -1;

	if (brojRedova != 2)
	{


		char* PORT = (char*)calloc(1, 10);
		itoa(Port, PORT, 10);
		// Socket used for listening for new clients 
		SOCKET listenSocket = INVALID_SOCKET;
		// Socket used for communication with client
		SOCKET acceptedSocket = INVALID_SOCKET;
		// variable used to store function return value
		int iResult;
		// Buffer used for storing incoming data
		char* recvbuf = (char*)calloc(1, DEFAULT_BUFLEN);
		int bajtoviZaPrimanje = 0;
		

		char* firstMessage = (char*)calloc(1, 2);

		if (InitializeWindowsSockets() == false)
		{
			// we won't log anything since it will be logged
			// by InitializeWindowsSockets() function
			return 1;
		}

		// Prepare address information structures
		addrinfo *resultingAddress = NULL;
		addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;       // IPv4 address
		hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
		hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
		hints.ai_flags = AI_PASSIVE;     // 

										 // Resolve the server address and port
		iResult = getaddrinfo(NULL, PORT, &hints, &resultingAddress);
		if (iResult != 0)
		{
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return 1;
		}

		free(PORT);

		// Create a SOCKET for connecting to server
		listenSocket = socket(AF_INET,      // IPv4 address famly
			SOCK_STREAM,  // stream socket
			IPPROTO_TCP); // TCP

		if (listenSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(resultingAddress);
			WSACleanup();
			return 1;
		}

		// Setup the TCP listening socket - bind port number and local address 
		// to socket
		iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(resultingAddress);
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		// Since we don't need resultingAddress any more, free it
		freeaddrinfo(resultingAddress);

		unsigned long int nonBlockingMode = 1;
		iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlockingMode);

		if (iResult == SOCKET_ERROR)
		{
			printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
			return 1;
		}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
		


		// Set listenSocket in listening mode
		iResult = listen(listenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("Server initialized, waiting for clients.\n");


		for (int i = 0; i < brojRedova; i++)
		{
			    
			int PortNumber = 63000 / brojRedova;
				OtvoriDrugiProces(Port,i*PortNumber);
				
				

			//ovde formirati novu matricu dimenzija manjih za 1
			int** novaMatrica = napraviManjuMatricu(matrica, brojRedova, i);

			int pom = 2;
			char* niz = (char*)calloc(1, sizeof(char) * 2 + sizeof(int)*(brojKolona - 1)*(brojRedova - 1));

			*(char*)niz = brojRedova - 1;
			*(char*)(niz + 1) = brojKolona - 1;

			for (int i = 0; i < brojRedova - 1; i++)
			{
				for (int j = 0; j < brojKolona - 1; j++)
				{
					*(int*)(niz + pom) = novaMatrica[i][j];
					pom += 4;

				}
			}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			//otvara socket za izvrsnu jedinicu

			bool provera = true; //za izlazenje iz while-a

			do
			{
				// Wait for clients and accept client connections.
				// Returning value is acceptedSocket used for further
				// Client<->Server communication. This version of
				// server will handle only one client.


				iResult = SelectCitanje(&listenSocket);

				// lets check if there was an error during select
				if (iResult == SOCKET_ERROR)
				{
					fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
					continue;
				}

				// now, lets check if there are any sockets ready
				if (iResult == 0)
				{
					// there are no ready sockets, sleep for a while and check again
					Sleep(SERVER_SLEEP_TIME);
					continue;
				}


				acceptedSocket = accept(listenSocket, NULL, NULL);
				

				if (acceptedSocket == INVALID_SOCKET)
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
					closesocket(listenSocket);
					WSACleanup();
					return 1;
				}

				// Set socket to nonblocking mode
				unsigned long int nonBlockingMode = 1;
				iResult = ioctlsocket(acceptedSocket, FIONBIO, &nonBlockingMode);

				if (iResult == SOCKET_ERROR)
				{
					printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
					return 1;
				}

				provera = false;
//------------------------------------------------------------------------------------------------------------------------------------------------

				LM ALL;
				ALL.acceptedSocket = acceptedSocket;
				//ALL.cs = cs;
				ALL.dimenzija = brojRedova;
				ALL.i = i;
				ALL.niz = niz;
				ALL.VrednostIzReda = matrica[0][i];
				ALL.determinanta = determinanta;
				DWORD id = i;
					
			   Tredovi[i] = CreateThread(NULL, 0, &Paralelno, &ALL, 0, &id);
					

			} while (provera);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		/*	if (i == 0 || (i % 2) == 0)
				determinanta += matrica[0][i] * *(int*)recvbuf;
			else
				determinanta -= matrica[0][i] * *(int*)recvbuf;*/

			

			//closesocket(acceptedSocket);
			memset(recvbuf,0,bajtoviZaPrimanje);
		}

		bool radTraje = true;
		bool* ugaseni = (bool*)calloc(1, sizeof(bool)*brojRedova);
		int brojZavrsenih = 0;

		while (radTraje)
		{
			DWORD exitCode;
			for (int i = 0; i < brojRedova; i++)
			{
				GetExitCodeThread(Tredovi[i], &exitCode);
				if (exitCode != STILL_ACTIVE && ugaseni[i] == false)
				{
					CloseHandle(Tredovi[i]);
					ugaseni[i] = true;
					brojZavrsenih++;
				}

			}
			if (brojZavrsenih >= brojRedova)
				radTraje = false;
			
		}
		free(ugaseni);
		
		closesocket(listenSocket);
		free(recvbuf);
		
	}

	else
		*determinanta = matrica[0][0] * matrica[1][1] - matrica[0][1] * matrica[1][0];

		
	return *determinanta;
}

int** napraviManjuMatricu(int** staraMatrica,int stariRed,int kolonaIzbacivanja)
{


	int i, j, red = stariRed - 1, kolona = stariRed - 1;

	int** matrica = (int **)calloc(1, red * sizeof(int*));

	for (i = 0; i < red; i++)
	{
		matrica[i] = (int *)calloc(1, kolona * sizeof(int));
	}

	int m = 0, n = 0;

	for (i = 1; i <= red; i++)
	{
		for (j = 0; j <= kolona; j++)
		{
			if (j != kolonaIzbacivanja)
			{
				matrica[m][n] = staraMatrica[i][j];
				n++;
			}
		}
		n = 0;
		m++;
	}
	return matrica;
}


void OtvoriDrugiProces(short Port,int i)
{
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	DWORD dwProcessId = 0;
	DWORD dwThreadId = 0;

	char* zaSlanje = (char*)calloc(1, 2 * sizeof(char));
	*(short*)zaSlanje = Port;
	*((short*)zaSlanje + 1) = Port + i + 1;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	BOOL bCreateProcess = NULL;

	bCreateProcess = CreateProcessA("..\\Debug\\IzvrsnaJedinica_clientServer.exe",
		zaSlanje, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	//drugi - parametre koje saljemo
}

int** Deserijalizacija(char* recvbuf, int dimenzije)
{

	int** matrica = (int **)calloc(1, dimenzije * sizeof(int));

	for (int i = 0; i < dimenzije; i++)
	{
		matrica[i] = (int *)calloc(1, dimenzije * sizeof(int));
	}


	int counter = 0;

	for (int i = 0; i < dimenzije; i++)
	{
		for (int j = 0; j < dimenzije; j++)
		{

			matrica[i][j] = *(int*)(recvbuf + 2 + counter * sizeof(int));
			counter++;
		}
	}

	printf("Matrica = [  ");
	for (int i = 0; i < dimenzije; i++)
	{
		for (int j = 0; j < dimenzije; j++)
		{
			printf("%d ", matrica[i][j]);
		}
		if (i + 1 < dimenzije)
			printf("\n\t");

	}
	printf("  ]");

	return matrica;
}

int OdradiMain(SOCKET* listenSocket)
{
	//SOCKET listenSocket = *LS;
	short Port = atoi(DEFAULT_PORT);
	// Socket used for communication with client
	SOCKET acceptedSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	// Buffer used for storing incoming data
	char* recvbuf = (char*)calloc(1, DEFAULT_BUFLEN);
	char* recvbuf2 = (char*)calloc(1, 4 * sizeof(char));

	do
	{
		// Wait for clients and accept client connections.
		// Returning value is acceptedSocket used for further
		// Client<->Server communication. This version of
		// server will handle only one client.



		iResult = SelectCitanje(listenSocket);

		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}

		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(SERVER_SLEEP_TIME);
			continue;
		}


		acceptedSocket = accept(*listenSocket, NULL, NULL);

		if (acceptedSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(*listenSocket);
			WSACleanup();
			return 1;
		}

		// Set socket to nonblocking mode
		unsigned long int nonBlockingMode = 1;
		iResult = ioctlsocket(acceptedSocket, FIONBIO, &nonBlockingMode);

		if (iResult == SOCKET_ERROR)
		{
			printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
			return 1;
		}

		do
		{


			iResult = SelectCitanje(&acceptedSocket); //radi onaj select/


			// lets check if there was an error during select
			if (iResult == SOCKET_ERROR)
			{
				fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
				continue;
			}

			// now, lets check if there are any sockets ready
			if (iResult == 0)
			{
				// there are no ready sockets, sleep for a while and check again
				Sleep(SERVER_SLEEP_TIME);
				continue;
			}


			// Receive data until the client shuts down the connection
			iResult = recv(acceptedSocket, recvbuf2, 4, 0);

			if (iResult > 0)
			{


				int velicinaPoruke = *(int*)recvbuf2;

				Recv1(&acceptedSocket, recvbuf, velicinaPoruke);


				int i, j, pom = 2;
				char red = *recvbuf;
				char kolona = *(recvbuf + 1);

				HANDLE* tredovi = (HANDLE*)calloc(1,sizeof(HANDLE)*red);
				

				int** matrica = Deserijalizacija(recvbuf, red);

				int determinanta = RacunajDeterminantu(matrica, red, kolona, Port + 1,tredovi);

				printf("\n\nDeterminanta matrice je:\n D = %d", determinanta);

//-------------------------------------------------------------------------------------------------------------------------------------------------
				// Send an prepared message with null terminator included
				char* messageToSend = (char*)calloc(1, sizeof(int) + 1);
				int* det_zaSlanje = &determinanta;
				for (int i = 0; i < sizeof(int); i++)
				{
					*(messageToSend + i) = *((char*)det_zaSlanje + i); // pretvaranje u format za slanje (deser.)
				}

				char* brojBajtova = (char*)calloc(1, sizeof(char));
				*brojBajtova = (strlen(messageToSend) + 1);

				Send1(&acceptedSocket, brojBajtova, sizeof(char)); //koliko bajtova saljemo

				Send1(&acceptedSocket, messageToSend, (strlen(messageToSend) + 1)); //poruka koju saljemo client-u

				free(brojBajtova);
				free(messageToSend);

				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket);
					WSACleanup();
					return 1;
				}


			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				printf("Connection with client closed.\n");
				closesocket(acceptedSocket);
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
			}
		} while (iResult > 0);

		// here is where server shutdown loguc could be placed

	} while (1);

	// shutdown the connection since we're done
	iResult = shutdown(acceptedSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
		WSACleanup();
		return 1;
	}
	closesocket(acceptedSocket);
	WSACleanup();
	return 0;
}

DWORD WINAPI Paralelno(LPVOID IpParam)
{
	LM ALL = *(LM*)IpParam;
	int iResult;
	int bajtoviZaPrimanje = 0;
	char* firstMessage = (char*)calloc(1, 2);
	char* recvbuf = (char*)calloc(1, DEFAULT_BUFLEN);
	//salje broj bajta i matricu matricu
	do {


		iResult = SelectPisanje(&ALL.acceptedSocket);


		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}

		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(50);
			continue;
		}

		// Send an prepared message with null terminator included
		int brojBajtova = sizeof(int)*(ALL.dimenzija - 1)*(ALL.dimenzija - 1) + sizeof(char) * 2;
		char prvaPoruka = brojBajtova;

		iResult = send(ALL.acceptedSocket, &prvaPoruka, 1, 0); //salje mu broj bajta koji se salje



		Send1(&ALL.acceptedSocket, ALL.niz, brojBajtova);

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ALL.acceptedSocket);
			WSACleanup();
			//return 1;
		}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		//ocekuje odgovor - recv 
		Recv1(&ALL.acceptedSocket, firstMessage, sizeof(char));
		bajtoviZaPrimanje = *(char*)firstMessage;


		Recv1(&ALL.acceptedSocket, recvbuf, bajtoviZaPrimanje);

	} while (iResult == 0);

	EnterCriticalSection(&cs);
	if (ALL.i == 0 || (ALL.i % 2) == 0)
		*ALL.determinanta += ALL.VrednostIzReda * *(int*)recvbuf;
	else
		*ALL.determinanta -= ALL.VrednostIzReda * *(int*)recvbuf;
	LeaveCriticalSection(&cs);
	

	return 0;
}
