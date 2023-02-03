cls

@echo off
setlocal

echo Scan for FanX dongle and put into bootloader mode....

for /f "tokens=1* delims==" %%I in ('wmic path win32_pnpentity get caption  /format:list ^| find "Arduino Micro"') do (
    call :resetCOM "%%~J"
)

:: end main batch
goto :EOF

:resetCOM <WMIC_output_line>
:: sets _COM#=line
setlocal
set "str=%~1"
set "num=%str:*(COM=%"
set "num=%num:)=%"
set port=COM%num%
echo FanX dongle found at: %port%
mode %port%: BAUD=1200 parity=N data=8 stop=1
timeout 4 > NUL
::goto :continue
goto :flash


:flash
echo Start flashing...
avrdude-v7.1-windows-windows-x64\avrdude -c avr109 -P usb:1B4F:9203 -p m32u4 -D -Uflash:w:mysensors.hex:i
