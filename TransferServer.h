#ifndef _TransferServer_H_
#define _TransferServer_H_

#include <iostream>
#include "servant/Application.h"
#include "MysqlUtil.h"

using namespace tars;

class TransferServer : public Application
{
public:

    virtual ~TransferServer() {};

    virtual void initialize();

    virtual void destroyApp();
};

extern TransferServer g_app;

////////////////////////////////////////////
#endif
