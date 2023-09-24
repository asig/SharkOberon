/*
	A simple routine to UUENCODE a buffer. It does not split up the
	uuencoded data into lines and append encode count bytes though.
	It assumes that the input buffer is an integer multiple of 3 bytes long.
	The number of bytes written to the output buffer is returned.
*/
unsigned int uuencode(unsigned char *in,unsigned char *out,unsigned int count)
{
        unsigned char t0;
        unsigned char t1;
        unsigned char t2;
	unsigned int result=0;
        for (;count;count-=3,in+=3,out+=4,result+=4) {
                t0=in[0];
		*out=' '+(t0>>2);
		t1=in[1];
                out[1]=' '+((t0<<4)&0x30)+(t1>>4);
		t2=in[2];
		out[2]=' '+((t1<<2)&0x3C)+ (t2>>6);
                out[3]=' '+(t2&0x3F);
        }
        return result;
}
