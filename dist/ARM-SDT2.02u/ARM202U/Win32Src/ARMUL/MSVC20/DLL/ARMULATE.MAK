SRCDIR1=..\..
EXTDIR1=..\..\..\clx
EXTDIR2=..\..\..\armdbg
CFLAGS= -O2 -c -D__STDC__ -D_WINDOWS -DCODE16 /MT -DWIN32 -I$(SRCDIR1) -I$(EXTDIR1) -I$(EXTDIR2) -DHOURGLASS_RATE=8191 -DRDI_VERBOSE
CC=cl
TOLOCAL=copy
OBJALL=ARMULDLL.OBJ ARMCOPRO.OBJ ARMOS.OBJ ARMVIRT.OBJ
ARMULLIB=..\iarm.lib
WINLIBS=user32.lib
RC=rc

#rules
armulate.dll: $(OBJALL)
armulate.dll: $(ARMULLIB)
	del $@
        link.exe /DEFAULTLIB:$(WINLIBS) /DLL /DEF:"..\..\armulate.def" /SUBSYSTEM:CONSOLE /INCREMENTAL:no /OUT:"armulate.dll" $(OBJALL) $(ARMULLIB)

armuldll.obj: ..\..\armuldll.c
armuldll.obj: ..\..\armuldll.h
        $(CC) $(CFLAGS) ..\..\armuldll.c

armcopro.obj: $(SRCDIR1)\armcopro.c
	$(CC) $(CFLAGS) $(SRCDIR1)\armcopro.c

armos.obj: $(SRCDIR1)\armos.c
	$(CC) $(CFLAGS) $(SRCDIR1)\armos.c

armvirt.obj: $(SRCDIR1)\armvirt.c
	$(CC) $(CFLAGS) $(SRCDIR1)\armvirt.c
