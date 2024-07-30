#include "privatechat.h"
#include "ui_privatechat.h"
#include <QMessageBox>
#include "tcpclient.h"
#include "protocol.h"

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat privateChat;
    return privateChat;
}

void PrivateChat::setChatName(QString chatName)
{
    m_strChatName = chatName;
}

void PrivateChat::updateMsg(PDU *pdu)
{
    char reName[32] = {'\0'};
    memcpy(reName,pdu->caData,32);
    m_strChatName = reName;
    QString newMsg = QString("%1 says: %2 ").arg(reName).arg((char*)(pdu->caMsg));
    ui->showMsg_te->append(newMsg);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString inputMsg = ui->inputMsg_le->text();
    if(!inputMsg.isEmpty())
    {
        m_strLoginName = TcpClient::getInstance().getUsrName();
        PDU *pdu = mkPDU(inputMsg.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData,m_strLoginName.toStdString().c_str(),m_strLoginName.size());
        memcpy(pdu->caData+32,m_strChatName.toStdString().c_str(),m_strChatName.size());
        memcpy((char*)(pdu->caMsg),inputMsg.toStdString().c_str(),inputMsg.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
        ui->inputMsg_le->clear();
        //ui->showMsg_te->setAlignment(Qt::AlignRight);
        ui->showMsg_te->append(QString("                                     %1").arg(inputMsg));
    }
    else
    {
        QMessageBox::warning(this,"输入内容","输入内容为空！");
    }
}

