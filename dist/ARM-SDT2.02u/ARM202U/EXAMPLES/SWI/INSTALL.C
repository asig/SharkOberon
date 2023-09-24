#include <stdio.h>
#include <stdlib.h>

extern void SWIHandler (void);
extern void MakeChain (void);

unsigned *Dswivec = (unsigned *) 0x20; /* ie place to store old one */

struct four_results 
{	unsigned a;
	unsigned b;
	unsigned c;
	unsigned d;
};

__swi (256) void  my_swi_256 (void);
__swi (257) void my_swi_257 (unsigned);
__swi (258) unsigned my_swi_258 (unsigned,unsigned,unsigned,unsigned);
__swi (259) __value_in_regs struct four_results
			my_swi_259 (unsigned, unsigned, unsigned,unsigned);

unsigned Install_Handler (unsigned routine, unsigned *vector)
/* Updates contents of 'vector' to contain branch instruction   */
/* to reach 'routine' from 'vector'. Function return value is   */                             /* original contents of 'vector'.                               */
/* NB: 'Routine' must be within range of 32Mbytes from 'vector'.*/
{	unsigned vec, oldvec;
	vec = ((routine - (unsigned)vector - 0x8)>>2);
 	if (vec & 0xff000000) 
	{	printf ("Installation of Handler failed");
		exit (0);
	}
	vec = 0xea000000 | vec;
	oldvec = *vector;
	*vector = vec;
	return (oldvec);
}

void Update_Demon_Vec (unsigned *original, unsigned *Dvec)
/* Returns updated instruction 'LDR pc, [pc,#offset]' when   */
/* moved from 'original' to 'Dvec' (ie recalculates offset). */
/* Assumes 'Dvec' is higher in memory than 'original'.       */
{	
	*Dvec = ((*Dvec &0xfff)
		- ((unsigned) Dvec - (unsigned) original))
		| (*Dvec & 0xfffff000);
}

unsigned C_SWI_Handler (unsigned number, unsigned *reg) 
{	unsigned done = 1;

	/* Set up parameter storage block pointers */
	unsigned *called_256 = (unsigned *) 0x24;
	unsigned *param_257 = (unsigned*) 0x28;
	unsigned *param_258 = (unsigned*) 0x2c; /* + 0x30,0x34,0x38 */
	unsigned *param_259 = (unsigned*) 0x3c; /* + 0x40,0x44,0x48 */
	switch (number) 
	{	case 256: 
			*called_256 = 256; /* Store a value to show that */
			break;             /* SWI was handled correctly. */
		case 257: 
			*param_257 = reg [0]; /* Store parameter */
			break;
		case 258:
			*param_258++ = reg [0]; /* Store parameters */
			*param_258++ = reg [1];
			*param_258++ = reg [2];
			*param_258 = reg [3];
			/* Now calculate result */
			reg [0] += reg [1] + reg [2] + reg [3];
			break;
		case 259:
			*param_259++ = reg [0]; /* Store parameters */
			*param_259++ = reg [1];
			*param_259++ = reg [2];
			*param_259 = reg [3];
			reg [0] *= 2; /* Calculate results */
			reg [1] *= 3;
			reg [2] *= 4;
			reg [3] *= 5;
			break;
		default: done = 0; /* SWI not handled */
	}
	return (done);
}

int main ()
{	
	struct four_results r_259; /* Results from SWI 259 */
	unsigned *swivec = (unsigned *)0x8; /* Pointer to SWI vector */
	*Dswivec = Install_Handler ((unsigned)SWIHandler, swivec);
	Update_Demon_Vec (swivec, Dswivec);
	MakeChain ();

	printf("Hello 256\n");
	my_swi_256 ();
	printf("Hello 257\n");
	my_swi_257 (257);
	printf("Hello 258\n");
	printf(" Result = %u\n",my_swi_258 (1,2,3,4));
	printf ("Hello 259\n");
	r_259 = my_swi_259 (10,20,30,40);
	printf (" Results are: %u %u %u %u\n",r_259.a,r_259.b,r_259.c,r_259.d);	
	printf("The end\n");
	return (0);
}
