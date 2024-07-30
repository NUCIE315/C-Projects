#include "sharefile.h"
#include <QMessageBox>
#include "tcpclient.h"
ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAll = new QPushButton("全部选择");
    m_pCancleAll = new QPushButton("全部取消");

    m_pEnsure = new QPushButton("确认");
    m_pCancle = new QPushButton("取消");

    m_pFriendSA = new QScrollArea;
    m_pFriendWD = new QWidget;
    m_pFriendBG = new QButtonGroup;
    m_pFriendBG->setExclusive(false);
    //m_qvb = new QVBoxLayout;
    m_qvb = new QVBoxLayout(m_pFriendWD);

    QVBoxLayout *qhtop  = new QVBoxLayout;
    qhtop->addWidget(m_pSelectAll);
    qhtop->addWidget(m_pCancleAll);
    qhtop->addStretch();

    QVBoxLayout *qbutton = new QVBoxLayout;
    qhtop->addWidget(m_pEnsure);
    qhtop->addWidget(m_pCancle);

    QHBoxLayout *p_main = new QHBoxLayout;
     p_main->addWidget(m_pFriendSA);
    p_main->addItem(qhtop);
    p_main->addItem(qbutton);

    setLayout(p_main);

    connect(m_pCancleAll,SIGNAL(clicked(bool)),this,SLOT(cancleAll()));
    connect(m_pSelectAll,SIGNAL(clicked(bool)),this,SLOT(selectAll()));
    connect(m_pEnsure,SIGNAL(clicked(bool)),this,SLOT(ensure()));
    connect(m_pCancle,SIGNAL(clicked(bool)),this,SLOT(cancle()));
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile sharefile;
    return sharefile;
}

void ShareFile::updateFriend(QListWidget *friendList)
{
    if(friendList == NULL)
    {
        QMessageBox::warning(this,"好友列表","可共享好友列表为空");
    }
    //删除列表中原有的信息
    QAbstractButton *tmp = NULL;
    QList<QAbstractButton*> preFriendList = m_pFriendBG->buttons();
    for(int i=0; i<preFriendList.size();i++)
    {
        tmp = preFriendList[i];
        m_qvb->removeWidget(tmp);
        m_pFriendBG->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    qDebug() <<"share file " << friendList->count();
    //向列表中添加按钮
    QCheckBox *pCB = NULL;
    for(int i=0; i<friendList->count();i++)
    {
        pCB = new QCheckBox(friendList->item(i)->text());
        m_qvb->addWidget(pCB);
        m_pFriendBG->addButton(pCB);
    }
    m_pFriendSA->setWidget(m_pFriendWD);
}

void ShareFile::cancleAll()
{
    QList<QAbstractButton *> buttonList = m_pFriendBG->buttons();
    for(int i=0; i<buttonList.count(); i++)
    {
        if(buttonList[i]->isChecked())
        {
            buttonList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton *> buttonList = m_pFriendBG->buttons();
    for(int i=0; i<buttonList.count(); i++)
    {
        if(!buttonList[i]->isChecked())
        {
            buttonList[i]->setChecked(true);
        }
    }
}

void ShareFile::ensure()
{
    QString reName = TcpClient::getInstance().getUsrName();
    QString curPath = TcpClient::getInstance().getCurPath();
    QString fileName = OpeWidget::getInstance().getBook()->getFileName();
    QString pPath = QString("%1/%2").arg(curPath).arg(fileName);
   // qDebug() << "client1 pPath " << pPath << " "<<pPath.size();

    int num = 0;
    QList<QAbstractButton *> buttonList = m_pFriendBG->buttons();
    for(int i=0; i<buttonList.count(); i++)
    {
        if(buttonList[i]->isChecked())
        {
            num++;
        }
    }

    PDU *pdu = mkPDU(num*32+pPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",reName.toStdString().c_str(),num);
    int j=0;
    for(int i=0; i<buttonList.count();i++)
    {
        if(buttonList[i]->isChecked())
        {
            memcpy((char*)pdu->caMsg+j*32,buttonList[i]->text().toStdString().c_str(),buttonList[i]->text().size());
        }
    }
    memcpy((char*)(pdu->caMsg)+num*32,pPath.toStdString().c_str(),pPath.size());
    //qDebug() << "client1 send caMsg " << (char*)pdu->caMsg << " " << (char*)(pdu->caMsg)+num*32;
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void ShareFile::cancle()
{
    hide();
}

