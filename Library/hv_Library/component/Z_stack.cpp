/**
  ******************************************************************************
 * @file    Z_stack.cpp
 * @author  Hoang Viet  <hoangtheviet93@gmail.com>
 * @version 1.0
 * @date    12-5-2015
 * @brief   TI Z-Stack simple API
  */
//-------------------------------------------------------------------------
#include "Z_stack.h"
#include "string.h"

namespace hv_driver {
	
Z_stack::Z_stack(CC2530 *znp){
	this->znp = znp;
}

bool Z_stack::init(START_OPT startOpt, LOGICAL_TYPE logicalType, uint16_t panID){
	bool retVal = true;
	
	if(this->znp->isZNPInit() != true){
		retVal = this->znp->init();
	}
	if(retVal == false){
		return retVal;
	}
	//this->writeConf(ZCD_NV_startupOption, startOpt);
	this->writeConf(ZCD_NV_logicalType, logicalType);
	this->writeConf(ZCD_NV_panID, panID);
	this->writeConf(ZCD_NV_chanList, ZCD_chanList_allChannels);
	return retVal;
}

Z_stack::STATUS Z_stack::writeConf(CONF_ID confID, conf_s &conf){
	uint8_t data[20] ;
	uint8_t dataLen = 2 + conf.Len;

	data[0] = confID;
	data[1] = conf.Len;
	memcpy(data + 2, conf.Buffer, conf.Len);

	/* send SREQ command	*/
	znp->SREQ(CC2530::ZB_WRITE_CONFIGURATION, data, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = znp->getRetCMD();
	uint8_t  retLen = znp->getRetLen();
	uint8_t* retData = znp->getRetData();

	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_WRITE_CONFIGURATION_SRSP && retLen == 1) {
		return (STATUS)retData[0];
	}
	return ZFailure;	
}

Z_stack::STATUS Z_stack::writeConf(CONF_ID confID, uint32_t confVal) {
	conf_s	conf;
	STATUS retVal;

	switch (confID) {
  /* confID with 1 byte in confValue */
  case ZCD_NV_startupOption:
  case ZCD_NV_logicalType:
  case ZCD_NV_ZDODirectCB:
  case ZCD_NV_pollFailureRetries:
  case ZCD_NV_indirectMsgTimeout:
  case ZCD_NV_APSFrameRetries:
  case ZCD_NV_preCfgKeyEnable:
  case ZCD_NV_securityMode:
  case ZCD_NV_useDefaultTCLK:
  case ZCD_NV_passiveAckTimeout:
  case ZCD_NV_bcastDeliveryTime:
  case ZCD_NV_routeExpiryTime:
  	conf.Len = 1;
  	conf.Buffer[0] = (uint8_t)confVal;
  	break;
  /* confID with 2 bytes in confValue */
  case ZCD_NV_pollRate:
  case ZCD_NV_queuedPollRate:
  case ZCD_NV_responsePollRate:
  case ZCD_NV_APSAckWaitDuration:
  case ZCD_NV_bindingTime:
  case ZCD_NV_panID:
  case ZCD_NV_bcastRetries:
  	conf.Len = 2;
  	conf.Buffer[0] = (uint8_t)confVal;
  	conf.Buffer[1] = (uint8_t)(confVal >> 8);
  	break;
  case ZCD_NV_chanList:
  	conf.Len = 4;
  	conf.Buffer[0] = (uint8_t)confVal;
  	conf.Buffer[1] = (uint8_t)(confVal >> 8);
  	conf.Buffer[2] = (uint8_t)(confVal >> 16);
  	conf.Buffer[3] = (uint8_t)(confVal >> 24);
  	break;
  default:
  	return ZFailure;
	}

	retVal = writeConf(confID, conf);
	return retVal;
}

Z_stack::STATUS Z_stack::readConf(CONF_ID confID, conf_s &conf) {
	uint8_t data[1] = {confID};
	uint8_t dataLen = 1;
	STATUS retVal = ZFailure;

	/*	send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_READ_CONFIGURATION, data, dataLen);

	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();
	uint8_t* retData_p = this->znp->getRetData();
	retVal = (STATUS)retData_p[0];

	if (retCMD == CC2530::ZB_READ_CONFIGURATION_SRSP && retVal == ZSuccess && retData_p[1] == confID) {
		conf.Len = retData_p[2];
		memcpy(conf.Buffer, retData_p + 3, conf.Len);
	}
	return retVal;
}

Z_stack::STATUS Z_stack::readConf(CONF_ID confID, uint32_t &retConfVal) {
	STATUS retVal = ZFailure;
	conf_s readConf;

	retVal = this->readConf(confID, readConf); // read config struct

	if (retVal != ZSuccess) {
		return retVal;
	}

	switch (readConf.Len) {
	case 1:
	case 2:
	case 4:
		retConfVal = 0; // convert to 32bit value
		for (uint8_t index = readConf.Len; index > 0; index--) {
			retConfVal = retConfVal << 8;
			retConfVal += readConf.Buffer[index - 1];
		}
		break;
	default:
		return ZFailure;
	}
	return retVal;
}

Z_stack::STATUS Z_stack::appReg(AppReg_s &AppReg) {
	STATUS retVal = ZFailure;
	uint8_t* data_p = new uint8_t [9 + 2*(AppReg.inputCmdNum + AppReg.outputCmdNum)];
	uint8_t dataLen = 9 + 2*(AppReg.inputCmdNum + AppReg.outputCmdNum);

	data_p[0] = AppReg.appEndPoint;
	data_p[1] = (uint8_t)AppReg.appProfileID;
	data_p[2] = (uint8_t)(AppReg.appProfileID >> 8);
	data_p[3] = (uint8_t)AppReg.deviceID;
	data_p[4] = (uint8_t)(AppReg.deviceID >> 8);
	data_p[5] = AppReg.deviceVersion;
	data_p[6] = 0xFF; // unused byte

	/*	copy 16bit input cmdID	*/
	data_p[7] = AppReg.inputCmdNum;
	for (uint8_t index = 0; index < AppReg.inputCmdNum; index++) {
		data_p[8 + 2 * index] = (uint8_t)AppReg.inputCmd[index];
		data_p[8 + 2 * index + 1] = (uint8_t)(AppReg.inputCmd[index] >> 8);
	}
	/*	copy 16bit output cmdID*/
	data_p[8 + 2 * AppReg.inputCmdNum] = AppReg.outputCmdNum;
	for (uint8_t index = 0; index < AppReg.inputCmdNum; index++) {
		data_p[9 + 2 * AppReg.inputCmdNum + 2 * index] = (uint8_t)AppReg.inputCmd[index];
		data_p[9 + 2 * AppReg.inputCmdNum + 2 * index + 1] = (uint8_t)(AppReg.inputCmd[index] >> 8);
	}

	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_APP_REGISTER_REQUEST, data_p, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();
	uint8_t  retLen = this->znp->getRetLen();
	uint8_t* retData = this->znp->getRetData();

	delete [] data_p;

	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_APP_REGISTER_REQUEST_SRSP && retLen == 1) {
		retVal = (STATUS)retData[0];
	}
	return retVal;
}

Z_stack::STATUS Z_stack::startReq(void) {
	STATUS retVal = ZFailure;

	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_START_REQUEST, NULL, 0);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();

	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_START_REQUEST_SRSP ) {
		retVal = ZSuccess;
	}
	return retVal;
}

Z_stack::STATUS Z_stack::permitJoinReq(uint16_t destAddr, uint8_t timeOut) {
	STATUS retVal = ZFailure;
	uint8_t data[3];
	uint8_t dataLen = 3;

	data[0] = (uint8_t)destAddr;
	data[1] = (uint8_t)(destAddr >> 8);
	data[2] = timeOut;

	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_PERMIT_JOINING_REQUEST, data, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();
	uint8_t  retLen = this->znp->getRetLen();
	uint8_t* retData = this->znp->getRetData();

	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_PERMIT_JOINING_REQUEST_SRSP && retLen == 1) {
		retVal = (STATUS)retData[0];
	}
	return retVal;
}


Z_stack::STATUS Z_stack::bindDevice(bool create, uint16_t cmdID, uint8_t* ieeeAddr) {
	STATUS retVal = ZFailure;
	uint8_t data[11];
	uint8_t dataLen = 11;

	data[0] = create;
	data[1] = (uint8_t)cmdID;
	data[2] = (uint8_t)(cmdID >> 8);
	for (uint8_t index; index < 8; index++) {
		if (ieeeAddr != NULL) {
			data[3 + index] = ieeeAddr[index];
		} else {
			data[3 + index] = 0;
		}
	}

	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_BIND_DEVICE, data, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();
	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_BIND_DEVICE_SRSP) {
		retVal = ZSuccess;
	}
	return retVal;
}

Z_stack::STATUS Z_stack::allowBind(uint8_t timeOut) {
	STATUS retVal = ZFailure;
	uint8_t data[1];
	uint8_t dataLen = 1;

	data[0] = timeOut;

	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_ALLOW_BIND, data, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();
	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_ALLOW_BIND_SRSP) {
		retVal = ZSuccess;
	}
	return retVal;
}


Z_stack::STATUS Z_stack::sendDataReq(TxPacket_s &txPacket, bool ack, uint8_t radius) {
	STATUS retVal = ZFailure;
	uint8_t data_p[100];
	uint8_t dataLen = 8 + txPacket.len;

	if(txPacket.txPtr == NULL) {	// TxBuf_p must point to static memory
		return retVal;
	}

	data_p[0] = (uint8_t)txPacket.dstAddr;
	data_p[1] = (uint8_t)(txPacket.dstAddr >> 8);
	data_p[2] = (uint8_t)txPacket.cmdID;
	data_p[3] = (uint8_t)(txPacket.cmdID >> 8);
	data_p[4] = txPacket.handle;
	data_p[5] = ack;
	data_p[6] = radius;
	data_p[7] = txPacket.len;
	for (uint8_t index = 0; index < txPacket.len; index++) {
		data_p[8 + index] = txPacket.txPtr[index];
	}
	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_SEND_DATA_REQUEST, data_p, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();
	uint8_t  retLen = this->znp->getRetLen();

	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_SEND_DATA_REQUEST_SRSP && retLen == 0) {
		retVal = ZSuccess;
	}
	return retVal;
}

Z_stack::STATUS Z_stack::getDeviceInfo(DeviceInfo_s &deviceinfo) {
	STATUS retVal = ZFailure;
	uint8_t data[1] = {deviceinfo.infoParam};
	uint8_t dataLen = 1;

	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_GET_DEVICE_INFO, data, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();
	uint8_t  retLen = this->znp->getRetLen();
	uint8_t *retData = this->znp->getRetData();

	/* check SRSP message	*/
	if (retCMD == CC2530::ZB_GET_DEVICE_INFO_SRSP && retData[0] == deviceinfo.infoParam) {
		retVal = ZSuccess;

		switch (deviceinfo.infoParam) {
    case Zb_deviceState:
    case Zb_workingChannel:
    	deviceinfo.len = 0x01;
        break;
    case Zb_deviceIEEEAddr:
    case Zb_parentIEEEAddr:
    case Zb_extPanID:
    	deviceinfo.len = 0x08;
        break;
    case Zb_deviceShortAddr:
    case Zb_parentShortAddr:
    case Zb_panID:
    	deviceinfo.len = 0x02;
        break;
    default:
        break;
		}
		memcpy(deviceinfo.value, retData +1, deviceinfo.len);
	}
	return retVal;
}

Z_stack::STATUS Z_stack::findDeviceReq(uint8_t* ieeeAddr) {
	STATUS retVal = ZFailure;
	uint8_t data[8] = {0};
	uint8_t dataLen = 8;

	/* copy 64bit ieee address to search	*/
	memcpy(data, ieeeAddr, 8);
	/*	Send SREQ command	*/
	this->znp->SREQ(CC2530::ZB_FIND_DEVICE_REQUEST, data, dataLen);
	/* get SRSP message	*/
	uint16_t retCMD = this->znp->getRetCMD();

	if (retCMD == CC2530::ZB_FIND_DEVICE_REQUEST_SRSP) {
		retVal = ZSuccess;
	}
	return retVal;
}

Z_stack::CALLBACK_TYPE Z_stack::getCallBack(void) {
	CALLBACK_TYPE retCallBack = Zb_noCallback;

	/*	check new message	*/
	if (znp->checkNewMes() == true) {
		/* get callback data*/
		uint16_t retCMD = this->znp->getRetCMD();
		uint8_t  retLen = this->znp->getRetLen();
		uint8_t *retData = this->znp->getRetData();

		switch (retCMD) {
    case Zb_startConfirm:
    case Zb_stateChangeInd:
    case Zb_bindConfirm:
    case Zb_allowBindConfirm:
    case Zb_sendDataConfirm:
    case Zb_receiveDataIndication:
    case Zb_findDeviceConfirm:
    	retCallBack = (CALLBACK_TYPE)retCMD;
    	this->lastCallBack = retCallBack;
    	this->callBackLen = retLen;
    	memcpy(this->callBackData, retData, this->callBackLen);
    	break;
    default:
    	retCallBack = Zb_otherCallBack;
    	break;
		}
	}
	return retCallBack;
}

Z_stack::START_COMFIRM_STATUS	Z_stack::startComfirm(void) {
	return (START_COMFIRM_STATUS)this->callBackData[0];
}

Z_stack::STATE_CHANGE Z_stack::stateChangeInd(void){
    return (STATE_CHANGE)this->callBackData[0];
}

Z_stack::STATUS	Z_stack::bindComfirm(uint16_t &bindCmdID) {
	bindCmdID = (uint16_t)(this->callBackData[1] << 8) + this->callBackData[0];
	return (STATUS)this->callBackData[2];
}

Z_stack::STATUS Z_stack::allowBindComfirm(uint16_t &srcAddr) {
	srcAddr = (uint16_t)(this->callBackData[1] << 8) + this->callBackData[0];
}

Z_stack::STATUS Z_stack::sendDataComfirm(TxPacket_s &txPacket) {
	STATUS retVal = ZFailure;
	if (txPacket.handle == this->callBackData[0]) {
		retVal = (STATUS)this->callBackData[1];
	}
	return retVal;
}

void Z_stack::receiveDataIndication(RxPacket_s &rxPacket) {
	rxPacket.srcAddr = (uint16_t)(this->callBackData[1] << 8) + this->callBackData[0];
	rxPacket.cmdID = (uint16_t)(this->callBackData[3] << 8) + this->callBackData[2];
	rxPacket.len = (uint16_t)(this->callBackData[5] << 8) + this->callBackData[4];
	rxPacket.rxPtr = this->callBackData + 5;
}

void Z_stack::findDeviceComfirm(uint8_t *&ieeeAddr,uint16_t& resultAddr) {
	ieeeAddr = this->callBackData + 3;
	resultAddr =  (uint16_t)(this->callBackData[2] << 8) + this->callBackData[1];
}

} /* hv_driver namespace */