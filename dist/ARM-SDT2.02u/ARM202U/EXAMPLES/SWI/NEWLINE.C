void __swi(0) SWI_WriteC(int ch);

void output_newline(void)
{ SWI_WriteC(13);
  SWI_WriteC(10);
}
