#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if(pdu == NULL) {return;}
    uint numName = pdu->uiMsgLen/32;
    char nameTemp[32];
    for(int i=0; i<numName; i++)
    {
        memcpy(nameTemp,(char*)(pdu->caMsg)+i*32,32);
        ui->online_lw->addItem(nameTemp);
    }
}

void Online::on_addFriend_pb_clicked()
{
    QString prename = ui->online_lw->currentItem()->text(); //获取想添加好友的名字
    QString curname = TcpClient::getInstance().getUsrName(); //当前用户的名字

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,curname.toStdString().c_str(),curname.size());
    memcpy(pdu->caData+32,prename.toStdString().c_str(),prename.size());

    TcpClient::getInstance().getTcpSocket().write((char*) pdu, pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

