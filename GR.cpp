#include "GR.h"
#include "util/tc_common.h"
#include "MysqlUtil.h"

using namespace std;
using namespace tars;

void GR::initialize()
{
	FDLOG("GR") << "GR::initialize begin..." << endl;
    entry_score = 2;

	FDLOG("GR") << "GR::initialize is end" << endl;
}

void GR::setLogInUser(const string& appid, const string& userid, const string& passWd, 
    const string& userProductInfo, const string& authCode)
{
    app_id = appid;
    user_id = userid;
    pass_wd = passWd;
    user_product_info = userProductInfo;
    auth_code = authCode;
}

void GR::setMonitorPair(const string& appid, const string& userid, MonitorPair& stMonitorPair)
{
	FDLOG("GR") << "setMonitorPair----appid:" <<appid << ",userid:" << userid 
        << ",stMonitorPair:" << stMonitorPair.toString()
        << endl;
    map< pair<string, string>, vector<MonitorPair> >::iterator it = m_Monitor.find(make_pair(appid, userid));
    if(it != m_Monitor.end())
    {
        it->second.push_back(stMonitorPair);
    }
    else
    {
        vector<MonitorPair> vMonitorPair;
        vMonitorPair.push_back(stMonitorPair);
        m_Monitor[make_pair(appid, userid)] = vMonitorPair;
    }
}

void GR::setInstrument(const string& instrument, const double& closePrice, const string& lastTime)
{
    FDLOG("GR") << "setInstrument------app_id:" << app_id << ",user_id:" << user_id 
        <<",instrument:" << instrument
        <<",closePrice:" << closePrice
        <<",lastTime:" << lastTime
        << endl;
    map< pair<string, string>, vector<MonitorPair> >::iterator it = m_Monitor.find(make_pair(app_id, user_id));
    if(it != m_Monitor.end())
    {
        vector<MonitorPair>& vMonitorPair = it->second;
        for(size_t i = 0; i < vMonitorPair.size(); i++)
        {
            vMonitorPair[i].setInstrument(instrument, closePrice, lastTime);
        }
    }
}

void GR::setInvestorPosition(const string& instrument, CThostFtdcInvestorPositionField *pInvestorPosition)
{
    if(pInvestorPosition == NULL)
    {
        FDLOG("GR") << "setInvestorPosition------app_id:" << app_id << ",user_id:" << user_id 
            <<",instrument:" << instrument << ", is null" << endl;

        return;
    }
    FDLOG("GR") << "setInvestorPosition------app_id:" << app_id << ",user_id:" << user_id 
        <<",instrument:" << instrument
        <<",Position:" << pInvestorPosition->Position
        <<",PositionProfit:" << pInvestorPosition->PositionProfit
        << endl;
    map< pair<string, string>, vector<MonitorPair> >::iterator it = m_Monitor.find(make_pair(app_id, user_id));
    if(it != m_Monitor.end())
    {
        vector<MonitorPair>& vMonitorPair = it->second;
        for(size_t i = 0; i < vMonitorPair.size(); i++)
        {
            vMonitorPair[i].setInvestorPosition(instrument, pInvestorPosition);
        }
    }
}

void GR::setUserInstrumentList(const string& appId, const string& userId, const string& instrumentList)
{
    m_InstrumentList[make_pair(appId, userId)] = instrumentList;
}

string GR::getUserInstrumentList(const string& appId, const string& userId)
{
    FDLOG("GR") << "getInstrumentList------app_id:" << appId << ",user_id:" << userId << endl;
    map< pair<string, string>, string >::iterator it = m_InstrumentList.find(make_pair(appId, userId));
    if(it != m_InstrumentList.end()) return it->second;
    else return "";
}

string GR::getUserInstrumentList()
{
    FDLOG("GR") << "getInstrumentList------app_id:" << app_id << ",user_id:" << user_id << endl;
    map< pair<string, string>, string >::iterator it = m_InstrumentList.find(make_pair(app_id, user_id));
    if(it != m_InstrumentList.end()) return it->second;
    else return "";
}

void MonitorPair::setInvestorPosition(const string& instrument, CThostFtdcInvestorPositionField *pInvestorPosition)
{
    it1.setInvestorPosition(instrument, pInvestorPosition);
    it2.setInvestorPosition(instrument, pInvestorPosition);
}
void MonitorPair::setInstrument(const string& instrument, const double& closePrice, const string& lastTime)
{
    it1.setInstrument(instrument, closePrice, lastTime);
    it2.setInstrument(instrument, closePrice, lastTime);
    double spread = it1.dClosePrice - it2.dClosePrice;
    FDLOG("setInstrument") << "appId[" << appId << "]"  << "----"
        << "userId[" << userId << "]"  << "----"
        << "instrument1[" << it1.toString() << "]" << "----"
        << "instrument2[" << it2.toString() << "]" << "----"
        << "spread[" << spread << "]"
        << endl;
    
    if(it1.dClosePrice <= 0 || it2.dClosePrice <= 0)
    {
        return;
    }
    if(isRun())
    {
        TThostFtdcInstrumentIDType instId;
        TThostFtdcInstrumentIDType instId2;
        strcpy(instId,it1.Instrument.c_str());
        strcpy(instId2,it2.Instrument.c_str());
        AG->g_pTraderUserSpi->ReqQryInvestorPosition(instId);
        AG->g_pTraderUserSpi->ReqQryInvestorPosition(instId2);
        MysqlUtil::getInstance()->insertInstrumentSpread(appId,userId,lastTime,
            it1.Instrument,it1.dClosePrice,
            it2.Instrument,it2.dClosePrice,
            spread
            );
        
        if(spread <= -9)
        {
            FDLOG("operation") << "step1--: buy in " << it1.Instrument << "--price:" << it1.dClosePrice << "--Position:" << it1.getPosition()
                <<";evening up " << it2.Instrument << "--price:" << it2.dClosePrice << "--Position:" << it2.getPosition()
                << ";spread:" << spread
                << endl;
            AG->g_pTraderUserSpi->ReqOrderInsert(instId, dir, kpp, price, vol); 
        }
        if(spread >= 11)
        {
            FDLOG("operation") << "step2--: buy in " << it2.Instrument << "--price:" << it2.dClosePrice << "--Position:" << it1.getPosition()
                <<";evening up " << it1.Instrument << "--price:" << it1.dClosePrice << "--Position:" << it2.getPosition()
                << ";spread:" << spread
                << endl;
        }
    }
}



