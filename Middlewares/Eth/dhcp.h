//*****************************************************************************
//
//! \file dhcp.h
//! \brief DHCP APIs Header file.
//! \details Processig DHCP protocol as DISCOVER, OFFER, REQUEST, ACK, NACK and DECLINE.
//! \version 1.1.0
//! \date 2013/11/18
//! \par  Revision history
//!       <2013/11/18> 1st Release
//!       <2012/12/20> V1.1.0
//!         1. Move unreferenced DEFINE to dhcp.c
//!       <2012/12/26> V1.1.1
//! \author Eric Jung & MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//! 
//! Redistribution and use in source and binary forms, with or without 
//! modification, are permitted provided that the following conditions 
//! are met: 
//! 
//!     * Redistributions of source code must retain the above copyright 
//! notice, this list of conditions and the following disclaimer. 
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution. 
//!     * Neither the name of the <ORGANIZATION> nor the names of its 
//! contributors may be used to endorse or promote products derived 
//! from this software without specific prior written permission. 
//! 
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
#ifndef _DHCP_H_
#define _DHCP_H_

#include "stdint.h"

/*
 * @brief 
 * @details If you want to display debug & procssing message, Define _DHCP_DEBUG_ 
 * @note    If defined, it dependens on <stdio.h>
 */
#define _DHCP_DEBUG_


/* Retry to processing DHCP */
#define	MAX_DHCP_RETRY          2        ///< Maxium retry count
#define	DHCP_WAIT_TIME          10       ///< Wait Time 10s


/* UDP port numbers for DHCP */
#define DHCP_SERVER_PORT      	 67	      ///< DHCP server port number
#define DHCP_CLIENT_PORT         68	      ///< DHCP client port number


#define MAGIC_COOKIE             0x63827663  ///< Any number. You can be modifyed it any number

#define DCHP_HOST_NAME           "iBoxClient\0"

/* DHCP state machine. */
#define STATE_DHCP_INIT          0        ///< Initialize
#define STATE_DHCP_DISCOVER      1        ///< send DISCOVER and wait OFFER
#define STATE_DHCP_REQUEST       2        ///< send REQEUST and wait ACK or NACK
#define STATE_DHCP_LEASED        3        ///< ReceiveD ACK and IP leased
#define STATE_DHCP_REREQUEST     4        ///< send REQUEST for maintaining leased IP
#define STATE_DHCP_RELEASE       5        ///< No use
#define STATE_DHCP_STOP          6        ///< Stop procssing DHCP

#define DHCP_FLAGSBROADCAST      0x8000   ///< The broadcast value of flags in @ref RIP_MSG 
#define DHCP_FLAGSUNICAST        0x0000   ///< The unicast   value of flags in @ref RIP_MSG

/* DHCP message OP code */
#define DHCP_BOOTREQUEST         1        ///< Request Message used in op of @ref RIP_MSG
#define DHCP_BOOTREPLY           2        ///< Reply Message used i op of @ref RIP_MSG

/* DHCP message type */
#define DHCP_DISCOVER            1        ///< DISCOVER message in OPT of @ref RIP_MSG
#define DHCP_OFFER               2        ///< OFFER message in OPT of @ref RIP_MSG
#define DHCP_REQUEST             3        ///< REQUEST message in OPT of @ref RIP_MSG
#define DHCP_DECLINE             4        ///< DECLINE message in OPT of @ref RIP_MSG
#define DHCP_ACK                 5        ///< ACK message in OPT of @ref RIP_MSG
#define DHCP_NAK                 6        ///< NACK message in OPT of @ref RIP_MSG
#define DHCP_RELEASE             7        ///< RELEASE message in OPT of @ref RIP_MSG. No use
#define DHCP_INFORM              8        ///< INFORM message in OPT of @ref RIP_MSG. No use

#define DHCP_HTYPE10MB           1        ///< Used in type of @ref RIP_MSG
#define DHCP_HTYPE100MB          2        ///< Used in type of @ref RIP_MSG

#define DHCP_HLENETHERNET        6        ///< Used in hlen of @ref RIP_MSG
#define DHCP_HOPS                0        ///< Used in hops of @ref RIP_MSG
#define DHCP_SECS                0        ///< Used in secs of @ref RIP_MSG

#define INFINITE_LEASETIME       0xffffffff	///< Infinite lease time

#define OPT_SIZE                 312               /// Max OPT size of @ref RIP_MSG
#define RIP_MSG_SIZE             (236+OPT_SIZE)    /// Max size of @ref RIP_MSG

/* 
 * @brief DHCP option and value (cf. RFC1533)
 */
enum
{
   padOption               = 0,
   subnetMask              = 1,
   timerOffset             = 2,
   routersOnSubnet         = 3,
   timeServer              = 4,
   nameServer              = 5,
   dns                     = 6,
   logServer               = 7,
   cookieServer            = 8,
   lprServer               = 9,
   impressServer           = 10,
   resourceLocationServer	= 11,
   hostName                = 12,
   bootFileSize            = 13,
   meritDumpFile           = 14,
   domainName              = 15,
   swapServer              = 16,
   rootPath                = 17,
   extentionsPath          = 18,
   IPforwarding            = 19,
   nonLocalSourceRouting   = 20,
   policyFilter            = 21,
   maxDgramReasmSize       = 22,
   defaultIPTTL            = 23,
   pathMTUagingTimeout     = 24,
   pathMTUplateauTable     = 25,
   ifMTU                   = 26,
   allSubnetsLocal         = 27,
   broadcastAddr           = 28,
   performMaskDiscovery    = 29,
   maskSupplier            = 30,
   performRouterDiscovery  = 31,
   routerSolicitationAddr  = 32,
   staticRoute             = 33,
   trailerEncapsulation    = 34,
   arpCacheTimeout         = 35,
   ethernetEncapsulation   = 36,
   tcpDefaultTTL           = 37,
   tcpKeepaliveInterval    = 38,
   tcpKeepaliveGarbage     = 39,
   nisDomainName           = 40,
   nisServers              = 41,
   ntpServers              = 42,
   vendorSpecificInfo      = 43,
   netBIOSnameServer       = 44,
   netBIOSdgramDistServer	= 45,
   netBIOSnodeType         = 46,
   netBIOSscope            = 47,
   xFontServer             = 48,
   xDisplayManager         = 49,
   dhcpRequestedIPaddr     = 50,
   dhcpIPaddrLeaseTime     = 51,
   dhcpOptionOverload      = 52,
   dhcpMessageType         = 53,
   dhcpServerIdentifier    = 54,
   dhcpParamRequest        = 55,
   dhcpMsg                 = 56,
   dhcpMaxMsgSize          = 57,
   dhcpT1value             = 58,
   dhcpT2value             = 59,
   dhcpClassIdentifier     = 60,
   dhcpClientIdentifier    = 61,
   endOption               = 255
};

/*
 * @brief DHCP message format
 */ 
typedef struct {
    uint8_t  op;            ///< @ref DHCP_BOOTREQUEST or @ref DHCP_BOOTREPLY
    uint8_t  htype;         ///< @ref DHCP_HTYPE10MB or @ref DHCP_HTYPE100MB
    uint8_t  hlen;          ///< @ref DHCP_HLENETHERNET
    uint8_t  hops;          ///< @ref DHCP_HOPS
    uint32_t xid;           ///< @ref DHCP_XID  This increase one every DHCP transaction.
    uint16_t secs;          ///< @ref DHCP_SECS
    uint16_t flags;         ///< @ref DHCP_FLAGSBROADCAST or @ref DHCP_FLAGSUNICAST
    uint8_t  ciaddr[4];     ///< @ref Request IP to DHCP sever
    uint8_t  yiaddr[4];     ///< @ref Offered IP from DHCP server
    uint8_t  siaddr[4];     ///< No use 
    uint8_t  giaddr[4];     ///< No use
    uint8_t  chaddr[16];    ///< DHCP client 6bytes MAC address. Others is filled to zero
    uint8_t  sname[64];     ///< No use
    uint8_t  file[128];     ///< No use
    uint8_t  OPT[OPT_SIZE]; ///< Option
} RIP_MSG;




/* 
 * @brief return value of @ref DHCP_run()
 */
enum
{
   DHCP_FAILED = 0,  ///< Procssing Fail
   DHCP_RUNNING,     ///< Procssing DHCP proctocol
   DHCP_IP_ASSIGN,   ///< First Occupy IP from DHPC server      (if cbfunc == null, act as default default_ip_assign)
   DHCP_IP_CHANGED,  ///< Change IP address by new ip from DHCP (if cbfunc == null, act as default default_ip_update)
   DHCP_IP_LEASED,   ///< Stand by 
   DHCP_STOPPED      ///< Stop procssing DHCP protocol
};

/*
 * @brief DHCP client initialization (outside of the main loop)
 * @param s   - socket number
 * @param buf - buffer for procssing DHCP message
 */
void DHCP_init(uint8_t s, uint8_t * buf);

/*
 * @brief DHCP 1s Tick Timer handler
 * @note SHOULD BE register to your system 1s Tick timer handler 
 */
void DHCP_time_handler(void);

/* 
 * @brief Register call back function 
 * @param ip_assign   - callback func when IP is assigned from DHCP server first
 * @param ip_update   - callback func when IP is changed
 * @prarm ip_conflict - callback func when the assigned IP is conflict with others.
 */
void reg_dhcp_cbfunc(void(*ip_assign)(void), void(*ip_update)(void), void(*ip_conflict)(void));

/*
 * @brief DHCP client in the main loop
 * @return    The value is as the follow \n
 *            @ref DHCP_FAILED     \n
 *            @ref DHCP_RUNNING    \n
 *            @ref DHCP_IP_ASSIGN  \n
 *            @ref DHCP_IP_CHANGED \n
 * 			  @ref DHCP_IP_LEASED  \n
 *            @ref DHCP_STOPPED    \n
 *
 * @note This function is always called by you main task.
 */ 
uint8_t DHCP_run(void);

/*
 * @brief Stop DHCP procssing
 * @note If you want to restart. call DHCP_init() and DHCP_run()
 */ 
void    DHCP_stop(void);

/* Get Network information assigned from DHCP server */
/*
 * @brief Get IP address
 * @param ip  - IP address to be returned
 */
void getIPfromDHCP(uint8_t* ip);
/*
 * @brief Get Gateway address
 * @param ip  - Gateway address to be returned
 */
void getGWfromDHCP(uint8_t* ip);
/*
 * @brief Get Subnet mask value
 * @param ip  - Subnet mask to be returned
 */
void getSNfromDHCP(uint8_t* ip);
/*
 * @brief Get DNS address
 * @param ip  - DNS address to be returned
 */
void getDNSfromDHCP(uint8_t* ip);

/*
 * @brief Get the leased time by DHCP sever
 * @retrun unit 1s
 */
uint32_t getDHCPLeasetime(void);

#endif	/* _DHCP_H_ */
