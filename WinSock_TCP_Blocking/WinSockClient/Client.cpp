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
#define DEFAULT_PORT 1025

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();

int** NapraviMatricu(int a, int b,int kontrola);
void IspisiMatricu(int** M);

typedef struct MatricaObaFormata 
{
	int** matrica;
	char* stringMatrica;
} MA;

int __cdecl main(int argc, char **argv) 
{
	char* pomocniBuffer = (char*)calloc(1, 1);
	char* recvbuf = (char*)calloc(1, sizeof(int) + 1);
	

	int dimenzija, ku=0;
	printf("Uneti dimenziju matrice mxm:");
	scanf("%d", &dimenzija);


	int  pom = 2;
	char kolona =dimenzija, red = dimenzija;

	do
	{
		printf("Ako zelite samostalno da unesete brojeve unesite 1\n Ako zelite nasumicno generisanje matrice unesite 2\n");
		scanf("%d", &dimenzija);
		if (dimenzija == 1 || dimenzija == 2)
			ku = 1;
	}
	while (ku == 0);

	int duzina = (int)(kolona*red) * sizeof(int) + 2;

	char* prvaPoruka = (char*)calloc(1, sizeof(int));
	*prvaPoruka = duzina;

	int** k = NapraviMatricu(red,kolona,dimenzija);

	char* niz = (char*)calloc(1, sizeof(char)*2 + sizeof(int)*kolona*red);

	*(char*)niz = red;
	*(char*)(niz + 1) = kolona;

	for (int i = 0; i < red; i++)
	{
		for (int j = 0; j < kolona; j++)
		{
			*(int*)(niz + pom) = k[i][j];
			pom += 4;
			
		}
	}

	

	

    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // message to send
    char *messageToSend = "this is a test";

	char* firstMessage =(char*) malloc(sizeof(char));
    
    // Validate the parameters
    if (argc != 2)
    {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    if(InitializeWindowsSockets() == false)
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
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(DEFAULT_PORT);
    // connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
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


	do {
		

		iResult = SelectPisanje(&connectSocket);


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

		iResult = send(connectSocket, prvaPoruka, 4, 0); //salje mu broj bajta koji se salje

		

		Send1(&connectSocket,niz, duzina);



		

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}
//---------------------------------------------------------------------------------------
		Recv1(&connectSocket, pomocniBuffer, sizeof(char));
		int bajtoviZaPrimanje = *(char*)pomocniBuffer;


		Recv1(&connectSocket, recvbuf, bajtoviZaPrimanje);
		

		int rezultat = *(int*)recvbuf;

		printf("Rezultat je: %d\n", rezultat);
		
		getchar();
//-------------------------------------------------------------------------------------

		
	} while (iResult == 0);
	

	
	free(recvbuf);
	system("pause");

	// cleanup
	closesocket(connectSocket);
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


int** NapraviMatricu(int a,int b,int kontrola)
{
	
	
	int i, j, red = a, kolona = b;

	int** matrica = (int **)calloc(1, red * sizeof(int*));

	for (i = 0; i < red; i++)
	{
		matrica[i] = (int *)calloc(1, kolona * sizeof(int));
	}
	
	int pom = 1;

	if (kontrola == 2)
	{
		for (i = 0; i < red; i++)
		{
			for (j = 0; j < kolona; j++)
			{

				matrica[i][j] = rand() % 10;
			}
		}
	}
	else
	{
		printf("Uneti elemente matrice:\n");
		for (i = 0; i < red; i++)
		{
			for (j = 0; j < kolona; j++)
			{
				scanf("%d",&pom);
				matrica[i][j] = pom;
			}
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


	return matrica;
}


void IspisiMatricu(int** M) 
{
	
	int k = sizeof(M); 
	int p = sizeof(M[0]);
	int p3 = sizeof(M[0][0]);

	int red = sizeof(M) / sizeof(M[0]);
	int kolona = sizeof(M[0]) / sizeof(M[0][0]);

	printf("Matrica = [  ");
	for (int i = 0; i < red; i++)
	{
		for (int j = 0; j < kolona; j++)
		{
			printf("%d ",M[i][j]);
		}
		if(i +1 < red)
		printf("\n");

	}
	printf("  ]");
}


