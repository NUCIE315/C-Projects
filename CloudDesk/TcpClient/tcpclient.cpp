#include "tcpclient.h"
#include "./ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "privatechat.h"
#include <QDir>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    resize(500,250);
    loadConfig();

    //信号槽——发送方 发出的信号 接收方 待调用的函数
    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));//服务器连接情况
    connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));

    //链接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

//加载配置文件
void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray badata =  file.readAll();
        QString strData = badata.toStdString().c_str();
        file.close();
        strData.replace("\r\n"," ");
        QStringList strList =  strData.split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() <<"ip:" << m_strIP << "port:"  << m_usPort;
    }
    else
    {
        QMessageBox::critical(this,"open config", "open config file failed");
    }
}

QString TcpClient::getUsrName()
{
    return m_usrName;
}

QString TcpClient::getCurPath()
{
    return m_rootDir;
}

void TcpClient::setCurPath(QString newPath)
{
    m_rootDir = newPath;
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","链接服务器成功");
}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getInstance().getBook()->getIsDownload())
    {
    qDebug() << m_tcpSocket.bytesAvailable(); //发送数据的大小
    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen-sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_RESPOND:
    {
        if(0==strcmp(pdu->caData,REGIST_OK)) {  QMessageBox::information(this,"注册",REGIST_OK);}
        else if(0==strcmp(pdu->caData,REGIST_FAILED)) QMessageBox::critical(this,"注册",REGIST_FAILED);
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(0==strcmp(pdu->caData,LOGIN_OK))
        {
            QMessageBox::information(this,"登录",LOGIN_OK);
            m_rootDir=QString("./%1").arg(m_usrName);
            OpeWidget::getInstance().show();
            this->hide();

        }
        else if(0==strcmp(pdu->caData,LOGIN_FAILED)) QMessageBox::critical(this,"登录",LOGIN_FAILED);
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
    {
        OpeWidget::getInstance().getFriend()->showAllUsr(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
    {
        if(strcmp(pdu->caData,USR_NO_EXIST)==0)
        {
            QMessageBox::information(this,"查找",QString("%1: no exist").arg(OpeWidget::getInstance().getFriend()->nameTemp));
        }
        else if(strcmp(pdu->caData,USR_ONLINE)==0)
        {
             QMessageBox::information(this,"查找",QString("%1: online").arg(OpeWidget::getInstance().getFriend()->nameTemp));
        }
        else if(strcmp(pdu->caData,USR_OFFLINE)==0)
        {
             QMessageBox::information(this,"查找",QString("%1: offline").arg(OpeWidget::getInstance().getFriend()->nameTemp));
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
    {
        if(strcmp(pdu->caData,INPUT_ERROR)==0)
        {
            QMessageBox::information(this,"添加好友","input error！！");
        }
        else if(strcmp(pdu->caData,USR_NO_EXIST)==0)
        {
            QMessageBox::information(this,"添加好友","no exist");
        }
        else if(strcmp(pdu->caData,USR_OFFLINE)==0)
        {
            QMessageBox::information(this,"添加好友","user is offline");
        }
        else if(strcmp(pdu->caData,HAS_BEEN_FRIEND)==0)
        {
            QMessageBox::information(this,"添加好友","you have been friends!");
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char curname[32]={'\0'};
        char prename[32]={'\0'};
        memcpy(curname,pdu->caData,32);
        QMessageBox::StandardButton ret = QMessageBox::information(this,"添加好友",QString("%1 want to add you as friend").arg(curname),
QMessageBox::Yes,QMessageBox::No);

        PDU* rrespdu = mkPDU(0);
        if(ret == QMessageBox::Yes)
        {
            rrespdu->uiMsgType = ENUM_MSG_TYPE_ADD_AGGREE_REQUEST;
        }
        else if(ret == QMessageBox::No)
        {
            rrespdu->uiMsgType = ENUM_MSG_TYPE_ADD_REFUSE_REQUEST;
        }
        memcpy(rrespdu->caData,curname,32);
        memcpy(rrespdu->caData+32,m_usrName.toStdString().c_str(),m_usrName.size());
        TcpClient::getInstance().getTcpSocket().write((char *)rrespdu,rrespdu->uiPDULen);
        free(rrespdu);
        rrespdu =NULL;
        break;

    }
    case ENUM_MSG_TYPE_ADD_AGGREE_RESPOND:
    {
        char prename[32] = {'\0'};
        memcpy(prename,pdu->caData,32);
        QMessageBox::information(this, "添加好友",QString("%1 has aggreed your requiest！").arg(prename));
        break;
    }
    case ENUM_MSG_TYPE_ADD_REFUSE_RESPOND:
    {
        char prename[32] = {'\0'};
        memcpy(prename,pdu->caData,32);
        QMessageBox::information(this, "添加好友",QString("%1 has rejected your requiest！").arg(prename));
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
    {
        qDebug() << "?????";
        OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
    {
        QMessageBox::information(this,"删除好友","删除好友成功");
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {
        char name[32]={'\0'};
        memcpy(name,pdu->caData,32);
        QMessageBox::information(this,"删除好友",QString("%1 已经删除你").arg(name));
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        qDebug()<<"I have receive msg!";
        if(PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
        }
        PrivateChat::getInstance().updateMsg(pdu);
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
    {
        QMessageBox::information(this,"创建文件夹",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_DIR_RESPOND:
    {
        qDebug() << "msgSize1: " << pdu->uiMsgLen;
        QString dirName = OpeWidget::getInstance().getBook()->getDirName();
        if(!dirName.isEmpty())
        {
            QString newPath = QString("%1/%2").arg(getCurPath()).arg(dirName);
            setCurPath(newPath);
            qDebug() << "current path "<<getCurPath();
        }
        OpeWidget::getInstance().getBook()->updateFileList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_DIR_RESPOND:
    {
        QMessageBox::information(this,"删除文件",(char*)pdu->caMsg);
        break;
    }
    case ENUM_MSG_TYPE_RENAME_DIR_RESPOND:
    {
        QMessageBox::information(this,"重命名文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
    {
        QMessageBox::information(this,"进入文件夹",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
    {
        QMessageBox::information(this,"上传文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
    {
        QMessageBox::information(this,"删除文件",(char*)pdu->caMsg);
        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
    {
        char fileName[32] = {'\0'};
        qint64 file_size;
        sscanf(pdu->caData,"%s %lld",fileName,&file_size);

        QString savePath = OpeWidget::getInstance().getBook()->saveFilePath;
        m_file.setFileName(savePath);
        if(m_file.open(QIODeviceBase::WriteOnly))
        {
            OpeWidget::getInstance().getBook()->setIsDownload(true);
            m_iTotal = file_size;
            qDebug() << "download file size: " << m_iTotal;
            m_iReceve = 0;
        }
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
    {
        QMessageBox::information(this,"分享文件",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST:
    {
        char reName[32] = {'\0'};
        memcpy(reName,pdu->caData,32);
        char* pPath = new char[pdu->uiMsgLen+1];
        memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
        qDebug() << "share client2 pPath "<<pPath;
        char *pos = strrchr(pPath,'/');
        qDebug()<< "client share2 " <<pos;
        QString infoStr = QString("%1 share file: %2\n are you receive it?").arg(reName).arg(pos);
        if(QMessageBox::question(this,"共享文件",infoStr)==QMessageBox::StandardButton::Yes)
        {
            PDU *respdu = mkPDU(pdu->uiMsgLen);

            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
            QString curName = getUsrName();
            memcpy(respdu->caData,curName.toStdString().c_str(),curName.size());
            strcpy(respdu->caData+32,pos);
            memcpy((char*)respdu->caMsg,(char*)pdu->caMsg,pdu->uiMsgLen);
            m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
        }
        break;
    }
    case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:
    {
        QMessageBox::information(this,"移动文件",pdu->caData);
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu = NULL;
    }
    else
    {
        QByteArray pbuffer = TcpClient::getInstance().getTcpSocket().readAll();
        m_file.write(pbuffer);
        m_iReceve+=pbuffer.size();
        if(m_iReceve == m_iTotal)
        {
            m_iReceve=0;
            m_iTotal=0;
            OpeWidget::getInstance().getBook()->setIsDownload(false);
            m_file.close();
            QMessageBox::information(this,"下载文件","下载文件成功!");
        }
        else if(m_iReceve > m_iTotal)
        {
            QMessageBox::warning(this,"下载文件","下载文件失败!");
        }

    }
}
#if 0
void TcpClient::on_send_pd_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU* pdu = mkPDU(strMsg.size()+1);
        pdu -> uiMsgType = 8888;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"消息发送","消息为空");
    }
}
#endif

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    m_usrName = strName;
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        pdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_REQUEST;
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this,"登录","登录失败：用户名或用户密码为空");
    }
}


void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        pdu->uiMsgType=ENUM_MSG_TYPE_REGIST_REQUEST;
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this,"用户注册","用户注册失败：用户名或用户密码为空");
    }
}


void TcpClient::on_cancel_pb_clicked()
{

}

