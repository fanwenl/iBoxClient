# iBoxClient #

[![Build Status](https://travis-ci.org/fanwenl/iBoxClient.svg?branch=master)](https://travis-ci.org/fanwenl/iBoxClient)

iBoxClient是运行在iBox产品上的固件软件包，其对应的服务器软件包为iBoxServer。本软件实现了一个简单的物联网系统。

## 硬件介绍 ##

[iBox:物联网开源硬件](http://z.elecfans.com/135.html)详细信息请在聚丰众筹网站查看，主要功能如下：
- CPU：STM32F103ZET6
- 供电：支持12V  DC005电源供电、PoE供电；
- 网络：
    * Wifi:ESP8266
    * GPRS:SIM800L
    * 以太网:W5500
    * LoRa:SX1276
- IO：支持2路输出控制继电器标准电路、数字输入检测电路、ADC、DAC各一路。
- 电路板作为物联网通用平台，既可以实现物联网网关、也可以实现物联网终端。

## 软件介绍 ##

目前只实现了一下底层的驱动文件，其他功能正在开发中....
- RTOS：RT-Thread
- MQTT：

## 编译 ##
目前支持两种编译方式
- Keil5：

    工程文件在\Project目录中。
- GCC：

    在更目录中执行make命令即可：
    `make`
    编译选项：

    * V=1 输出全部的编译信息.(默认V=0 精简输出编译信息)
    * USE_WIFI 由于WIFI和GPRS共用接口，所有WIFI和GPRS只能选之一。默认使用GPRS，当USE_WIFI=1时使用WIFI。
    * USE_RTOS 是否使用RTOS.默认是`使用`。在使用本项目的驱动文件是请注意该选项。

    其他：

    本项目使用的arm-none-eabi版本为`4.9.3 20150529`。其他版本未测试。
## [工具](./tools/readme.md) ##
tools文件夹中包还一些该项目的工具。

## 许可证 ##
