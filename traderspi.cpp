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

TThostFtdcBrokerIDType appId;		// Ӧ�õ�Ԫ
TThostFtdcUserIDType userId;		// Ͷ���ߴ���


int t_requestId = 0; 

// �Ự����
int	 frontId;	//ǰ�ñ��
int	 sessionId;	//�Ự���
char orderRef[13];

vector<CThostFtdcOrderField*> orderList;
vector<CThostFtdcTradeField*> tradeList;

char MapDirection(char src, bool toOrig);
char MapOffset(char src, bool toOrig);

void CtpTraderSpi::OnFrontConnected()
{
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("----���ӽ���ǰ��...�ɹ�")<<endl;
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
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("----���� | ���ͽ�����֤����...")<<((ret == 0) ? TC_Encoder::gbk2utf8("�ɹ�") :TC_Encoder::gbk2utf8("ʧ��")) << endl;

    return ret;
}

void CtpTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
    if ( !IsErrorRspInfo(pRspInfo) && pRspAuthenticateField ) {  
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ������֤�ɹ�...")
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
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("----���� | ���ͽ��׵�¼...")<<((ret == 0) ? TC_Encoder::gbk2utf8("�ɹ�") :TC_Encoder::gbk2utf8("ʧ��")) << endl;
}

void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
    if ( !IsErrorRspInfo(pRspInfo) && pRspUserLogin ) {  
        // ����Ự����	
        frontId = pRspUserLogin->FrontID;
        sessionId = pRspUserLogin->SessionID;
        int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
        sprintf(orderRef, "%d", ++nextOrderRef);
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ���׵�¼�ɹ�...��ǰ������:")
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
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | ���ͽ��㵥ȷ��...")<<((ret == 0)?TC_Encoder::gbk2utf8("�ɹ�"):TC_Encoder::gbk2utf8("ʧ��"))<<endl;
}

void CtpTraderSpi::OnRspSettlementInfoConfirm(
    CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{	
    if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ���㵥...")<<pSettlementInfoConfirm->InvestorID
            <<"...<"<<pSettlementInfoConfirm->ConfirmDate
            <<" "<<pSettlementInfoConfirm->ConfirmTime<<TC_Encoder::gbk2utf8(">...ȷ��")<<endl;
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
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | ���ͺ�Լ��ѯ...")<<((ret == 0)?TC_Encoder::gbk2utf8("�ɹ�"):TC_Encoder::gbk2utf8("ʧ��"))<<endl;
}

void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{ 	
    if ( !IsErrorRspInfo(pRspInfo) &&  pInstrument){
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ��Լ:")<<pInstrument->InstrumentID
            <<TC_Encoder::gbk2utf8(" ������:")<<pInstrument->DeliveryMonth
            <<TC_Encoder::gbk2utf8(" ��ͷ��֤����:")<<pInstrument->LongMarginRatio
            <<TC_Encoder::gbk2utf8(" ��ͷ��֤����:")<<pInstrument->ShortMarginRatio<<endl;
    }
}

void CtpTraderSpi::ReqQryTradingAccount()
{
    CThostFtdcQryTradingAccountField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, "3010");
    strcpy(req.InvestorID, "10102291");
    int ret = pUserApi->ReqQryTradingAccount(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | �����ʽ��ѯ...")<<((ret == 0)?TC_Encoder::gbk2utf8("�ɹ�"):TC_Encoder::gbk2utf8("ʧ��"))<<endl;
}

void CtpTraderSpi::OnRspQryTradingAccount(
    CThostFtdcTradingAccountField *pTradingAccount, 
   CThostFtdcRspInfoField *pRspInfo, 
   int nRequestID, 
   bool bIsLast)
{ 
	if(!IsErrorRspInfo(pRspInfo) && pTradingAccount)
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | Ȩ��:")<<pTradingAccount->Balance
            <<TC_Encoder::gbk2utf8(" ����:")<<pTradingAccount->Available   
            <<TC_Encoder::gbk2utf8(" ��֤��:")<<pTradingAccount->CurrMargin
            <<TC_Encoder::gbk2utf8(" ƽ��ӯ��:")<<pTradingAccount->CloseProfit
            <<TC_Encoder::gbk2utf8(" �ֲ�ӯ��")<<pTradingAccount->PositionProfit
            <<TC_Encoder::gbk2utf8(" ������:")<<pTradingAccount->Commission
            <<TC_Encoder::gbk2utf8(" ���ᱣ֤��:")<<pTradingAccount->FrozenMargin
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
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | ���ͳֲֲ�ѯ...")<<((ret == 0)?TC_Encoder::gbk2utf8("�ɹ�"):TC_Encoder::gbk2utf8("ʧ��"))<<endl;
}

void CtpTraderSpi::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField *pInvestorPosition, 
    CThostFtdcRspInfoField *pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{ 
    if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPosition ){
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ��Լ:")<<pInvestorPosition->InstrumentID
            <<TC_Encoder::gbk2utf8(" ����:")<<MapDirection(pInvestorPosition->PosiDirection-2,false)
            <<TC_Encoder::gbk2utf8(" �ֲܳ�:")<<pInvestorPosition->Position
            <<TC_Encoder::gbk2utf8(" ���:")<<pInvestorPosition->YdPosition 
            <<TC_Encoder::gbk2utf8(" ���:")<<pInvestorPosition->TodayPosition
            <<TC_Encoder::gbk2utf8(" �ֲ�ӯ��:")<<pInvestorPosition->PositionProfit
            <<TC_Encoder::gbk2utf8(" ��֤��:")<<pInvestorPosition->UseMargin<<endl;
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
    strcpy(req.BrokerID, appId);  //Ӧ�õ�Ԫ����	
    strcpy(req.InvestorID, userId); //Ͷ���ߴ���	
    strcpy(req.InstrumentID, instId); //��Լ����	
    strcpy(req.OrderRef, orderRef);  //��������
    int nextOrderRef = atoi(orderRef);
    sprintf(orderRef, "%d", ++nextOrderRef);

    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//�۸�����=�޼�	
    req.Direction = MapDirection(dir,true);  //��������	
    req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //THOST_FTDC_OF_Open; //��Ͽ�ƽ��־:����
    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //���Ͷ���ױ���־
    req.LimitPrice = price;	//�۸�
    req.VolumeTotalOriginal = vol;	///����	
    req.TimeCondition = THOST_FTDC_TC_GFD;  //��Ч������:������Ч
    req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
    req.MinVolume = 1;	//��С�ɽ���:1	
    req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����

    //TThostFtdcPriceType	StopPrice;  //ֹ���
    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
    req.IsAutoSuspend = 0;  //�Զ������־:��	
    req.UserForceClose = 0;   //�û�ǿ����־:��

    int ret = pUserApi->ReqOrderInsert(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | ���ͱ���...")<<((ret == 0)?TC_Encoder::gbk2utf8("�ɹ�"):TC_Encoder::gbk2utf8("ʧ��"))<< endl;
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
          CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if( !IsErrorRspInfo(pRspInfo) && pInputOrder )
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8("��Ӧ | �����ύ�ɹ�...��������:")<<pInputOrder->OrderRef<<endl;
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
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | ����������.")<<endl;
        return;
    }

    CThostFtdcInputOrderActionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, appId);   //���͹�˾����	
    strcpy(req.InvestorID, userId); //Ͷ���ߴ���
    //strcpy(req.OrderRef, pOrderRef); //��������	
    //req.FrontID = frontId;           //ǰ�ñ��	
    //req.SessionID = sessionId;       //�Ự���
    strcpy(req.ExchangeID, orderList[i]->ExchangeID);
    strcpy(req.OrderSysID, orderList[i]->OrderSysID);
    req.ActionFlag = THOST_FTDC_AF_Delete;  //������־ 

    int ret = pUserApi->ReqOrderAction(&req, ++t_requestId);
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | ���ͳ���...") <<((ret == 0)?TC_Encoder::gbk2utf8("�ɹ�"):TC_Encoder::gbk2utf8("ʧ��")) << endl;
}

void CtpTraderSpi::OnRspOrderAction(
      CThostFtdcInputOrderActionField *pInputOrderAction, 
      CThostFtdcRspInfoField *pRspInfo, 
      int nRequestID, 
      bool bIsLast)
{	
    if (!IsErrorRspInfo(pRspInfo) && pInputOrderAction)
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | �����ɹ�...")
            << TC_Encoder::gbk2utf8("������:")<<pInputOrderAction->ExchangeID
            <<TC_Encoder::gbk2utf8(" �������:")<<pInputOrderAction->OrderSysID<<endl;
    }
}

///�����ر�
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
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" �ر� | �������ύ...���:")<<order->BrokerOrderSeq<<endl;
}

///�ɽ�֪ͨ
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
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" �ر� | �����ѳɽ�...�ɽ����:")<<trade->TradeID<<endl;
}

void CtpTraderSpi::OnFrontDisconnected(int nReason)
{
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ���������ж�...") 
        << TC_Encoder::gbk2utf8(" reason=") << nReason << endl;
}
		
void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ����������ʱ����...") 
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
    // ���ErrorID != 0, ˵���յ��˴������Ӧ
    bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if (ret)
    {
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ��Ӧ | ")
            <<pRspInfo->ErrorID<<"|"<<TC_Encoder::gbk2utf8(pRspInfo->ErrorMsg)<<endl;
    }
    return ret;
}

//��Ҫд�����ݿ�
void CtpTraderSpi::PrintOrders()
{
    CThostFtdcOrderField* pOrder; 
    for(unsigned int i=0; i<orderList.size(); i++)
    {
        pOrder = orderList[i];
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" ���� | ��Լ:")<<pOrder->InstrumentID
            <<TC_Encoder::gbk2utf8(" ����:")<<MapDirection(pOrder->Direction,false)
            <<TC_Encoder::gbk2utf8(" ��ƽ:")<<MapOffset(pOrder->CombOffsetFlag[0],false)
            <<TC_Encoder::gbk2utf8(" �۸�:")<<pOrder->LimitPrice
            <<TC_Encoder::gbk2utf8(" ����:")<<pOrder->VolumeTotalOriginal
            <<TC_Encoder::gbk2utf8(" ���:")<<pOrder->BrokerOrderSeq 
            <<TC_Encoder::gbk2utf8(" �������:")<<pOrder->OrderSysID
            <<TC_Encoder::gbk2utf8(" ״̬:")<<pOrder->StatusMsg<<endl;
    }
}

//��Ҫд�����ݿ�
void CtpTraderSpi::PrintTrades()
{
    CThostFtdcTradeField* pTrade;
    for(unsigned int i=0; i<tradeList.size(); i++)
    {
        pTrade = tradeList[i];
        FDLOG("CtpTraderSpi") << TC_Encoder::gbk2utf8(" �ɽ� | ��Լ:")<< pTrade->InstrumentID 
            <<TC_Encoder::gbk2utf8(" ����:")<<MapDirection(pTrade->Direction,false)
            <<TC_Encoder::gbk2utf8(" ��ƽ:")<<MapOffset(pTrade->OffsetFlag,false) 
            <<TC_Encoder::gbk2utf8(" �۸�:")<<pTrade->Price
            <<TC_Encoder::gbk2utf8(" ����:")<<pTrade->Volume
            <<TC_Encoder::gbk2utf8(" �������:")<<pTrade->OrderSysID
            <<TC_Encoder::gbk2utf8(" �ɽ����:")<<pTrade->TradeID<<endl;
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

