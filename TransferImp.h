#ifndef _TransferImp_H_
#define _TransferImp_H_

#include "servant/Application.h"
#include "Transfer.h"

using namespace BIT;

class TransferImp : public BIT::Hello
{
public:
 
    virtual ~TransferImp() {}

    virtual void initialize();

    virtual void destroy();

    virtual int test(tars::TarsCurrentPtr current) { return 0;};

    virtual int setUserAccount(const UserReq &req, UserRsp &rsp, tars::TarsCurrentPtr current);
	
	virtual int getInstrument(const InstrumentReq &req, InstrumentRsp &rsp, tars::TarsCurrentPtr current) { return 0;};

	virtual int getTradingAccount(const TradingAccountReq &req, TradingAccountRsp &rsp, tars::TarsCurrentPtr current) { return 0;};

	virtual int getInvestorPosition(const InvestorPositionReq &req, InvestorPositionRsp &rsp, tars::TarsCurrentPtr current) { return 0;};

	virtual int OrderInsert(const OrderInsertReq &req, OrderInsertRsp &rsp, tars::TarsCurrentPtr current) { return 0;};

	virtual int OrderAction(const OrderActionReq &req, OrderActionRsp &rsp, tars::TarsCurrentPtr current) { return 0;};
};
/////////////////////////////////////////////////////
#endif
