char __swi(4) SWI_ReadC(void);

void readline(char *buffer)
{ char ch;
  do {
    *buffer++=ch=SWI_ReadC();
  } while (ch!=13);
  *buffer=0;
}
