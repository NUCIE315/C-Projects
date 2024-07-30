#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QScrollArea>
#include <QCheckBox>
#include <QListWidget>

class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);
    static ShareFile& getInstance();
    void updateFriend(QListWidget* friendList);

signals:

public slots:
    void cancleAll();
    void selectAll();
    void ensure();
    void cancle();

private:
    QPushButton *m_pSelectAll;
    QPushButton *m_pCancleAll;

    QPushButton *m_pEnsure;
    QPushButton *m_pCancle;

    QScrollArea *m_pFriendSA;
    QWidget *m_pFriendWD;
    QButtonGroup *m_pFriendBG;
    QVBoxLayout *m_qvb;
};

#endif // SHAREFILE_H
