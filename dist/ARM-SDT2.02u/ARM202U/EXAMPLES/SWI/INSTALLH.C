typedef struct SWI_InstallHandler_struct
{ unsigned exception;
  unsigned workspace;
  unsigned handler;
} SWI_InstallHandler_block;


SWI_InstallHandler_block 
  __value_in_regs  
    __swi(0x70) SWI_InstallHandler(unsigned r0, unsigned r1, unsigned r2);

void InstallHandler(SWI_InstallHandler_block *regs_in,
                    SWI_InstallHandler_block *regs_out)
{ *regs_out=SWI_InstallHandler(regs_in->exception,
                               regs_in->workspace,
                               regs_in->handler);
}
