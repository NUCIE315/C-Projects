#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    QString getUsrName(); //获取用户名
    QString getCurPath(); //获取当前路径
    void setCurPath(QString newPath);

    static TcpClient& getInstance(); //单例模式
    QTcpSocket& getTcpSocket(); //获取成员TcpSocket

public slots:
    void showConnect();
    void recvMsg();

private slots:
   // void on_send_pd_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;

    //链接服务器，与服务器数据交互
    QTcpSocket m_tcpSocket;

    QString m_usrName; //用户名
    QString m_rootDir; //根目录
    //下载文件时的辅助变量
    qint64 m_iTotal;
    qint64 m_iReceve;

    QFile m_file;
};
#endif // TCPCLIENT_H
