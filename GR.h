#ifndef _GR_H_
#define _GR_H_

#include <vector>
#include <map>
#include <string>
#include <math.h>
#include "util/tc_mysql.h"
#include "util/tc_singleton.h"
#include "util/tc_monitor.h"
#include "util/tc_common.h"
#include "util/tc_config.h"
#include "util/tc_http.h"
#include "util/tc_thread_rwlock.h"

#include "servant/Application.h"
#include "mdspi.h"
#include "traderspi.h"


using namespace std;
using namespace tars;

struct InstrumentInfo
{
	string Instrument;
	int iCount;
	double dClosePrice;
	string LastTime;
	CThostFtdcInvestorPositionField *m_pInvestorPosition;
	InstrumentInfo()
	{
		iCount = 0;
		dClosePrice = 0.0d;
		m_pInvestorPosition = NULL;
	};
	void Init()
	{
		iCount = 0;
	};
	void setInstrument(const string& instrument, const double& closePrice, const string& lastTime)
	{
		if(Instrument == instrument)
		{
			iCount++;
			LastTime = lastTime;
			dClosePrice = closePrice;
		}
	}
	void setInvestorPosition(const string& instrument, CThostFtdcInvestorPositionField *pInvestorPosition)
	{
		if(Instrument == instrument)
		{
			m_pInvestorPosition = pInvestorPosition;
		}
	}
	string toString()
	{
		stringstream ss;
		ss << Instrument << "|" << dClosePrice << "|" << LastTime << "|" << iCount
			 << "|" << getPosition() << "|" << getPositionProfit();
		return ss.str();
	}
	int getPosition()
	{
		if(m_pInvestorPosition != NULL)
			return m_pInvestorPosition->Position;

		return -1;
	}
	int getYdPosition()
	{
		if(m_pInvestorPosition != NULL)
			return m_pInvestorPosition->YdPosition;

		return -1;
	}
	int getTodayPosition()
	{
		if(m_pInvestorPosition != NULL)
			return m_pInvestorPosition->TodayPosition;

		return -1;
	}
	double getPositionProfit()
	{
		if(m_pInvestorPosition != NULL)
			return m_pInvestorPosition->PositionProfit;

		return -1;
	}
	double getUseMargin()
	{
		if(m_pInvestorPosition != NULL)
			return m_pInvestorPosition->UseMargin;

		return -1;
	}
};

class MonitorPair
{
public:
	int pairId;
	string appId;
	string userId;
	int strategyId;
	InstrumentInfo it1;
	InstrumentInfo it2;
	int positionPer;
public:
	MonitorPair()
	{
		pairId = 0;
		strategyId = 0;
		positionPer = 0;
	};
	MonitorPair(const string& appid, const string& userid, 
		const string& instrument1, const string& instrument2,
		const int& pairid, const int& strategyid, const int& positionper)
	{
		pairId = pairid;
		strategyId = strategyid;
		positionPer = positionper;
		appId = appid;
		userId = userid;
		it1.Instrument = instrument1;
		it2.Instrument = instrument2;
	};
	MonitorPair(const MonitorPair& other)
	{
		pairId = other.pairId;
		strategyId = other.strategyId;
		positionPer = other.positionPer;
		appId = other.appId;
		userId = other.userId;
		it1.Instrument = other.it1.Instrument;
		it2.Instrument = other.it2.Instrument;
	};
	void Init()
	{
		it1.Init();
		it2.Init();
	};
	string toString()
	{
		stringstream ss;
		ss << "pairId[" << pairId << "]"  << "----"
			<< "appId[" << appId << "]"  << "----"
			<< "strategyId[" << strategyId << "]"  << "----"
			<< "userId[" << userId << "]"  << "----"
			<< "instrument1[" << it1.toString() << "]" << "----"
			<< "instrument2[" << it2.toString() << "]" << "----"
			<< "positionPer[" << positionPer << "]";
		return ss.str();
	} 
	void setInstrument(const string& instrument, const double& closePrice, const string& lastTime);
	void setInvestorPosition(const string& instrument, CThostFtdcInvestorPositionField *pInvestorPosition);
	bool isRun()
	{
		int tmp = 150;
		//60¿ÉÅäÖÃ£¬5·ÖÖÓ
		if(it1.iCount > tmp && it2.iCount > tmp)
		{
			it1.Init();
			it2.Init();
			return true;
		}

		return false;
	};
	bool isMatch(const string& appid, const string& userid)
	{
		if(appId == appid && userId == userid)
		{
			return true;
		}

		return false;
	}
};

#define AG (GR::getInstance())

class GR :public TC_Singleton<GR>, public TC_ThreadLock
{
public:
    virtual void initialize();
	
	void setUserInstrumentList(const string& appId, const string& userId, const string& instrumentList);
	string getUserInstrumentList(const string& appId, const string& userId);
	string getUserInstrumentList();
	
	void setMonitorPair(const string& appid, const string& userid, MonitorPair& stMonitorPair);
	void setInstrument(const string& instrument, const double& closePrice, const string& lastTime);
	void setInvestorPosition(const string& instrument, CThostFtdcInvestorPositionField *pInvestorPosition);
	
	void setLogInUser(const string& appid, const string& userid, const string& passWd, 
	    const string& userProductInfo, const string& authCode);
	void setTraderUserSpi(CtpTraderSpi* pTraderUserSpi)
	{
		if(pTraderUserSpi == NULL) return;
		g_pTraderUserSpi = pTraderUserSpi;
	}
	void setUserSpi(CtpMdSpi* pUserSpi)
	{
		if(pUserSpi == NULL) return;
		g_pUserSpi = pUserSpi;
	}
	string getAppId(){return app_id;}
	string getUserId(){return user_id;}
	string getPassWd(){return pass_wd;}
	string getUserProductInfo(){return user_product_info;}
	string getAuthCode(){return auth_code;}
	
protected:
	map< pair<string, string>, string > m_InstrumentList;
	map< pair<string, string>, vector<MonitorPair> > m_Monitor;
	int entry_score;
	string app_id;
	string user_id;
	string pass_wd;
	string user_product_info;
	string auth_code;
public:
	CtpTraderSpi* 	g_pTraderUserSpi;
	CtpMdSpi* 		g_pUserSpi;
};

#endif


