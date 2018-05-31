#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef __GNUC__
#define inline
#endif

//#define USE_SX1232_RADIO
//#define USE_SX1272_RADIO
#define USE_SX1276_RADIO
//#define USE_SX1243_RADIO

/*!
 * Module choice. There are three existing module with the SX1276.
 * Please set the connected module to the value 1 and set the others to 0
 */
#ifdef USE_SX1276_RADIO
#define MODULE_SX1276RF1IAS                         0
#define MODULE_SX1276RF1JAS                         0
#define MODULE_SX1276RF1KAS                         1
#endif


#endif // __CONFIG_H__
