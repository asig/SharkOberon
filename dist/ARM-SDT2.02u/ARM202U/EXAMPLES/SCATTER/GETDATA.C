#include <stdio.h>

int getData(char *buffer,int length)
{
	int charsIn;
	int charRead;
	charsIn=0;
	for (charsIn=0;charsIn<length;) {
		charRead=getchar();
		if (charRead == EOF) break;
		buffer[charsIn++]=charRead;
		if (charRead == '\n' ) break;
	}
	for (;(charsIn%3)!=0; ) buffer[charsIn++]='\0';
	return charsIn;
}
