#include <stdio.h>
#include <string.h>
#define INBUFFSIZE 192
#define OUTBUFFSIZE 257

static char IDstring[]="This is initailised data.";
static char inBuff[INBUFFSIZE];
static char outBuff[OUTBUFFSIZE];
static int charCount=0;

extern int getData(char *,int);
extern int uuencode(char *, char *, int);

int main(int argc,char **argv)
{
	int uuCount;
	charCount = getData(inBuff,sizeof(inBuff));
	if (charCount<0) {
		fprintf(stderr,"Error reading data.\n");
	}
	else {
		uuCount=uuencode(inBuff,outBuff,charCount);
		outBuff[uuCount]='\0';
		puts(outBuff);
	}
	return 0;
}

void MemCopy(void *d,void *s,int c)
{
	memmove(d,s,c);
}

