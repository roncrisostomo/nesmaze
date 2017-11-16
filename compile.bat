@echo off

set name="NESMaze"
set CC65_HOME=..\
set path=%path%;..\bin

cc65 -Oi game.c -g --add-source || goto fail
ca65 crt0.s -g || goto fail
ca65 game.s -g || goto fail
ld65 -C nrom_256_horz.cfg -o %name%.nes crt0.o game.o nes.lib -Ln labels.txt || goto fail

del game.s
del *.o
del labels.txt

%name%.nes

goto done

:fail

pause

:done