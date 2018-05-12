#include <iostream>
#include <vector>
#include <cstring>
#include <string.h>
#include "servant/Application.h"
#include "traderspi.h"
#include "stdio.h"
#include "util/tc_encoder.h"
#include "GR.h"


using namespace std;

TThostFtdcBrokerIDType appId;		// 应用单元
TThostFtdcUserIDType userId;		// 投资者代码


int t_requestId = 0; 

// 会话参数
int	 frontId;	//前置编号
int	 sessionId;	//会话编号
char orderRef[13];

vector<CThostFtdcOrderField*> orderList;
vector<CThostFtdcTradeField*> tradeList;

char MapDirection(char src, bool toOrig);
char MapOffset(char src, bool toOrig);

void CtpTraderSpi::OnFrontConnected()
{
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("----连接交易前置...成功")<<endl;
    TThostFtdcBrokerIDType	appId;
    TThostFtdcUserIDType	userId;	
    TThostFtdcPasswordType	passwd;
    strcpy(appId,AG->getAppId().c_str());
    strcpy(userId,AG->getUserId().c_str());
    strcpy(passwd,AG->getPassWd().c_str());
	//ReqUserLogin(appId,userId,passwd);
	ReqAuthenticate();
}

int CtpTraderSpi::ReqAuthenticate()
{
    CThostFtdcReqAuthenticateField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, AG->getAppId().c_str());
    strcpy(req.UserID, AG->getUserId().c_str());
    strcpy(req.UserProductInfo, AG->getUserProductInfo().c_str());
    strcpy(req.AuthCode, AG->getAuthCode().c_str());
    int ret = pUserApi->ReqAuthenticate(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("----请求 | 发送交易认证请求...")<<((ret == 0) ? TC_Encoder::gbk2utf8("成功") :TC_Encoder::gbk2utf8("失败")) << endl;

    return ret;
}

void CtpTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
    if ( !IsErrorRspInfo(pRspInfo) && pRspAuthenticateField ) {  
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 交易认证成功...")
            << pRspAuthenticateField->BrokerID 
            << "|" << pRspAuthenticateField->UserID
            << "|" << pRspAuthenticateField->UserProductInfo
            <<endl;
        TThostFtdcBrokerIDType	appId;
        TThostFtdcUserIDType	userId;	
        TThostFtdcPasswordType	passwd;
        strcpy(appId,AG->getAppId().c_str());
        strcpy(userId,AG->getUserId().c_str());
        strcpy(passwd,AG->getPassWd().c_str());
    	ReqUserLogin(appId,userId,passwd);
    }
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,
    TThostFtdcUserIDType	vUserId,
    TThostFtdcPasswordType	vPasswd)
{
  
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, vAppId); strcpy(appId, vAppId); 
    strcpy(req.UserID, vUserId);  strcpy(userId, vUserId); 
    strcpy(req.Password, vPasswd);
    int ret = pUserApi->ReqUserLogin(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("----请求 | 发送交易登录...")<<((ret == 0) ? TC_Encoder::gbk2utf8("成功") :TC_Encoder::gbk2utf8("失败")) << endl;
}

void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
    if ( !IsErrorRspInfo(pRspInfo) && pRspUserLogin ) {  
        // 保存会话参数	
        frontId = pRspUserLogin->FrontID;
        sessionId = pRspUserLogin->SessionID;
        int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
        sprintf(orderRef, "%d", ++nextOrderRef);
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 交易登录成功...当前交易日:")
            <<pRspUserLogin->TradingDay<<endl;
        ReqSettlementInfoConfirm();
    }
}

void CtpTraderSpi::ReqSettlementInfoConfirm()
{
    CThostFtdcSettlementInfoConfirmField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, appId);
    strcpy(req.InvestorID, userId);
    int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 请求 | 发送结算单确认...")<<((ret == 0)?TC_Encoder::gbk2utf8("成功"):TC_Encoder::gbk2utf8("失败"))<<endl;
}

void CtpTraderSpi::OnRspSettlementInfoConfirm(
    CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{	
    if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 结算单...")<<pSettlementInfoConfirm->InvestorID
            <<"...<"<<pSettlementInfoConfirm->ConfirmDate
            <<" "<<pSettlementInfoConfirm->ConfirmTime<<TC_Encoder::gbk2utf8(">...确认")<<endl;
        //for test
        /*
        TThostFtdcInstrumentIDType instId = "IF1806";
        TThostFtdcInstrumentIDType instId2 = "if1806";
        TThostFtdcDirectionType dir = '0';
        TThostFtdcCombOffsetFlagType kpp; //= {"0","0","0","0","0"};
        strcpy(kpp, "0");
        TThostFtdcPriceType price = 2715;
        TThostFtdcVolumeType vol = 7;
        */

        
        //ReqQryInstrument(instId);  ok
        //ReqQryTradingAccount();
        //ReqQryInvestorPosition(instId);  ok

        //ReqOrderInsert(instId, dir, kpp, price, vol);  //ok
        //TThostFtdcSequenceNoType orderSeq = 2;
        //OnRtnOrder(orderSeq);
        PrintOrders();   //ok
        PrintTrades();
    }
}

void CtpTraderSpi::ReqQryInstrument(TThostFtdcInstrumentIDType instId)
{
    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, instId);
    int ret = pUserApi->ReqQryInstrument(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 请求 | 发送合约查询...")<<((ret == 0)?TC_Encoder::gbk2utf8("成功"):TC_Encoder::gbk2utf8("失败"))<<endl;
}

void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{ 	
    if ( !IsErrorRspInfo(pRspInfo) &&  pInstrument){
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 合约:")<<pInstrument->InstrumentID
            <<TC_Encoder::gbk2utf8(" 交割月:")<<pInstrument->DeliveryMonth
            <<TC_Encoder::gbk2utf8(" 多头保证金率:")<<pInstrument->LongMarginRatio
            <<TC_Encoder::gbk2utf8(" 空头保证金率:")<<pInstrument->ShortMarginRatio<<endl;
    }
}

void CtpTraderSpi::ReqQryTradingAccount()
{
    CThostFtdcQryTradingAccountField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, "3010");
    strcpy(req.InvestorID, "10102291");
    int ret = pUserApi->ReqQryTradingAccount(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 请求 | 发送资金查询...")<<((ret == 0)?TC_Encoder::gbk2utf8("成功"):TC_Encoder::gbk2utf8("失败"))<<endl;
}

void CtpTraderSpi::OnRspQryTradingAccount(
    CThostFtdcTradingAccountField *pTradingAccount, 
   CThostFtdcRspInfoField *pRspInfo, 
   int nRequestID, 
   bool bIsLast)
{ 
	if(!IsErrorRspInfo(pRspInfo) && pTradingAccount)
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 权益:")<<pTradingAccount->Balance
            <<TC_Encoder::gbk2utf8(" 可用:")<<pTradingAccount->Available   
            <<TC_Encoder::gbk2utf8(" 保证金:")<<pTradingAccount->CurrMargin
            <<TC_Encoder::gbk2utf8(" 平仓盈亏:")<<pTradingAccount->CloseProfit
            <<TC_Encoder::gbk2utf8(" 持仓盈亏")<<pTradingAccount->PositionProfit
            <<TC_Encoder::gbk2utf8(" 手续费:")<<pTradingAccount->Commission
            <<TC_Encoder::gbk2utf8(" 冻结保证金:")<<pTradingAccount->FrozenMargin
            << endl;
    }
}

void CtpTraderSpi::ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId)
{
    CThostFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, AG->getAppId().c_str());
    strcpy(req.InvestorID, AG->getUserId().c_str());
    strcpy(req.InstrumentID, instId);	
    int ret = pUserApi->ReqQryInvestorPosition(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 请求 | 发送持仓查询...")<<((ret == 0)?TC_Encoder::gbk2utf8("成功"):TC_Encoder::gbk2utf8("失败"))<<endl;
}

void CtpTraderSpi::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField *pInvestorPosition, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{ 
    if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPosition ){
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 合约:")<<pInvestorPosition->InstrumentID
            <<TC_Encoder::gbk2utf8(" 方向:")<<MapDirection(pInvestorPosition->PosiDirection-2,false)
            <<TC_Encoder::gbk2utf8(" 总持仓:")<<pInvestorPosition->Position
            <<TC_Encoder::gbk2utf8(" 昨仓:")<<pInvestorPosition->YdPosition 
            <<TC_Encoder::gbk2utf8(" 今仓:")<<pInvestorPosition->TodayPosition
            <<TC_Encoder::gbk2utf8(" 持仓盈亏:")<<pInvestorPosition->PositionProfit
            <<TC_Encoder::gbk2utf8(" 保证金:")<<pInvestorPosition->UseMargin<<endl;
        AG->setInvestorPosition(pInvestorPosition->InstrumentID, pInvestorPosition);
    }
}

void CtpTraderSpi::ReqOrderInsert(TThostFtdcInstrumentIDType instId,
    TThostFtdcDirectionType dir, 
    TThostFtdcCombOffsetFlagType kpp,
    TThostFtdcPriceType price,
    TThostFtdcVolumeType vol)
{
    CThostFtdcInputOrderField req;
    memset(&req, 0, sizeof(req));	
    strcpy(req.BrokerID, appId);  //应用单元代码	
    strcpy(req.InvestorID, userId); //投资者代码	
    strcpy(req.InstrumentID, instId); //合约代码	
    strcpy(req.OrderRef, orderRef);  //报单引用
    int nextOrderRef = atoi(orderRef);
    sprintf(orderRef, "%d", ++nextOrderRef);

    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//价格类型=限价	
    req.Direction = MapDirection(dir,true);  //买卖方向	
    req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //THOST_FTDC_OF_Open; //组合开平标志:开仓
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //组合投机套保标志
    req.LimitPrice = price;	//价格
    req.VolumeTotalOriginal = vol;	///数量	
    req.TimeCondition = THOST_FTDC_TC_GFD;  //有效期类型:当日有效
    req.VolumeCondition = THOST_FTDC_VC_AV; //成交量类型:任何数量
    req.MinVolume = 1;	//最小成交量:1	
    req.ContingentCondition = THOST_FTDC_CC_Immediately;  //触发条件:立即

    //TThostFtdcPriceType	StopPrice;  //止损价
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//强平原因:非强平	
    req.IsAutoSuspend = 0;  //自动挂起标志:否	
    req.UserForceClose = 0;   //用户强评标志:否

    int ret = pUserApi->ReqOrderInsert(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 请求 | 发送报单...")<<((ret == 0)?TC_Encoder::gbk2utf8("成功"):TC_Encoder::gbk2utf8("失败"))<< endl;
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
          CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if( !IsErrorRspInfo(pRspInfo) && pInputOrder )
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("响应 | 报单提交成功...报单引用:")<<pInputOrder->OrderRef<<endl;
    }
}

void CtpTraderSpi::ReqOrderAction(TThostFtdcSequenceNoType orderSeq)
{
    bool found=false; unsigned int i=0;
    for(i=0;i<orderList.size();i++)
    {
        if(orderList[i]->BrokerOrderSeq == orderSeq)
        { 
            found = true; 
            break;
        }
    }
    if(!found)
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 请求 | 报单不存在.")<<endl;
        return;
    }

    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, appId);   //经纪公司代码	
    strcpy(req.InvestorID, userId); //投资者代码
    //strcpy(req.OrderRef, pOrderRef); //报单引用	
    //req.FrontID = frontId;           //前置编号	
    //req.SessionID = sessionId;       //会话编号
    strcpy(req.ExchangeID, orderList[i]->ExchangeID);
    strcpy(req.OrderSysID, orderList[i]->OrderSysID);
    req.ActionFlag = THOST_FTDC_AF_Delete;  //操作标志 

    int ret = pUserApi->ReqOrderAction(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 请求 | 发送撤单...") <<((ret == 0)?TC_Encoder::gbk2utf8("成功"):TC_Encoder::gbk2utf8("失败")) << endl;
}

void CtpTraderSpi::OnRspOrderAction(
      CThostFtdcInputOrderActionField *pInputOrderAction, 
      CThostFtdcRspInfoField *pRspInfo, 
      int nRequestID, 
      bool bIsLast)
{	
    if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction)
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 撤单成功...")
            << TC_Encoder::gbk2utf8("交易所:")<<pInputOrderAction->ExchangeID
            <<TC_Encoder::gbk2utf8(" 报单编号:")<<pInputOrderAction->OrderSysID<<endl;
    }
}

///报单回报
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{	
    CThostFtdcOrderField* order = new CThostFtdcOrderField();
    memcpy(order,  pOrder, sizeof(CThostFtdcOrderField));
    bool founded=false;    
    unsigned int i=0;
    for(i=0; i<orderList.size(); i++)
    {
        if(orderList[i]->BrokerOrderSeq == order->BrokerOrderSeq) 
        {
            founded=true;    
            break;
        }
    }
    if(founded)
    {
        orderList[i]= order; 
    }
    else  
    {
        orderList.push_back(order);
    }
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 回报 | 报单已提交...序号:")<<order->BrokerOrderSeq<<endl;
}

///成交通知
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    CThostFtdcTradeField* trade = new CThostFtdcTradeField();
    memcpy(trade,  pTrade, sizeof(CThostFtdcTradeField));
    bool founded=false;     
    unsigned int i=0;
    for(i=0; i<tradeList.size(); i++)
    {
        if(tradeList[i]->TradeID == trade->TradeID) 
        {
            founded=true;   
            break;
        }
    }
    if(founded) 
    {
        tradeList[i] = trade;   
    }
    else  
    {
        tradeList.push_back(trade);
    }
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 回报 | 报单已成交...成交编号:")<<trade->TradeID<<endl;
}

void CtpTraderSpi::OnFrontDisconnected(int nReason)
{
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 交易连接中断...") 
        << TC_Encoder::gbk2utf8(" reason=") << nReason << endl;
}
		
void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | 交易心跳超时警告...") 
        << TC_Encoder::gbk2utf8(" TimerLapse = ") << nTimeLapse << endl;
}

void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
}

bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
    // 如果ErrorID != 0, 说明收到了错误的响应
    bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if (ret)
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 响应 | ")
            <<pRspInfo->ErrorID<<"|"<<TC_Encoder::gbk2utf8(pRspInfo->ErrorMsg)<<endl;
    }
    return ret;
}

//需要写到数据库
void CtpTraderSpi::PrintOrders()
{
    CThostFtdcOrderField* pOrder; 
    for(unsigned int i=0; i<orderList.size(); i++)
    {
        pOrder = orderList[i];
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 报单 | 合约:")<<pOrder->InstrumentID
            <<TC_Encoder::gbk2utf8(" 方向:")<<MapDirection(pOrder->Direction,false)
            <<TC_Encoder::gbk2utf8(" 开平:")<<MapOffset(pOrder->CombOffsetFlag[0],false)
            <<TC_Encoder::gbk2utf8(" 价格:")<<pOrder->LimitPrice
            <<TC_Encoder::gbk2utf8(" 数量:")<<pOrder->VolumeTotalOriginal
            <<TC_Encoder::gbk2utf8(" 序号:")<<pOrder->BrokerOrderSeq 
            <<TC_Encoder::gbk2utf8(" 报单编号:")<<pOrder->OrderSysID
            <<TC_Encoder::gbk2utf8(" 状态:")<<pOrder->StatusMsg<<endl;
    }
}

//需要写到数据库
void CtpTraderSpi::PrintTrades()
{
    CThostFtdcTradeField* pTrade;
    for(unsigned int i=0; i<tradeList.size(); i++)
    {
        pTrade = tradeList[i];
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" 成交 | 合约:")<< pTrade->InstrumentID 
            <<TC_Encoder::gbk2utf8(" 方向:")<<MapDirection(pTrade->Direction,false)
            <<TC_Encoder::gbk2utf8(" 开平:")<<MapOffset(pTrade->OffsetFlag,false) 
            <<TC_Encoder::gbk2utf8(" 价格:")<<pTrade->Price
            <<TC_Encoder::gbk2utf8(" 数量:")<<pTrade->Volume
            <<TC_Encoder::gbk2utf8(" 报单编号:")<<pTrade->OrderSysID
            <<TC_Encoder::gbk2utf8(" 成交编号:")<<pTrade->TradeID<<endl;
    }
}
char MapDirection(char src, bool toOrig=true)
{
    if(toOrig)
    {
        if('b'==src||'B'==src)
        {
            src='0';
        }else if('s'==src||'S'==src)
        {
            src='1';
        }
    }
    else
    {
        if('0'==src)
        {
            src='B';
        }else if('1'==src)
        {
            src='S';
        }
    }
    return src;
}
char MapOffset(char src, bool toOrig=true)
{
    if(toOrig)
    {
        if('o'==src||'O'==src)
        {
            src='0';
        }
        else if('c'==src||'C'==src)
        {
            src='1';
        }
        else if('j'==src||'J'==src)
        {
            src='3';
        }
    }
    else
    {
        if('0'==src)
        {
            src='O';
        }
        else if('1'==src)
        {
            src='C';
        }
        else if('3'==src)
        {
            src='J';
        }
    }
    
    return src;
}

