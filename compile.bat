@echo off

set name="NESMaze"
set CC65_HOME=..\
set path=%path%;..\bin

cc65 -Oi main.c -g --add-source || goto fail
ca65 crt0.s -g || goto fail
ca65 main.s -g || goto fail
ld65 -C nrom_256_horz.cfg -o %name%.nes crt0.o main.o nes.lib -Ln labels.txt || goto fail

REM del main.s
del *.o
del labels.txt

%name%.nes

goto done

:fail

pause

:done