#include "mdspi.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <string.h>
#include "servant/Application.h"
#include "util/tc_logger.h"
#include <iomanip>
#include "MysqlUtil.h"
#include "util/tc_common.h"
#include "stdio.h"
#include "util/tc_encoder.h"
#include "GR.h"

using namespace std;

int m_requestId = 0;  

void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
    int nRequestID, 
    bool bIsLast)
{
    IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
    TLOGDEBUG(TC_Encoder::gbk2utf8(" 响应 | 行情连接中断...") << " reason=" << nReason << endl);
}
		
void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
    TLOGDEBUG(TC_Encoder::gbk2utf8(" 响应 | 行情心跳超时警告...") << " TimerLapse = " << nTimeLapse << endl);
}

void CtpMdSpi::OnFrontConnected()
{
	TLOGDEBUG(TC_Encoder::gbk2utf8(" 连接行情前置...成功") << endl);
    TThostFtdcBrokerIDType	appId;
    TThostFtdcUserIDType	userId;	
    TThostFtdcPasswordType	passwd;
    strcpy(appId,AG->getAppId().c_str());
    strcpy(userId,AG->getUserId().c_str());
    strcpy(passwd,AG->getPassWd().c_str());
	ReqUserLogin(appId, userId, passwd);
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId,
    TThostFtdcUserIDType	userId,	
    TThostFtdcPasswordType	passwd)
{
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, appId);
    strcpy(req.UserID, userId);
    strcpy(req.Password, passwd);
    int ret = pUserApi->ReqUserLogin(&req, ++m_requestId);
    TLOGDEBUG(TC_Encoder::gbk2utf8(" 请求 | 发送行情登录...")<<((ret == 0) ? TC_Encoder::gbk2utf8("成功") :TC_Encoder::gbk2utf8("失败")) << endl);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
    if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
    {
        TLOGDEBUG(TC_Encoder::gbk2utf8(" 响应 | 行情登录成功...当前交易日:")<<pRspUserLogin->TradingDay<<endl);
        string sResult = AG->getUserInstrumentList();
        FDLOG("mdspi")<<"sResult:"<<sResult<<endl;
        char instIdList[5000];
        strcpy(instIdList,sResult.c_str());
        SubscribeMarketData(instIdList);
    }
}

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
    vector<char*> list;
    char *token = strtok(instIdList, ",");
    while( token != NULL )
    {
        list.push_back(token); 
        token = strtok(NULL, ",");
    }
    unsigned int len = list.size();
    char** pInstId = new char* [len];  
    for(unsigned int i=0; i<len;i++) 
    {
        pInstId[i]=list[i]; 
    }
    int ret=pUserApi->SubscribeMarketData(pInstId, len);
    TLOGDEBUG(TC_Encoder::gbk2utf8(" 请求 | 发送行情订阅... ")<<((ret == 0) ? TC_Encoder::gbk2utf8("成功") : TC_Encoder::gbk2utf8("失败"))<< endl);
}

void CtpMdSpi::OnRspSubMarketData(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
	TLOGDEBUG(TC_Encoder::gbk2utf8(" 响应 |  行情订阅...成功")<< endl);
}

void CtpMdSpi::OnRspUnSubMarketData(
    CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
	TLOGDEBUG(TC_Encoder::gbk2utf8(" 响应 |  行情取消订阅...成功")<< endl);
}

void CtpMdSpi::OnRtnDepthMarketData(
    CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    string sTradingDay = pDepthMarketData->TradingDay;
    string sUpdateTime = pDepthMarketData->UpdateTime;
    int sUpdateMillisec = pDepthMarketData->UpdateMillisec;
	string sInstrumentID = pDepthMarketData->InstrumentID;
	double sLastPrice = pDepthMarketData->LastPrice > 10000000000 ? 0 : pDepthMarketData->LastPrice;
	double sHighestPrice = pDepthMarketData->HighestPrice > 10000000000 ? 0 : pDepthMarketData->HighestPrice;
	double sLowestPrice = pDepthMarketData->LowestPrice > 10000000000 ? 0 : pDepthMarketData->LowestPrice;

    
    double PreSettlementPrice = pDepthMarketData->PreSettlementPrice > 10000000000 ? 0 : pDepthMarketData->PreSettlementPrice;
    double PreClosePrice = pDepthMarketData->PreClosePrice > 10000000000 ? 0 : pDepthMarketData->PreClosePrice;
    double PreOpenInterest = pDepthMarketData->PreOpenInterest > 10000000000 ? 0 : pDepthMarketData->PreOpenInterest;
    double OpenPrice = pDepthMarketData->OpenPrice > 10000000000 ? 0 : pDepthMarketData->OpenPrice;
    int Volume = pDepthMarketData->Volume;
    double Turnover = pDepthMarketData->Turnover > 10000000000 ? 0 : pDepthMarketData->Turnover;
    double sOpenInterest = pDepthMarketData->OpenInterest > 10000000000 ? 0 : pDepthMarketData->OpenInterest;
    double ClosePrice = pDepthMarketData->ClosePrice > 10000000000 ? 0 : pDepthMarketData->ClosePrice;
    double SettlementPrice = pDepthMarketData->SettlementPrice > 10000000000 ? 0 : pDepthMarketData->SettlementPrice;
    double UpperLimitPrice = pDepthMarketData->UpperLimitPrice > 10000000000 ? 0 : pDepthMarketData->UpperLimitPrice;
    double LowerLimitPrice = pDepthMarketData->LowerLimitPrice > 10000000000 ? 0 : pDepthMarketData->LowerLimitPrice;
    double AveragePrice = pDepthMarketData->AveragePrice > 10000000000 ? 0 : pDepthMarketData->AveragePrice;
    double PreDelta = pDepthMarketData->PreDelta > 10000000000 ? 0 : pDepthMarketData->PreDelta;
    double CurrDelta = pDepthMarketData->CurrDelta > 10000000000 ? 0 : pDepthMarketData->CurrDelta;
	double sAskPrice1 = pDepthMarketData->AskPrice1 > 10000000000 ? 0 : pDepthMarketData->AskPrice1;
	double sAskVolume1 = pDepthMarketData->AskVolume1 > 10000000000 ? 0 : pDepthMarketData->AskVolume1;
	double sBidPrice1 = pDepthMarketData->BidPrice1 > 10000000000 ? 0 : pDepthMarketData->BidPrice1;
	double sBidVolume1 = pDepthMarketData->BidVolume1 > 10000000000 ? 0 : pDepthMarketData->BidVolume1;
	double sAskPrice2 = pDepthMarketData->AskPrice2 > 10000000000 ? 0 : pDepthMarketData->AskPrice2;
	double sAskVolume2 = pDepthMarketData->AskVolume2 > 10000000000 ? 0 : pDepthMarketData->AskVolume2;
	double sBidPrice2 = pDepthMarketData->BidPrice2 > 10000000000 ? 0 : pDepthMarketData->BidPrice2;
	double sBidVolume2 = pDepthMarketData->BidVolume2 > 10000000000 ? 0 : pDepthMarketData->BidVolume2;
	double sAskPrice3 = pDepthMarketData->AskPrice3 > 10000000000 ? 0 : pDepthMarketData->AskPrice3;
	double sAskVolume3 = pDepthMarketData->AskVolume3 > 10000000000 ? 0 : pDepthMarketData->AskVolume3;
	double sBidPrice3 = pDepthMarketData->BidPrice3 > 10000000000 ? 0 : pDepthMarketData->BidPrice3;
	double sBidVolume3 = pDepthMarketData->BidVolume3 > 10000000000 ? 0 : pDepthMarketData->BidVolume3;
	double sAskPrice4 = pDepthMarketData->AskPrice4 > 10000000000 ? 0 : pDepthMarketData->AskPrice4;
	double sAskVolume4 = pDepthMarketData->AskVolume4 > 10000000000 ? 0 : pDepthMarketData->AskVolume4;
	double sBidPrice4 = pDepthMarketData->BidPrice4 > 10000000000 ? 0 : pDepthMarketData->BidPrice4;
	double sBidVolume4 = pDepthMarketData->BidVolume4 > 10000000000 ? 0 : pDepthMarketData->BidVolume4;
	double sAskPrice5 = pDepthMarketData->AskPrice5 > 10000000000 ? 0 : pDepthMarketData->AskPrice5;
	double sAskVolume5 = pDepthMarketData->AskVolume5 > 10000000000 ? 0 : pDepthMarketData->AskVolume5;
	double sBidPrice5 = pDepthMarketData->BidPrice5 > 10000000000 ? 0 : pDepthMarketData->BidPrice5;
	double sBidVolume5 = pDepthMarketData->BidVolume5 > 10000000000 ? 0 : pDepthMarketData->BidVolume5;
    
    stringstream sTime;
    string hrminsec = TC_Common::replace(sUpdateTime, ":", "");
    if(hrminsec.size() <= 4)
    {
        FDLOG("hq_error")<< "step1--sTradingDay:" << sTradingDay
        <<"|" << "sInstrumentID:" << sInstrumentID
        <<"|" << "sUpdateTime:" << sUpdateTime
        << endl;
        return;
    }
    string hrmin = hrminsec.substr(0,4);
    string nowhrmin = TC_Common::now2str("%H%M");
    if(((nowhrmin >="0900" && nowhrmin<="1015")
        || (nowhrmin >="1030" && nowhrmin<="1500")
        || (nowhrmin >="2100" && nowhrmin<="2330")
        )&& hrmin == nowhrmin)
    {
        sTime << sTradingDay << hrminsec;
        if(sUpdateMillisec < 10)
        {
            sTime << "00" << sUpdateMillisec;
        }
        else if(sUpdateMillisec < 100)
        {
            sTime << "0" << sUpdateMillisec;
        }
        else
        {
            sTime << sUpdateMillisec;
        }
        string nowTime = TC_Common::now2str("%Y%m%d") + sTime.str().substr(8);
        FDLOG("hq")<< "sTradingDay:" << sTradingDay
            <<"|" << "sInstrumentID:" << sInstrumentID
            <<"|" << "sLastPrice:" << TC_Common::tostr<double>(sLastPrice)
            <<"|" << "sHighestPrice:" << TC_Common::tostr<double>(sHighestPrice)
            <<"|" << "sLowestPrice:" << TC_Common::tostr<double>(sLowestPrice)
            <<"|" << "sAskPrice1:" << TC_Common::tostr<double>(sAskPrice1)
            <<"|" << "sAskVolume1:" << TC_Common::tostr<double>(sAskVolume1) 
            <<"|" << "sBidPrice1:" << TC_Common::tostr<double>(sBidPrice1)
            <<"|" << "sBidVolume1:" << TC_Common::tostr<double>(sBidVolume1)
            <<"|" << "sOpenInterest:" << TC_Common::tostr<double>(sOpenInterest)
            <<"|" << "ClosePrice:" << TC_Common::tostr<double>(ClosePrice)
            <<"|" << "SettlementPrice:" << TC_Common::tostr<double>(SettlementPrice)
            <<"|" << "UpperLimitPrice:" << TC_Common::tostr<double>(UpperLimitPrice)
            <<"|" << "LowerLimitPrice:" << TC_Common::tostr<double>(LowerLimitPrice)
            <<"|" << "AveragePrice:" << TC_Common::tostr<double>(AveragePrice)
            <<"|" << "PreDelta:" << TC_Common::tostr<double>(PreDelta)
            <<"|" << "CurrDelta:" << TC_Common::tostr<double>(CurrDelta)
            <<"|" << "sAskPrice2:" << TC_Common::tostr<double>(sAskPrice2)
            <<"|" << "sAskVolume2:" << TC_Common::tostr<double>(sAskVolume2) 
            <<"|" << "sBidPrice2:" << TC_Common::tostr<double>(sBidPrice2)
            <<"|" << "sBidVolume2:" << TC_Common::tostr<double>(sBidVolume2)
            <<"|" << "sAskPrice3:" << TC_Common::tostr<double>(sAskPrice3)
            <<"|" << "sAskVolume3:" << TC_Common::tostr<double>(sAskVolume3) 
            <<"|" << "sBidPrice3:" << TC_Common::tostr<double>(sBidPrice3)
            <<"|" << "sBidVolume3:" << TC_Common::tostr<double>(sBidVolume3)
            <<"|" << "sAskPrice4:" << TC_Common::tostr<double>(sAskPrice4)
            <<"|" << "sAskVolume4:" << TC_Common::tostr<double>(sAskVolume4) 
            <<"|" << "sBidPrice4:" << TC_Common::tostr<double>(sBidPrice4)
            <<"|" << "sBidVolume4:" << TC_Common::tostr<double>(sBidVolume4)
            <<"|" << "sAskPrice5:" << TC_Common::tostr<double>(sAskPrice5)
            <<"|" << "sAskVolume5:" << TC_Common::tostr<double>(sAskVolume5) 
            <<"|" << "sBidPrice5:" << TC_Common::tostr<double>(sBidPrice5)
            <<"|" << "sBidVolume5:" << TC_Common::tostr<double>(sBidVolume5)
            <<"|" << "sUpdateTime:" << sUpdateTime
            <<"|" << "sUpdateMillisec:" << sUpdateMillisec
            <<"|" << "nowTime:" << nowTime
            <<"|" << "PreSettlementPrice:" << PreSettlementPrice
            <<endl;
        //monitor 
        AG->setInstrument(sInstrumentID, sLastPrice, nowTime);

        //insert db
        MysqlUtil::getInstance()->insertQT(sTime.str(),
            sInstrumentID,
            sLastPrice,
            sHighestPrice,
            sLowestPrice,
            sAskPrice1,
            sAskVolume1,
            sBidPrice1,
            sBidVolume1,
            sOpenInterest,
            PreSettlementPrice,
            PreClosePrice,
            PreOpenInterest,
            OpenPrice,
            Volume,
            Turnover,
            ClosePrice,
            SettlementPrice,
            UpperLimitPrice,
            LowerLimitPrice,
            AveragePrice,
            PreDelta,
            CurrDelta,
            sAskPrice2,
            sAskVolume2,
            sBidPrice2,
            sBidVolume2,
            sAskPrice3,
            sAskVolume3,
            sBidPrice3,
            sBidVolume3,
            sAskPrice4,
            sAskVolume4,
            sBidPrice4,
            sBidVolume4,
            sAskPrice5,
            sAskVolume5,
            sBidPrice5,
            sBidVolume5,
            nowTime
        );
    }
    else
    {
        FDLOG("hq_error")<< "step2--sTradingDay:" << sTradingDay
        <<"|" << "sInstrumentID:" << sInstrumentID
        <<"|" << "sUpdateTime:" << sUpdateTime
        <<"|" << "hrmin:" << hrmin
        <<"|" << "nowhrmin:" << nowhrmin
        << endl;
    }
    
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
    bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if (ret)
    {
        TLOGDEBUG(TC_Encoder::gbk2utf8(" 响应 | ")<<pRspInfo->ErrorMsg<<endl);
    }
    return ret;
}
