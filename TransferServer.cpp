#include "TransferServer.h"
#include "TransferImp.h"
#include "TransferOrderImp.h"
#include "GR.h"

using namespace std;

TransferServer g_app;

/////////////////////////////////////////////////////////////////
void
TransferServer::initialize()
{
    MysqlUtil::getInstance()->initialize();
    AG->initialize();
    //initialize application here:
    addServant<TransferImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".HelloObj");
    addServant<TransferOrderImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".OrderObj");
    
}
/////////////////////////////////////////////////////////////////
void
TransferServer::destroyApp()
{
    //destroy application here:
    //...
}
/////////////////////////////////////////////////////////////////
int
main(int argc, char* argv[])
{
    try
    {
        g_app.main(argc, argv);
        g_app.waitForShutdown();
    }
    catch (std::exception& e)
    {
        cerr << "std::exception:" << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }
    return -1;
}
////////////////////////////////////////////////////////////////
