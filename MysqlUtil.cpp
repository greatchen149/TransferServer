#include "MysqlUtil.h"
#include "TransferServer.h"
#include "util/tc_config.h"
#include "util/tc_common.h"
#include "GR.h"

using namespace tars;

static void getDBInfo(const TC_Config& dbCfg, map<string, map<string, string> >& mDBInfo)
{
    const vector<string>& vDomain = dbCfg.getDomainVector("/");

    for(size_t i = 0; i < vDomain.size(); i++)
    {
        const string& moudle = vDomain[i];
        mDBInfo[moudle]["charset"] =
            dbCfg.get("/" + moudle + "/<charset>");
        mDBInfo[moudle]["dbhost"] =
            dbCfg.get("/" + moudle + "/<dbhost>");
        mDBInfo[moudle]["dbname"] =
            dbCfg.get("/" + moudle + "/<dbname>");
        mDBInfo[moudle]["dbpass"] =
            dbCfg.get("/" + moudle + "/<dbpass>");
        mDBInfo[moudle]["dbport"] =
            dbCfg.get("/" + moudle + "/<dbport>");
        mDBInfo[moudle]["dbuser"] =
            dbCfg.get("/" + moudle + "/<dbuser>");
    }
}

int MysqlUtil::initialize()
{
    TC_Config dbCfg;
    //const string& sFile = ServerConfig::BasePath + "DBInfo.conf";
    const string& sFile = "/home/greatchen/work/TransferServer/DBInfo.conf";
    dbCfg.parseFile(sFile);

    map<string, map<string, string> > mDBInfo;
    getDBInfo(dbCfg, mDBInfo);

    //RECOMM
    TC_DBConf tcDBConf_recomm;
    tcDBConf_recomm.loadFromMap(mDBInfo["QUOTATION"]);
    m_quotation_mysql.init(tcDBConf_recomm);
    m_spread_mysql.init(tcDBConf_recomm);

    selectUser();
    selectInstrument();
    selectMonitor();

    FDLOG("Init") << "MysqlUtil initialized." << endl;
    return 0;
}

Mysql::RET_CODE MysqlUtil::insertQT(const string& fdate, 
    const string& instrument_id,
    const double& last_price,
    const double& highest_price,
    const double& lowest_price,
    const double& ask_price1,
    const double& ask_volume1,
    const double& bid_price1,
    const double& bid_volume1,
    const double& open_interest,
    const double& PreSettlementPrice,
    const double& PreClosePrice,
    const double& PreOpenInterest,
    const double& OpenPrice,
    const int& Volume,
    const double& Turnover,
    const double& ClosePrice,
    const double& SettlementPrice,
    const double& UpperLimitPrice,
    const double& LowerLimitPrice,
    const double& AveragePrice,
    const double& PreDelta,
    const double& CurrDelta,
    const double& ask_price2,
    const double& ask_volume2,
    const double& bid_price2,
    const double& bid_volume2,
    const double& ask_price3,
    const double& ask_volume3,
    const double& bid_price3,
    const double& bid_volume3,
    const double& ask_price4,
    const double& ask_volume4,
    const double& bid_price4,
    const double& bid_volume4,
    const double& ask_price5,
    const double& ask_volume5,
    const double& bid_price5,
    const double& bid_volume5,
    const string& nowTime
    )
{
    string nowTmpTime = TC_Common::now2str("%Y%m%d");
    stringstream ss;
    ss.str("");
    ss << "insert into t_quotation_" << nowTmpTime <<" set fdate='" << fdate << "'"
        << ",instrument_id='" << instrument_id << "'"
        << ",last_price=" << last_price
        << ",highest_price=" << highest_price
        << ",lowest_price=" << lowest_price
        << ",ask_price1=" << ask_price1
        << ",ask_volume1=" << ask_volume1
        << ",bid_price1=" << bid_price1
        << ",bid_volume1=" << bid_volume1
        << ",open_interest=" << open_interest
        << ",pre_settlement_price=" << PreSettlementPrice
        << ",pre_close_price=" << PreClosePrice
        << ",pre_open_interest=" << PreOpenInterest
        << ",open_price=" << OpenPrice
        << ",volume=" << Volume
        << ",turnover=" << Turnover
        << ",close_price=" << ClosePrice
        << ",settlement_price=" << SettlementPrice
        << ",upper_limit_price=" << UpperLimitPrice
        << ",lower_limit_price=" << LowerLimitPrice
        << ",average_price=" << AveragePrice
        << ",pre_delta=" << PreDelta
        << ",curr_delta=" << CurrDelta
        << ",ask_price2=" << ask_price2
        << ",ask_volume2=" << ask_volume2
        << ",bid_price2=" << bid_price2
        << ",bid_volume2=" << bid_volume2
        << ",ask_price3=" << ask_price3
        << ",ask_volume3=" << ask_volume3
        << ",bid_price3=" << bid_price3
        << ",bid_volume3=" << bid_volume3
        << ",ask_price4=" << ask_price4
        << ",ask_volume4=" << ask_volume4
        << ",bid_price4=" << bid_price4
        << ",bid_volume4=" << bid_volume4
        << ",ask_price5=" << ask_price5
        << ",ask_volume5=" << ask_volume5
        << ",bid_price5=" << bid_price5
        << ",bid_volume5=" << bid_volume5
        << ",ftodaydate='" << nowTime << "'"
        ;
    //FDLOG("insertQT") << "ss=" << ss.str() << endl;
    try
    {
        TC_ThreadLock::Lock lock(m_quotation_mutex);
        m_quotation_mysql.execute(ss.str());
        //FDLOG("insertQT") << "ss=" << ss.str() << ",succ" << endl;
    }
    catch(exception& ex)
    {
        FDLOG("mysql_error") << ex.what() << endl;
        return Mysql::ERR;
    }
    return Mysql::SUCC;
}


Mysql::RET_CODE MysqlUtil::selectUser()
{
    string sql = "select * from t_user";

    try
    {
        TC_Mysql::MysqlData result;
        {
            TC_ThreadLock::Lock lock(m_quotation_mutex);
            result = m_quotation_mysql.queryRecord(sql);
        }

        for(size_t i = 0; i < result.size(); ++i)
        {
            UserInfo stUserInfo;
            stUserInfo.app_id = result[i]["app_id"];
            stUserInfo.user_id = result[i]["user_id"];
            stUserInfo.passwd = result[i]["passwd"];
            m_UserInfo.push_back(stUserInfo);
        }
    }
    catch(exception& ex)
    {
        FDLOG("mysql_error") << ex.what() << endl;
        return Mysql::ERR;
    }
    return Mysql::SUCC;
}

Mysql::RET_CODE MysqlUtil::selectInstrument()
{
    string sql = "select * from t_instrument_list";

    try
    {
        TC_Mysql::MysqlData result;
        {
            TC_ThreadLock::Lock lock(m_quotation_mutex);
            result = m_quotation_mysql.queryRecord(sql);
        }

        for(size_t i = 0; i < result.size(); ++i)
        {
            AG->setUserInstrumentList(result[i]["app_id"], result[i]["user_id"], result[i]["instrument_list"]);
        }
    }
    catch(exception& ex)
    {
        FDLOG("mysql_error") << ex.what() << endl;
        return Mysql::ERR;
    }
    return Mysql::SUCC;
}

Mysql::RET_CODE MysqlUtil::selectMonitor()
{
    string sql = "select * from t_monitor_list";

    try
    {
        TC_Mysql::MysqlData result;
        {
            TC_ThreadLock::Lock lock(m_quotation_mutex);
            result = m_quotation_mysql.queryRecord(sql);
        }

        for(size_t i = 0; i < result.size(); i++)
        {
            MonitorPair stMonitorPair;
            stMonitorPair.pairId = TC_Common::strto<int>(result[i]["pair_id"]);
            stMonitorPair.appId = result[i]["app_id"];
            stMonitorPair.userId = result[i]["user_id"];
            stMonitorPair.strategyId = TC_Common::strto<int>(result[i]["strategy_id"]);
            stMonitorPair.it1.Instrument = result[i]["main_instrument"];
            stMonitorPair.it2.Instrument = result[i]["second_instrument"];
            stMonitorPair.positionPer = TC_Common::strto<int>(result[i]["position_per"]);
            AG->setMonitorPair(result[i]["app_id"], result[i]["user_id"], stMonitorPair);
        }
    }
    catch(exception& ex)
    {
        FDLOG("mysql_error") << ex.what() << endl;
        return Mysql::ERR;
    }
    return Mysql::SUCC;
}

Mysql::RET_CODE MysqlUtil::insertInstrumentSpread(
    const string& app_id, 
    const string& user_id, 
    const string& fdate, 
    const string& main_instrument_id,
    const double& main_last_price,
    const string& second_instrument_id,
    const double& second_last_price,
    const double& spread
    )
{
    string nowTime = TC_Common::now2str("%Y%m%d");
    stringstream ss;
    ss.str("");
    ss << "insert into t_instrument_spread_" << nowTime <<" set "
        << "app_id='" << app_id << "'"
        << ",user_id='" << user_id << "'"
        << ",fdate='" << fdate << "'"
        << ",main_instrument='" << main_instrument_id << "'"
        << ",main_last_price=" << main_last_price
        << ",second_instrument='" << second_instrument_id << "'"
        << ",second_last_price=" << second_last_price
        << ",spread=" << spread
        ;
    //FDLOG("insertQT") << "ss=" << ss.str() << endl;
    try
    {
        TC_ThreadLock::Lock lock(m_quotation_mutex);
        m_quotation_mysql.execute(ss.str());
        //FDLOG("insertQT") << "ss=" << ss.str() << ",succ" << endl;
    }
    catch(exception& ex)
    {
        FDLOG("mysql_error") << ex.what() << endl;
        return Mysql::ERR;
    }
    return Mysql::SUCC;
}


