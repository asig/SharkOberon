#include <stdio.h>
#include <string.h>
#include "lib.h"

void StringFunction(int nNum, char *szString);

int main()
{
	char szText[80] = "";

/* HASH_DEFINED has been defined as a per-file parameter    */

	printf(HASH_DEFINED);

/* Call a function in a pre-built library           */

	print("Hello, World - From the Library!";

/* A simple printf call                             */

	printf("Another Line.\n");

/* A call to a simple function                      */

	StringFunction(1, szText);
	printf("StringFunction() returned: '%s'\n", szText);

	StringFunction(1, szText);
	printf("StringFunction() returned: '%s'\n", szText);
}

void StringFunction(int nNum, char *szString)
{
	switch (nNum)
	{
		case 1:
		{
			strcpy(szString, "ARM Project Manager");
			break;
		}
		case 2:
		{
			strcpy(szString, "ARM Software Development Toolkit v2.00");
			break;
		}
		default:
		{
			strcpy(szString, "Default");
			break;
		}
	}
}
