#/bin/bash
echo $0
echo $1
if [ -e $1 ];
then
echo $1 "文件存在"
if [ -e $PWD/jlink.jlink ];
then
rm $PWD/jlink.jlink
echo "删除已有的jlink.jlink"
fi
echo "h" >> jlink.jlink
echo "loadfile" $1 "0x08000000" >> jlink.jlink
echo "g" >> jlink.jlink
echo "qc" >> jlink.jlink
JLinkExe -device STM32F103ZE -Speed 4000 -IF JTAG -JTAGConf -1,-1 -CommanderScript jlink.jlink
else
echo $1 "文件不存在"
fi
