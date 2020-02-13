#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>

int SelectCitanje(SOCKET* socket)
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

	iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);

	return iResult;
}

void Recv1(SOCKET* socket, char* buff, int lenght)
{
	int primljeno = lenght;
	int iResult = 0;
	while (primljeno > 0)
	{
		iResult = SelectCitanje(socket);

		if (iResult == 0)
		{
			continue;
		}

		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			break;
		}


		iResult = recv(*socket, buff + lenght- primljeno, primljeno, 0);

		if (iResult > 0) {
			primljeno -= iResult;
		}

	}
	

}