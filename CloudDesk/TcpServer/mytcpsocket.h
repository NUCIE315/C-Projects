#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include <QFile>
#include <QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();
    void copyDir(QString oldDir,QString newDir);
signals:
    void offline(MyTcpSocket *mysocket);
public slots:
    void recvMsg();
    void clientOffline(); //处理客户端下线
    void sendFileToClient(); //发送文件到客户端

private:
    QString m_strName;

    //用于文件上传的四个辅助函数
    QFile m_file;    //辅助文件操作
    qint64 m_iTotal; //总的字节数
    qint64 m_iRecved; //现在已经接收的字节数
    bool m_upload;  //是否正在上传

    QTimer *timer;
};

#endif // MYTCPSOCKET_H
