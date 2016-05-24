/**
  ******************************************************************************
 * @file    Z_stack.h
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    12-5-2015
 * @brief   TI Z-Stack simple API
  */
//-------------------------------------------------------------------------

#ifndef Z_STACK_H
#define Z_STACK_H

#include "CC2530.h"

namespace hv_driver {
class Z_stack {	
public:
	/**< ZbStack status	*/
enum STATUS {
  ZSuccess = 0x00,
  ZFailure = 0x01,
  ZInvalidParameter = 0x02,
	NV_ITEM_UNINIT = 0x09,
  NV_OPER_FAILED = 0x0a,
  NV_BAD_ITEM_LEN = 0x0c,
  ZMemError = 0x10,
  ZBufferFull = 0x11,
  ZUnsupportedMode = 0x12,
  ZMacMemError = 0x13,
  zdoInvalidEndpoint = 0x82,
  zdoUnsupported = 0x84,
  zdoTimeout = 0x85,
  zdoNoMatch = 0x86,
  zdoTableFull = 0x87,
  zdoNoBindEntry = 0x88,
  ZSecNoKey = 0xa1,
  ZSecMaxFrmCount = 0xa3,
  ZApsFail = 0xb1,
  ZApsTableFull = 0xb2,
  ZApsIllegalRequest = 0xb3,
  ZApsInvalidBinding = 0xb4,
  ZApsUnsupportedAttrib = 0xb5,
  ZApsNotSupported = 0xb6,
  ZApsNoAck = 0xb7,
  ZApsDuplicateEntry = 0xb8,
  ZApsNoBoundDevice = 0xb9,
  ZNwkInvalidParam = 0xc1,
  ZNwkInvalidRequest = 0xc2,
  ZNwkNotPermitted = 0xc3,
  ZNwkStartupFailure = 0xc4,
  ZNwkTableFull = 0xc7,
  ZNwkUnknownDevice = 0xc8,
  ZNwkUnsupportedAttribute = 0xc9,
  ZNwkNoNetwork = 0xca,
  ZNwkLeaveUnconfirmed = 0xcb,
  ZNwkNoAck = 0xcc,
  ZNwkNoRoute = 0xcd,
  ZMacNoACK = 0xe9
};

/*	config command	ID	*/
enum CONF_ID {
	ZCD_NV_startupOption = 0x03,
	ZCD_NV_logicalType = 0x87,
	ZCD_NV_ZDODirectCB = 0x8F,
	ZCD_NV_pollRate = 0x24,
	ZCD_NV_queuedPollRate = 0x25,
  ZCD_NV_responsePollRate = 0x26,
  ZCD_NV_pollFailureRetries = 0x29,
  ZCD_NV_indirectMsgTimeout = 0x2B,
  ZCD_NV_APSFrameRetries = 0x43,
  ZCD_NV_APSAckWaitDuration = 0x44,
  ZCD_NV_bindingTime = 0x46,
  ZCD_NV_userDesc = 0x81,
  ZCD_NV_panID = 0x83,
  ZCD_NV_chanList = 0x84,
  ZCD_NV_preCfgKey = 0x62,
  ZCD_NV_preCfgKeyEnable = 0x63,
  ZCD_NV_securityMode = 0x64,
  ZCD_NV_useDefaultTCLK = 0x6D,
  ZCD_NV_bcastRetries = 0x2E,
  ZCD_NV_passiveAckTimeout = 0x2F,
  ZCD_NV_bcastDeliveryTime = 0x30,
  ZCD_NV_routeExpiryTime = 0x2C
};

enum START_OPT {
	ZCD_startOpt_clearAll = 0x03,
  ZCD_startOpt_clearState = 0x02,
  ZCD_startOpt_clearConfig = 0x01,
  ZCD_startOpt_noClear = 0x00
};

/*	ZNP device logic type	*/
enum LOGICAL_TYPE  {
  ZCD_coordinator = 0x00,
  ZCD_router = 0x01,
  ZCD_endDevice = 0x02,
};

/*	RF channel list	*/
enum RF_CHANNEL {
  ZCD_chanList_11 = 0x00000800,
  ZCD_chanList_12 = 0x00001000,
  ZCD_chanList_13 = 0x00002000,
  ZCD_chanList_14 = 0x00004000,
  ZCD_chanList_15 = 0x00008000,
  ZCD_chanList_16 = 0x00010000,
  ZCD_chanList_17 = 0x00020000,
  ZCD_chanList_18 = 0x00040000,
  ZCD_chanList_19 = 0x00080000,
  ZCD_chanList_20 = 0x00100000,
  ZCD_chanList_21 = 0x00200000,
  ZCD_chanList_22 = 0x00400000,
  ZCD_chanList_23 = 0x00800000,
  ZCD_chanList_24 = 0x01000000,
  ZCD_chanList_25 = 0x02000000,
  ZCD_chanList_26 = 0x04000000,
  ZCD_chanList_allChannels = 0x07FFF800,
  ZCD_chanList_none = 0x00000000
};

/*	for get Device info command	*/
enum DEVICE_INFO {
    Zb_deviceState = 0x00,
    Zb_deviceIEEEAddr = 0x01,
    Zb_deviceShortAddr = 0x02,
    Zb_parentShortAddr = 0x03,
    Zb_parentIEEEAddr = 0x04,
    Zb_workingChannel = 0x05,
    Zb_panID = 0x06,
    Zb_extPanID = 0x07
};

/**< for callbacks type	*/
enum CALLBACK_TYPE {
	Zb_noCallback = 0x0000,
	Zb_startConfirm = 0x4680,
	Zb_stateChangeInd = 0x45C0,
	Zb_bindConfirm = 0x4681,
	Zb_allowBindConfirm = 0x4682,
	Zb_sendDataConfirm = 0x4683,
	Zb_receiveDataIndication = 0x4687,
	Zb_findDeviceConfirm = 0x4685,
	Zb_otherCallBack = 0xFFFF
};

/*	for start comfirm callback	*/
enum START_COMFIRM_STATUS {
    Zb_success = 0x00,
    Zb_init = 0x22,
};

/*	for state change index callback	*/
enum STATE_CHANGE {
    Zb_devHold = 0x00,
    Zb_devInit = 0x01,
    Zb_devNWKDisc = 0x02,
    Zb_devNWKJoining = 0x03,
    Zb_devNWKRejoin = 0x04,
    Zb_devEndDeviceUnauth = 0x05,
    Zb_devEndDevice = 0x06,
    Zb_devRouter = 0x07,
    Zb_devCoordStarting = 0x08,
    Zb_devZBCoord = 0x09,
    Zb_devNWKOrphan = 0x0A,
};

typedef struct {
	uint8_t Len;
	uint8_t Buffer[10];
} conf_s;

typedef struct {
	uint8_t  appEndPoint;
	uint16_t appProfileID;
	uint16_t deviceID;
	uint8_t  deviceVersion;
	uint8_t  inputCmdNum;
	uint16_t  inputCmd[10];
	uint8_t  outputCmdNum;
	uint16_t  outputCmd[10];
} AppReg_s;

typedef struct {
	uint16_t dstAddr;
	uint16_t cmdID;
	uint8_t handle;
	uint8_t  len;
	uint8_t* txPtr;
} TxPacket_s;

typedef struct {
	uint16_t srcAddr;
	uint16_t cmdID;
	uint16_t len;
	uint8_t* rxPtr;
} RxPacket_s;

typedef struct {
	DEVICE_INFO infoParam;

	uint8_t	len;
	uint8_t value[8];
} DeviceInfo_s;

public:
	Z_stack(CC2530* znp);
	
	bool init(START_OPT startOpt, LOGICAL_TYPE logicalType, uint16_t panID);
	STATUS writeConf(CONF_ID confID, conf_s &conf);
	STATUS writeConf(CONF_ID confID, uint32_t confVal);
	STATUS readConf(CONF_ID confID, conf_s &conf);
	STATUS readConf(CONF_ID confID, uint32_t &retConfVal);

	STATUS appReg(AppReg_s &AppReg);
	STATUS startReq(void);
	STATUS permitJoinReq(uint16_t dstAddr, uint8_t timeOut);
	STATUS bindDevice(bool create, uint16_t cmdID, uint8_t* ieeeAddr);
	STATUS allowBind(uint8_t timeOut);
	STATUS sendDataReq(TxPacket_s &txPacket, bool ack, uint8_t radius);
	STATUS getDeviceInfo(DeviceInfo_s &deviceinfo);
	STATUS findDeviceReq(uint8_t* ieeeAddr);

	CALLBACK_TYPE 				getCallBack(void);
	START_COMFIRM_STATUS	startComfirm(void);
	STATE_CHANGE 					stateChangeInd (void);
								 STATUS bindComfirm(uint16_t &bindCmdID);
								 STATUS allowBindComfirm(uint16_t &srcAddr);
								 STATUS sendDataComfirm(TxPacket_s &txPacket);
									 void receiveDataIndication(RxPacket_s &rxPacket);
									 void findDeviceComfirm(uint8_t *&ieeeAddr,uint16_t &resultAddr);
private:	
	CC2530* znp;
	CALLBACK_TYPE lastCallBack;
	uint8_t callBackLen;
	uint8_t callBackData[100];
};	
} /* hv_driver namespace */

#endif /* Z_STACK_H*/
