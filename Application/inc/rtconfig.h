#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Project Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 100
#define RT_DEBUG
#define RT_USING_OVERFLOW_CHECK
#define RT_DEBUG_INIT 0
#define RT_DEBUG_THREAD 0
#define RT_USING_HOOK
#define IDLE_THREAD_STACK_SIZE 512
#define RT_USING_TIMER_SOFT

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

//#define RT_USING_DEVICE
//#define RT_USING_CONSOLE                 //console定义了rt_kprintf相关的函数
#define RT_CONSOLEBUF_SIZE 128             //finsh中有使用
//#define RT_CONSOLE_DEVICE_NAME "uart1"
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M3

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_ARG_MAX 10

/* Device virtual file system */

//#define RT_USING_DFS
//#define DFS_USING_WORKDIR
//#define DFS_FILESYSTEMS_MAX 2
//#define DFS_FILESYSTEM_TYPES_MAX 2
//#define DFS_FD_MAX 4
//#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

//#define RT_DFS_ELM_CODE_PAGE 437
//#define RT_DFS_ELM_WORD_ACCESS
//#define RT_DFS_ELM_USE_LFN_0
//#define RT_DFS_ELM_USE_LFN 0
//#define RT_DFS_ELM_MAX_LFN 255
//#define RT_DFS_ELM_DRIVES 2
//#define RT_DFS_ELM_MAX_SECTOR_SIZE 512
//#define RT_DFS_ELM_REENTRANT
//#define RT_USING_DFS_DEVFS
//#define RT_USING_DFS_NET

/* Device Drivers */

//#define RT_USING_RTC

/* Using USB */


/* POSIX layer and C standard library */

//#define RT_USING_LIBC
//#define RT_USING_POSIX

/* Network stack */

/* light weight TCP/IP stack */

//#define RT_USING_LWIP
//#define RT_USING_LWIP202
//#define RT_LWIP_IGMP
//#define RT_LWIP_ICMP
//#define RT_LWIP_DNS
//#define RT_LWIP_DHCP
//#define IP_SOF_BROADCAST 1
//#define IP_SOF_BROADCAST_RECV 1

/* Static IPv4 Address */

//#define RT_LWIP_IPADDR "192.168.1.30"
//#define RT_LWIP_GWADDR "192.168.1.1"
//#define RT_LWIP_MSKADDR "255.255.255.0"
//#define RT_LWIP_UDP
//#define RT_LWIP_TCP
//#define RT_MEMP_NUM_NETCONN 8
//#define RT_LWIP_PBUF_NUM 16
//#define RT_LWIP_RAW_PCB_NUM 4
//#define RT_LWIP_UDP_PCB_NUM 4
//#define RT_LWIP_TCP_PCB_NUM 4
//#define RT_LWIP_TCP_SEG_NUM 40
//#define RT_LWIP_TCP_SND_BUF 8196
//#define RT_LWIP_TCP_WND 8196
//#define RT_LWIP_TCPTHREAD_PRIORITY 10
//#define RT_LWIP_TCPTHREAD_MBOX_SIZE 8
//#define RT_LWIP_TCPTHREAD_STACKSIZE 1024
//#define RT_LWIP_ETHTHREAD_PRIORITY 12
//#define RT_LWIP_ETHTHREAD_STACKSIZE 1024
//#define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
//#define LWIP_NETIF_STATUS_CALLBACK 1
//#define SO_REUSE 1
//#define LWIP_SO_RCVTIMEO 1
//#define LWIP_SO_SNDTIMEO 1
//#define LWIP_SO_RCVBUF 1
//#define LWIP_NETIF_LOOPBACK 0

/* Modbus master and slave stack */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* ARM CMSIS */

//#define RT_USING_RTT_CMSIS

/* RT-Thread online packages */

/* system packages */

/* RT-Thread GUI Engine */


/* IoT - internet of things */

//#define PKG_USING_PAHOMQTT
//#define PKG_USING_PAHOMQTT_EXAMPLE
//#define PKG_USING_PAHOMQTT_TEST
//#define MQTT_USING_TLS
//#define PKG_PAHOMQTT_SUBSCRIBE_HANDLERS 1
//#define PKG_USING_PAHOMQTT_LATEST_VERSION

/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* security packages */

//#define PKG_USING_MBEDTLS
//#define PKG_USING_MBEDTLS_LATEST_VERSION

/* language packages */


/* multimedia packages */


/* tools packages */

#define PKG_USING_CMBACKTRACE
#define PKG_USING_CMBACKTRACE_LATEST_VERSION
#define PKG_CMBACKTRACE_PLATFORM_M3
#define PKG_CMBACKTRACE_DUMP_STACK
#define PKG_CMBACKTRACE_PRINT_ENGLISH
#define PKG_USING_SYSTEMVIEW
#define PKG_SYSVIEW_APP_NAME "RT-Thread Trace"
#define PKG_SYSVIEW_DEVICE_NAME "Cortex-M3"
#define PKG_SYSVIEW_TIMESTAMP_FREQ 0
#define PKG_SYSVIEW_CPU_FREQ 0
#define PKG_SYSVIEW_RAM_BASE 0x20000000
#define PKG_SYSVIEW_EVENTID_OFFSET 32
#define PKG_SYSVIEW_USE_CYCCNT_TIMESTAMP
#define PKG_SYSVIEW_SYSDESC0 "I#15=SysTick"
#define PKG_SYSVIEW_SYSDESC1 ""
#define PKG_SYSVIEW_SYSDESC2 ""

/* Segger RTT configuration */

#define PKG_SEGGER_RTT_MAX_NUM_UP_BUFFERS 3
#define PKG_SEGGER_RTT_MAX_NUM_DOWN_BUFFERS 3
#define PKG_SEGGER_RTT_BUFFER_SIZE_UP 1024
#define PKG_SEGGER_RTT_BUFFER_SIZE_DOWN 16
#define PKG_SEGGER_RTT_PRINTF_BUFFER_SIZE 64
#define PKG_SEGGER_RTT_MODE_ENABLE_NO_BLOCK_SKIP
#define PKG_SEGGER_RTT_MAX_INTERRUPT_PRIORITY 0x20

/* SystemView buffer configuration */

#define PKG_SEGGER_SYSVIEW_RTT_BUFFER_SIZE 1024
#define PKG_SEGGER_SYSVIEW_RTT_CHANNEL 1
#define PKG_SEGGER_SYSVIEW_USE_STATIC_BUFFER
//关闭事后分析模式
//#define PKG_SEGGER_SYSVIEW_POST_MORTEM_MODE

/* SystemView Id configuration */

#define PKG_SEGGER_SYSVIEW_ID_BASE 0x10000000
#define PKG_SEGGER_SYSVIEW_ID_SHIFT 2

/* miscellaneous packages */


/* example package: hello */

//#define SOC_STM32F1

#endif
