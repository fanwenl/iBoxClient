#/bin/bash
echo $0
echo $1
if [ -e $1 ];
then
echo $1 "文件存在"
if [ -e $PWD/jlink.jlinkconfig ];
then
rm $PWD/jlink.jlinkconfig
echo "删除已有的jlink.jlinkconfig"
fi
echo "h" >> jlink.jlinkconfig
echo "loadfile" $1 "0x08000000" >> jlink.jlinkconfig
echo "g" >> jlink.jlinkconfig
echo "qc" >> jlink.jlinkconfig
JLinkExe -device STM32F103ZE -Speed 4000 -IF JTAG -JTAGConf -1,-1 -CommanderScript jlink.jlinkconfig
else
echo $1 "文件不存在"
fi
