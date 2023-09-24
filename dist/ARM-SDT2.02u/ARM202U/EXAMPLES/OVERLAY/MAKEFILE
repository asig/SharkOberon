LIB=

.c.o:
	armcc -bi -c  -APCS 3/32/noswst $< 

.s.o:
	armasm -bi -APCS 3/32/noswst $<

all: ov/root
ov/root: main.o uue.o getdata.o overmgrs.o maininit.o startup.o
	armlink -bin -base 0x40000 -data 0x10000 -ov ovlist \
	      -o ov maininit.o startup.o main.o overmgrs.o uue.o getdata.o \
	      -FIRST maininit.o ${LIB}/armlib.32b -map -symbols - -list tS.map \
	      -v

startup.o: startup.s

overmgrs.o: overmgrs.s

maininit.o: maininit.s

main.o: main.c

uue.o: uue.c

getdata.o: getdata.c

