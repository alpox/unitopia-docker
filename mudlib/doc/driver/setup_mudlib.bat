@echo off
REM Dieses Script legt die wichtigsten Verzeichnisse an, die noch fehlen.

cd ..\lib

if not exist static\adm\LOAD_AT_STARTUP.org goto weiter
echo Die Verzeichnisse wurden anscheinend schon angelegt.
goto ende

:weiter
mkdir d
mkdir log
mkdir log\sys
mkdir p
mkdir save
REM mkdir var
mkdir var\adm
mkdir var\adm\logfiles
mkdir var\spool
mkdir var\statistik
mkdir var\statistik\buch
mkdir var\spool\wahlen
mkdir var\players
cd var\players
for %%f in (a b c d e f g h i j k l m n o p q r s t u v w x y z suicid wizzes) do mkdir %%f
cd ..\..
mkdir var\spool\mail
cd var\spool\mail
for %%f in (a b c d e f g h i j k l m n o p q r s t u v w x y z suicid) do mkdir %%f
cd ..\..\..
mkdir var\spool\news
mkdir w
echo *.*.*.*:0:-1:0:0:No msg >> ACCESS.ALLOW
copy static\adm\LOAD_AT_STARTUP static\adm\LOAD_AT_STARTUP.org
:ende
cd ..\bin
