@echo off
ECHO Add CRC32  At the end of the document
ECHO -------------------------------------

SET SREC_PATH=C:\SREC

for /f %%i in ('dir /b .\raw.bin') do (
set indexdx=%%~zi
)

ECHO %indexdx%

ECHO %SREC_PATH%\srec_cat.exe raw.bin -Binary -crop 0 %indexdx%  -crc32-b-e %indexdx%  -o gps_bass.bin -Binary
%SREC_PATH%\srec_cat.exe raw.bin -Binary -crop 0 %indexdx%  -crc32-l-e %indexdx%  -o gps_bass.bin -Binary