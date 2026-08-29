#include "datastore.h"
#include "SystemConfigManager.h"
#include <QDebug>

DataStore::DataStore()
{
}

void DataStore::readConfigurationFiles()
{
    auto &config = GISApp::Core::SystemConfigManager::instance();
    config.loadConfig();

    client_side_udp_recv_port = config.getUdpListenPort();
    server_side_send_udp_port = config.getUdpSendPort();

    qDebug() << "[DataStore] Updated ports from SystemConfigManager -> Listen:" 
             << client_side_udp_recv_port << "| Send:" << server_side_send_udp_port;
}
