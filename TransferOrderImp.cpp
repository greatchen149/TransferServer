#include "TransferOrderImp.h"
#include "servant/Application.h"
#include "lh_api/ThostFtdcMdApi.h"
#include "lh_api/ThostFtdcTraderApi.h"
#include "lh_api/ThostFtdcUserApiDataType.h"
#include "lh_api/ThostFtdcUserApiStruct.h"
#include "mdspi.h"
#include "traderspi.h"
#include "MysqlUtil.h"
#include "GR.h"

using namespace std;


//////////////////////////////////////////////////////

void create_order()
{
    // init trader
    CThostFtdcTraderApi* pTraderUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    CtpTraderSpi* pTraderUserSpi = new CtpTraderSpi(pTraderUserApi);
    pTraderUserApi->RegisterSpi((CThostFtdcTraderSpi*)pTraderUserSpi);	    // 注册事件类
    pTraderUserApi->SubscribePublicTopic(THOST_TERT_RESTART);				// 注册公有流
    pTraderUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);			    // 注册私有流
    AG->setTraderUserSpi(pTraderUserSpi);
    //for test
    //char Front[] = "tcp://180.168.146.187:10000";
    //char Front[] = "tcp://122.224.98.87:27225";
    //正式
    char Front[] = "tcp://222.66.101.190:27205";
    pTraderUserApi->RegisterFront(Front);	

    pTraderUserApi->Init();
    pTraderUserApi->Join();  
    pTraderUserSpi->PrintOrders();
    //pTraderUserSpi->Release();
}

void TransferOrderImp::initialize()
{
    //initialize servant here:
    //...
    
    TLOGDEBUG("begin market-----------\n");
    //string appId = "9999";
    //string userId = "113956";
    //string passWd = "88496383";

    //string appId = "3010";
    //string userId = "10102291";
    //string passWd = "xdqh123456";
    //正式
    string appId = "1000";
    string userId = "10691818";
    string passWd = "135246";
    string userProductInfo = "lh@!2018";
    string authCode = "E5WTDDT0BG83FNIX";
    AG->setLogInUser(appId, userId, passWd, userProductInfo, authCode);
    
    create_order();

    TLOGDEBUG("end market-----------\n");
}

//////////////////////////////////////////////////////
void TransferOrderImp::destroy()
{
    //destroy servant here:
    //...
}

int TransferOrderImp::setUserAccount(const UserReq &req, UserRsp &rsp, tars::TarsCurrentPtr current)
{
    return 0;
}

