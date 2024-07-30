#include "friend.h"
#include "protocol.h"
#include "TcpClient.h"
#include <QInputDialog>
#include <QMessageBox>
#include "privatechat.h"

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit;
    m_pFrinedListWidget = new QListWidget;
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new  QPushButton("删除");
    m_pFlushFriendPB = new  QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new  QPushButton("显式在线用户");
    m_pSearchUsrPB = new  QPushButton("查找用户");
    m_pMsgSendPB = new  QPushButton("信息发送");;
    m_pPrivateChatPB = new  QPushButton("私聊");;

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL ->addWidget(m_pShowMsgTE);
    pTopHBL ->addWidget(m_pFrinedListWidget);
    pTopHBL ->addLayout(pRightPBVBL);

    //pTopHBL ->addWidget(m_pOnline);
    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;
    QVBoxLayout *pMain = new QVBoxLayout;
    //pMain->addWidget(m_pOnline);
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
   // pMain->addWidget(m_pOnline);
    //m_pOnline->hide();


    setLayout(pMain);

    connect(m_pShowOnlineUsrPB,SIGNAL(clicked(bool)),this,SLOT(showOnline()));
    connect(m_pSearchUsrPB,SIGNAL(clicked(bool)),this,SLOT(searchUsr()));
    connect(m_pFlushFriendPB,SIGNAL(clicked(bool)),this,SLOT(flushFriends()));
    connect(m_pDelFriendPB,SIGNAL(clicked(bool)),this,SLOT(delFriend()));
    connect(m_pPrivateChatPB,SIGNAL(clicked(bool)),this,SLOT(privateChat()));
    connect(m_pMsgSendPB,SIGNAL(clicked(bool)),this,SLOT(groupChat()));
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden())
    {
        m_pOnline->show();

        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    nameTemp = QInputDialog::getText(this,"搜索","用户名");
    if(!nameTemp.isEmpty())
    {
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        memcpy(pdu->caData,nameTemp.toStdString().c_str(),nameTemp.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Friend::flushFriends()
{
    QString name = TcpClient::getInstance().getUsrName();
    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData,name.toStdString().c_str(),name.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::delFriend()
{
    if(NULL != m_pFrinedListWidget->currentItem())
    {
        QString friName = m_pFrinedListWidget->currentItem()->text();
        QString myName = TcpClient::getInstance().getUsrName();
        PDU* pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(pdu->caData,myName.toStdString().c_str(),32);
        memcpy(pdu->caData+32,friName.toStdString().c_str(),32);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Friend::privateChat()
{
    if(m_pFrinedListWidget->currentItem())
    {
        QString name = m_pFrinedListWidget->currentItem()->text();
        PrivateChat::getInstance().setChatName(name);
        if(PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
        }
    }
    else
    {
        QMessageBox::warning(this,"聊天对象","你没有选择聊天对象");
    }
}

void Friend::groupChat()
{
    QString msg = m_pInputMsgLE->text();
    if(!msg.isEmpty())
    {
        QString logname = TcpClient::getInstance().getUsrName();
        PDU* pdu = mkPDU(msg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        memcpy(pdu->caData,logname.toStdString().c_str(),logname.size());
        memcpy((char*)(pdu->caMsg),msg.toStdString().c_str(),msg.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"发送消息","消息不能为空！！！");
    }
}

void Friend::showAllUsr(PDU *pdu)
{
    if(NULL == pdu){qDebug()<<"NULL"; return;}
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    m_pFrinedListWidget->clear();
    if(pdu == NULL) { return;}
    int num_name = pdu->uiMsgLen/32;
    for(int i=0; i<num_name; i++)
    {
        char name[32] = {'\0'};
        memcpy(name,(char*)(pdu->caMsg)+32*i,32);
        qDebug()<<name;
        m_pFrinedListWidget->addItem(name);
    }
}

void Friend::updateGroupMsg(PDU *pdu)
{
    char name[32] = {'\0'};
    memcpy(name,pdu->caData,32);
    QString newMsg = QString("%1 says:%2").arg(name).arg((char*)pdu->caMsg);
    m_pShowMsgTE->append(newMsg);
}

QListWidget* Friend::getFriendListWidget()
{
    return m_pFrinedListWidget;
}
