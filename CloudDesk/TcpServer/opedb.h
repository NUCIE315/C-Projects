#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase> //链接数据库
#include <QSqlQuery>    //查询数据库
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();
    void init();
    ~OpeDB();

    //处理注册，将收到的用户名和密码写入到数据库中
    bool handleRegist(const char *name, const char* pwd);
    bool handleLogin(const char *name, const char* pwd); //处理查询登录用户时的信息
    void handleOffline(const char* name); //数据库操作处理用户下线时的操作
    QStringList handleAllOnline(); //查询所有在线用户
    int handleSearchUsr(QString name); //根据名字查询用户是否存在及在线？
    int handleAddFriend(const char* curname, const char* prename); //输入当前用户名与待添加用户名，处理好友操作
    void handleUpdateFriends(const char* curname, const char* prename); //将两个名字插入到friend列表中
    QStringList handleFlushFriends(const char* name); //数据库操作好友列表刷新
    void handleDelFriend(const char* resName, const char* friName);

signals:

private:
    QSqlDatabase m_db; //连接数据库
};

#endif // OPEDB_H
