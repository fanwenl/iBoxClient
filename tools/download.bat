@echo off

::请按照自己电脑Jlink安装位置修改PATH中jLink.exe的路径(可以将jlink.exe添加到环境变量)
::按照使用的芯片型号修改-device参数。
::按照使用的接口SWD或者JTAG修改：
::其他参数的修改课具体查看jlink的help。
::JTAG:jLink.exe -device STM32F103ZE -Speed auto -si JTAG -Config -1,-1 -CommanderScript jlink.jlinkconfig
::JTAG:jLink.exe -device STM32F103ZE -Speed auto -IF JTAG -JTAGConf -1,-1 -CommanderScript jlink.jlinkconfig
::SWD:jLink.exe -device STM32F103VC -Speed auto -si SWD -CommanderScript jlink.jlinkconfig

set PATH= %path%;C:\Program Files (x86)\SEGGER\JLink_V6232b\jLink.exe
::注意if-else语句的格式
::注意.bat和.sh脚本的区别
if exist %1 (
    echo %1 file exist 
    if exist %cd%\jlink.jlinkconfig (
        del %cd%\jlink.jlinkconfig
        echo del exit jlink.jlinkconfig
    )
    echo h >> jlink.jlinkconfig
    echo loadfile %1 0x8000000 >> jlink.jlinkconfig
    echo g >> jlink.jlinkconfig
    echo qc >> jlink.jlinkconfig
    jLink.exe -device STM32F103ZE -Speed auto -si JTAG -JTAGConf -1,-1 -CommanderScript jlink.jlinkconfig
)else (
    echo %1 file not found
)