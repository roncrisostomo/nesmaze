@echo off

set name="NESMaze"
set CC65_HOME=..\
set scriptsDir=scripts
set libDir=scripts\lib

cc65 -Oi %scriptsDir%\main.c -g --add-source || goto fail
ca65 %libDir%\crt0.s -g || goto fail
ca65 %scriptsDir%\main.s -g || goto fail
ld65 -C %libDir%\nrom_256_horz.cfg -o %name%.nes %libDir%\crt0.o %scriptsDir%\main.o nes.lib -Ln labels.txt || goto fail

REM del main.s
del %scriptsDir%\*.o
del %libDir%\*.o
del labels.txt

%name%.nes

goto done

:fail

pause

:done