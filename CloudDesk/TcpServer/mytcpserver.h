#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT //支持信号槽
public:
    MyTcpServer();

    //单例模式
    static MyTcpServer &getInstance();
    //虚函数重载
    void incomingConnection(qintptr socketDescriptor);

    //将信息重新发送给好友待添加方
    void resend(QString prename, PDU* respdu);
public slots:
    void deleteSocket(MyTcpSocket *mysocket);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
