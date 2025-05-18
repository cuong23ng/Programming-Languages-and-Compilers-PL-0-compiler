@echo off
setlocal

cd "D:\git-repos\PhuongPhapDich"

gcc mainDich.c phantichtuvung.c phantichcuphap.c -o mainDich.exe && mainDich.exe tong.pl0

endlocal
pause