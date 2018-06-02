@echo off
set PATH= %path%;C:\Program Files (x86)\SEGGER\JLink_V620e
::注意if-else语句的格式
::注意.bat和.sh脚本的区别
if exist %1 (
    echo %1 file exist 
    if exist %cd%\jlink.config (
        del %cd%\jlink.config
        echo del exit jlink.config
    )
    echo h >> jlink.config
    echo loadfile %1 0x8000000 >> jlink.config
    echo g >> jlink.config
    echo qc >> jlink.config
    echo hhh
    jLink.exe -device STM32F103ZE -Speed auto -IF JTAG -JTAGConf -1,-1 -CommanderScript jlink.config
)else (
    echo %1 file not found
)