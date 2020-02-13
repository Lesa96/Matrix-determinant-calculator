#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>


int SelectPisanje(SOCKET* socket)
{
	int iResult = 0;
	FD_SET set;
	timeval timeVal;

	FD_ZERO(&set);
	// Add socket we will wait to read from
	FD_SET(*socket, &set);

	// Set timeouts to zero since we want select to return
	// instantaneously
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);

	return iResult;
}


void Send1(SOCKET* socket, char* buffer, int lenght) // popravi
{
	int poslatoBajta = 0;
	int novaDuzinaPoruke = 0;

	int iResult = 0;
	while (poslatoBajta != lenght)
	{

		iResult = SelectPisanje(socket);


		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			break;
		}

		// now, lets check if there are any sockets ready
		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(50);
			continue;
		}


		novaDuzinaPoruke = lenght - poslatoBajta;

		iResult = send(*socket, buffer + (poslatoBajta), novaDuzinaPoruke, 0);
		poslatoBajta += iResult;
	}
	printf("\n\nBytes Sent: %d\n", poslatoBajta);
}

