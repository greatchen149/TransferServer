#include "TransferImp.h"
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

void create_md(){
    // init md
    CThostFtdcMdApi* pUserApi=CThostFtdcMdApi::CreateFtdcMdApi();
    CtpMdSpi* pUserSpi=new CtpMdSpi(pUserApi);
    pUserApi->RegisterSpi(pUserSpi);
    AG->setUserSpi(pUserSpi);
    //for test
    //char MdFront[] = "tcp://180.168.146.187:10010";
    //char MdFront[] = "tcp://122.224.98.87:27235";
    //正式
    char MdFront[] = "tcp://222.66.101.190:27215";
    pUserApi->RegisterFront(MdFront);
    pUserApi->Init();
    pUserApi->Join();
    //pUserApi->Release(); //接口对象释放
}

void TransferImp::initialize()
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
    
    create_md();

    TLOGDEBUG("end market-----------\n");
}

//////////////////////////////////////////////////////
void TransferImp::destroy()
{
    //destroy servant here:
    //...
}

int TransferImp::setUserAccount(const UserReq &req, UserRsp &rsp, tars::TarsCurrentPtr current)
{
    return 0;
}

