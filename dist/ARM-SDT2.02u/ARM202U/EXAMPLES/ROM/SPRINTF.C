#include <stdio.h>

/* We use the following Debug Monitor SWIs to write things out
 * in this example
 */
extern __swi(2) Write0(char *s);	/* Write a string */

/* The following symbols are defined by the linker and define
 * various memory regions which may need to be copied or initialised
 */
extern char Image$$RO$$Base[];
extern char Image$$RO$$Limit[];
extern char Image$$RW$$Base[];

/* We define some more meaningful names here */
#define rom_code_base Image$$RO$$Base
#define rom_data_base Image$$RO$$Limit
#define ram_data_base Image$$RW$$Base

void C_Entry(void)
{
  char s[80];

  if (rom_data_base == ram_data_base) {
    Write0("Warning: Image has been linked as an application. To link as a ROM image\r\n");
    Write0("         link with the options -RO <rom-base> -RW <ram-base>\r\n");
  }

  sprintf(s, "ROM is at address %p, RAM is at address %p\n", rom_code_base, ram_data_base);
  Write0(s);
}
