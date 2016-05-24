/**
  ******************************************************************************
 * @file    CC2530.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    11-5-2015
 * @brief   CC2530 Zigbee Network Processor driver
  */
//-------------------------------------------------------------------------
#include "GPIO.h"
#include "SPI.h"

#ifndef CC2530_H
#define CC2530_H

namespace hv_driver {

class CC2530 {
public:
enum COMMAND {
	NONE												= 0xFFFF,
	/* System Interface */
	SYS_RESET_REQ               = 0x4100,
	SYS_RESET_IND               = 0x4180,
	SYS_VERSION                 = 0x2102,
	SYS_VERSION_SRSP            = 0x6102,
	SYS_OSAL_NV_READ            = 0x2108,
	SYS_OSAL_NV_SRSP            = 0x6108,
	SYS_OSAL_NV_WRITE           = 0x2109,
	SYS_OSAL_NV_WRITE_SRSP      = 0x6109,
	SYS_OSAL_NV_ITEM_INIT       = 0x2107,
	SYS_OSAL_NV_ITEM_INIT_SRSP  = 0x6107,
	SYS_OSAL_NV_DELETE          = 0x2112,
	SYS_OSAL_NV_DELETE_SRSP     = 0x6112,
	SYS_OSAL_NV_LENGTH          = 0x2113,
	SYS_OSAL_NV_LENGTH_SRSP     = 0x6113,
	SYS_ADC_READ                = 0x210D,
	SYS_ADC_READ_SRSP           = 0x610D,
	SYS_GPIO                    = 0x210E,
	SYS_GPIO_SRSP               = 0x610E,
	SYS_RANDOM                  = 0x210C,
	SYS_RANDOM_SRSP             = 0x610C,
	SYS_SET_TIME                = 0x2110,
	SYS_SET_TIME_SRSP           = 0x6110,
	SYS_GET_TIME                = 0x2111,
	SYS_GET_TIME_SRSP           = 0x6111,
	SYS_SET_TX_POWER            = 0x2114,
	SYS_SET_TX_POWER_SRSP       = 0x6114,

	/* Configuration Interface */
	ZB_READ_CONFIGURATION       = 0x2604,
	ZB_READ_CONFIGURATION_SRSP  = 0x6604,
	ZB_WRITE_CONFIGURATION      = 0x2605,
	ZB_WRITE_CONFIGURATION_SRSP = 0x6605,

	/* Simple API interface */
	ZB_APP_REGISTER_REQUEST     = 0x260A,
	ZB_APP_REGISTER_REQUEST_SRSP = 0x660A,
	ZB_START_REQUEST            = 0x2600,
	ZB_START_REQUEST_SRSP       = 0x6600,
	ZB_START_CONFIRM            = 0x4680,
	ZB_PERMIT_JOINING_REQUEST   = 0x2608,
	ZB_PERMIT_JOINING_REQUEST_SRSP = 0x6608,
	ZB_BIND_DEVICE              = 0x2601,
	ZB_BIND_DEVICE_SRSP         = 0x6601,
	ZB_BIND_CONFIRM             = 0x4681,
	ZB_ALLOW_BIND               = 0x2602,
	ZB_ALLOW_BIND_SRSP          = 0x6602,
	ZB_ALLOW_BIND_CONFIRM       = 0x4682,
	ZB_SEND_DATA_REQUEST        = 0x2603,
	ZB_SEND_DATA_REQUEST_SRSP   = 0x6603,
	ZB_SEND_DATA_CONFIRM        = 0x4683,
	ZB_RECEIVE_DATA_INDICATION  = 0x4687,
	ZB_GET_DEVICE_INFO          = 0x2606,
	ZB_GET_DEVICE_INFO_SRSP     = 0x6606,
	ZB_FIND_DEVICE_REQUEST      = 0x2607,
	ZB_FIND_DEVICE_REQUEST_SRSP = 0x6607,
	ZB_FIND_DEVICE_CONFIRM      = 0x4685,
};

typedef struct {
	uint8_t resetReason;
	uint8_t transportRev; // transport protocol revision
	uint8_t productID;
	uint16_t releaseNum; // release number
	uint8_t hwRev; // hardware revision
} revID_s;
public:
	CC2530(GPIO* rstPin, GPIO* srdyPin, GPIO* mrdyPin, SPI* spi,GPIO* ssPin);

	bool init(void);
	void Poll(void);
	bool SREQ(uint16_t cmd, uint8_t *txPtr, uint8_t len);

	bool reset(void);
	bool checkNewMes(void);
	bool isZNPInit(){ return this->isInit;}
	uint16_t getRetCMD(void){return this->retCMD;}
	uint8_t  getRetLen(void){return this->retLen;}
	uint8_t* getRetData(void){return this->retData;}
private:
	GPIO* rstPin;
	GPIO* srdyPin;
	GPIO* mrdyPin;
	GPIO* ssPin;
	SPI*  spi;

	uint16_t retCMD;
	uint8_t  retLen;
	uint8_t	 retData[100];

	revID_s	 revID;
	bool isInit;
};

} /* hv_driver namespace*/

#endif /* CC2530_H */