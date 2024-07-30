#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include "online.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);

    void showAllUsr(PDU *pdu);

    void updateFriendList(PDU *pdu);
    void updateGroupMsg(PDU* pdu);
    QListWidget* getFriendListWidget();

    QString nameTemp;

signals:

public slots:
 void showOnline();
 void searchUsr();
 void flushFriends();
 void delFriend();
 void privateChat();
 void groupChat();


private:
    QTextEdit *m_pShowMsgTE;
    QListWidget *m_pFrinedListWidget;
    QLineEdit *m_pInputMsgLE;

    QPushButton *m_pDelFriendPB;
    QPushButton *m_pFlushFriendPB;
    QPushButton *m_pShowOnlineUsrPB;
    QPushButton *m_pSearchUsrPB;
    QPushButton *m_pMsgSendPB;
    QPushButton *m_pPrivateChatPB;

    Online *m_pOnline;
};

#endif // FRIEND_H
