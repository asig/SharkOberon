#include <stdio.h>
#include <string.h>
#define INBUFFSIZE 192
#define OUTBUFFSIZE 257

static char inBuff[INBUFFSIZE];
static char outBuff[OUTBUFFSIZE];
static int charCount=0;

extern int getData(char *,int);
extern int uuencode(char *, char *, int);
extern int _sys_flen(int );
extern int _sys_open(char *,int );
extern int _sys_read(int, void *, int, int );
extern int _sys_close(int);

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

int LoadOverlaySegment(int nameLen,char *name,void *baseAdr)
{
	char name0[16];
	int length;
	int fh;

	memmove(name0,name,nameLen);
	name0[nameLen]='\0';
#define OPEN_B 1
#define OPEN_R 0

	fh = _sys_open(name0,OPEN_B|OPEN_R);
	if (fh==0) return 0;
	length = _sys_flen(fh);
	(void)_sys_read(fh,baseAdr,length,0);
	(void)_sys_close(fh);
	return length;
}

