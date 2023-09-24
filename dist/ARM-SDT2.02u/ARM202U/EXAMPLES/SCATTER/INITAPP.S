       AREA InitApp, CODE , READONLY
       EXPORT InitialiseApp
InitialiseApp
       ADR    r0,ziTable
       MOV    R3,#0
ziLoop
       LDR    r1,[r0],#4
       CMP    r1,#0
       BEQ    initLoop
       LDR    r2,[r0],#4
ziFillLoop
       STR    r3,[r2],#4
       SUBS   r1,r1,#4
       BNE    ziFillLoop
       B      ziLoop

initLoop
       LDR    r1,[r0],#4
       CMP    r1,#0
       MOVEQ  pc,lr
       LDMIA  r0!,{r2,r3}
       CMP    r1,#16
       BLT    copyWords
copy4Words
       LDMIA  r3!,{r4,r5,r6,r7}
       STMIA  r2!,{r4,r5,r6,r7}
       SUBS   r1,r1,#16
       BGT    copy4Words
       BEQ    initLoop
copyWords
       SUBS r1,r1,#8
       LDMIAGE r3!,{r4,r5}
       STMIAGE r2!,{r4,r5}
       BEQ   initLoop

       LDR    r4,[r3]
       STR    r4,[r2]

       B      initLoop
       
       MACRO
       ZIEntry  $execname
       LCLS   lensym
       LCLS   basesym
       LCLS   namecp
namecp SETS "$execname"
lensym SETS   "|Image$$":CC:namecp:CC:"$$ZI$$Length|"
basesym SETS   "|Image$$":CC:namecp:CC:"$$ZI$$Base|"
       IMPORT $lensym
       IMPORT $basesym
       DCD    $lensym
       DCD    $basesym
       MEND

       MACRO
       InitEntry  $execname
       LCLS   lensym
       LCLS   basesym
       LCLS   loadsym
       LCLS   namecp
namecp SETS "$execname"
lensym SETS   "|Image$$":CC:namecp:CC:"$$Length|"
basesym SETS   "|Image$$":CC:namecp:CC:"$$Base|"
loadsym SETS   "|Load$$":CC:namecp:CC:"$$Base|"
       IMPORT $lensym
       IMPORT $basesym
       IMPORT $loadsym
       DCD    $lensym
       DCD    $basesym
       DCD    $loadsym
       MEND

ziTable
       ZIEntry root
       DCD    0

InitTable
       InitEntry root
       InitEntry overmgr
       DCD    0
       END
