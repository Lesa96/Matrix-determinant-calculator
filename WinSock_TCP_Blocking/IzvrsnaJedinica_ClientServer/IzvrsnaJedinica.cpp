#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "SelectUpis.h"
#include "SelectCitanje.h"

#define DEFAULT_BUFLEN 512
#define SERVER_SLEEP_TIME 50
#define DEFAULT_ADDRESS "127.0.0.1"


// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();

void OtvoriDrugiProces(short Port,int i);

//za client deo---------------------------------------------------------
int** NapraviMatricu(int a, int b,int kontrola);
void IspisiMatricu(int** M);
//-----------------------------------------------------------------------

//za server-----------------------------------------------------------------------
			int RacunajDeterminantu(int** matrica, int brojRedova, int brojKolona,short Port,HANDLE*);
			int** napraviManjuMatricu(int** staraMatrica, int stariRed, int kolonaIzbacivanja);
//---------------------------------------------------------------------------------------------
		

DWORD WINAPI Paralelno(LPVOID IpParam);

CRITICAL_SECTION cs;

typedef struct ListMatrica
{
	SOCKET acceptedSocket;
	int dimenzija;
	char* niz;
	int* determinanta;
	int i;
	int VrednostIzReda;

} LM;

int __cdecl main(int argc, char **argv) 
{
	InitializeCriticalSection(&cs);
	

	char* Port = argv[0];
	short PORT = *(short*)Port;

	char* SvojPort = (char*)calloc(1, 10);
	*(short*)SvojPort = *((short*)argv[0] + 1);
	short SVOJPORT = *(short*)SvojPort;
	//char* ppp = (char*)calloc(1,20);
	itoa(PORT, Port, 10);

	
    
	
	printf("%hd\n", PORT);
	printf("%hd\n", SVOJPORT);
	
	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	

	char* firstMessage = (char*)malloc(sizeof(char));

	char* recvbuf = (char*)calloc(1, DEFAULT_BUFLEN);

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	
	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	
	

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
	serverAddress.sin_port = htons(PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server: %d.\n", GetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}
	
	

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}
	

	do
	{


		iResult = SelectCitanje(&connectSocket); //radi onaj select/


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
		iResult = recv(connectSocket, firstMessage, 1, 0);

		if (iResult > 0)
		{


			int velicinaPoruke = firstMessage[0];
			Recv1(&connectSocket, recvbuf, velicinaPoruke);


			int i, j, pom = 2;
			char red = *recvbuf;
			char kolona = *(recvbuf + 1);

			int** matrica = (int **)calloc(1, red * sizeof(int));

			for (i = 0; i < red; i++)
			{
				matrica[i] = (int *)calloc(1, kolona * sizeof(int));
			}


			int counter = 0;

			for (i = 0; i < red; i++)
			{
				for (j = 0; j < kolona; j++)
				{

					matrica[i][j] = *(int*)(recvbuf + 2 + counter * sizeof(int));
					counter++;
				}
			}

			printf("Matrica = [  ");
			for (int i = 0; i < red; i++)
			{
				for (int j = 0; j < kolona; j++)
				{
					printf("%d ", matrica[i][j]);
				}
				if (i + 1 < red)
					printf("\n\t");

			}
			printf("  ]");

			HANDLE* tredovi = (HANDLE*)calloc(1, sizeof(HANDLE)*red);

			int determinanta = RacunajDeterminantu(matrica, red, kolona, SVOJPORT,tredovi);
			printf("\nDet je = %d\n", determinanta);
			

			// Send an prepared message with null terminator included
			char* messageToSend = (char*)calloc(1, sizeof(int) + 1);
			int* det_zaSlanje = &determinanta;
			for (int i = 0; i < sizeof(int); i++)
			{
				*(messageToSend + i) = *((char*)det_zaSlanje + i);
			}



			char* brojBajtova = (char*)calloc(1, sizeof(char));
			*brojBajtova = (strlen(messageToSend) + 1);

			Send1(&connectSocket, brojBajtova, sizeof(char)); //koliko bajtova saljemo
			Send1(&connectSocket, messageToSend, (strlen(messageToSend)+1)); //poruka koju saljemo client-u

			free(brojBajtova);
			free(messageToSend);




			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}



		}

		} while (iResult == 0);




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





int RacunajDeterminantu(int** matrica, int brojRedova, int brojKolona,short Port,HANDLE* Tredovi) // komunikacija sa onim ispod njega
{
	int *determinanta = (int*)calloc(1, sizeof(int));



	if (brojRedova != brojKolona)
		return -1;

	if (brojRedova != 2)
	{
		//otvara socket za izvrsnu jedinicu

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
			//system("pause");
			
			iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
			if (iResult == SOCKET_ERROR)
			{
				printf("bind failed with error: %d\n", WSAGetLastError());
				system("pause");
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


			
		for (int i = 0; i < brojRedova; i++)
		{
			
			//otvara novi proces
			OtvoriDrugiProces(Port,i + 1);
			Sleep(4000);
			

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
			

			
			bool provera = true;

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
//				//salje broj bajta i matricu matricu


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

			


			/*if (i == 0 || (i % 2) == 0)
				determinanta += matrica[0][i] * *(char*)recvbuf;
			else
				determinanta += (-1)*matrica[0][i] * *(char*)recvbuf;

			closesocket(acceptedSocket);
			memset(recvbuf, 0, bajtoviZaPrimanje);*/
		}

		bool Zzzz = true;
		bool* ugaseni = (bool*)calloc(1, sizeof(bool)*brojRedova);
		int koma = 0;

		while (Zzzz)
		{
			DWORD exitCode;
			for (int i = 0; i < brojRedova; i++)
			{
				GetExitCodeThread(Tredovi[i], &exitCode);
				if (exitCode != STILL_ACTIVE && ugaseni[i] == false)
				{
					CloseHandle(Tredovi[i]);
					ugaseni[i] = true;
					koma++;
				}

			}
			if (koma >= brojRedova)
				Zzzz = false;

		}
		free(ugaseni);

		free(recvbuf);
		closesocket(listenSocket);
	}

	else
		*determinanta = matrica[0][0] * matrica[1][1] - matrica[0][1] * matrica[1][0];


	return *determinanta;
}

int** napraviManjuMatricu(int** staraMatrica, int stariRed, int kolonaIzbacivanja)
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
	*((short*)zaSlanje + 1) = Port + i;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	BOOL bCreateProcess = NULL;

	bCreateProcess = CreateProcessA("..\\Debug\\IzvrsnaJedinica_clientServer.exe",
		zaSlanje, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	//drugi - parametre koje saljemo
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

