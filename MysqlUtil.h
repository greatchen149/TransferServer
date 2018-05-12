#pragma once

#include <map>
#include <set>

#include "util/tc_mysql.h"
#include "util/tc_singleton.h"
#include "GR.h"

using namespace std;
using namespace tars;


namespace Mysql {

    enum RET_CODE
    {
        SUCC = 0,
        ERR = -1,
    };
};

struct UserInfo
{
	string app_id;
	string user_id;
	string passwd;
};

struct InstrumentList
{
	string app_id;
	string user_id;
	string instrument_list;
};

class MysqlUtil: public TC_Singleton<MysqlUtil>
{
    public:
        int initialize();

    public:
        Mysql::RET_CODE insertQT(const string& fdate, 
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
	    );
        Mysql::RET_CODE selectUser();
		Mysql::RET_CODE selectInstrument();
		Mysql::RET_CODE selectMonitor();
		Mysql::RET_CODE insertInstrumentSpread(
			const string& app_id, 
		    const string& user_id, 
			const string& fdate, 
		    const string& main_instrument_id,
		    const double& main_last_price,
		    const string& second_instrument_id,
		    const double& second_last_price,
		    const double& spread
	    );
    private:
        TC_Mysql m_quotation_mysql;
        TC_ThreadLock m_quotation_mutex;

		TC_Mysql m_spread_mysql;
        TC_ThreadLock m_spread_mutex;

		vector<UserInfo>	m_UserInfo;
		
};
