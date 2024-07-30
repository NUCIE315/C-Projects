#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\Work\\4ProjectTest\\1CloudDesk\\Projects\\QT\\TcpServer\\cloud.db");
    if(m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while(query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            //ui->textEdit->append(data);
            qDebug() << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(name == NULL || pwd==NULL) return false;
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(name == NULL || pwd==NULL) return false;
    QString data = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        data = QString("update usrInfo set online=1 where name = \'%1\' and pwd = \'%2\'").arg(name).arg(pwd);
        QSqlQuery query;
        bool res = query.exec(data);
        //qDebug() << "update faile? " << res;
        return true;
    }
    else
    {
        //qDebug() << "select failed！ " ;
        return false;
    }

}

void OpeDB::handleOffline(const char *name)
{
    if(name == NULL) return;
    QString data = QString("update usrInfo set online=0 where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(data);

    QStringList result;
    result.clear();
    while(query.next())
    {
        qDebug() << "ddd " << query.value(0).toString();
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(QString name)
{
    QString data = QString("select online from usrInfo where name='\%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);

    if(query.next())
    {
        int ret = query.value(0).toInt();
        //qDebug() << "is online " << ret;
        return ret;
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *curname, const char *prename)
{
    if(curname == NULL || prename == NULL) { return -1; }
    QString data = QString("select * from friend where(id=(select id from usrInfo where name= \'%1\')"
                           " and friendId = (select id from usrInfo where name = \'%2\'))"
                           " or (id = (select id from usrInfo where name =\'%3\')"
                           " and friendId=(select id from usrInfo where name = \'%4\'))")
                              .arg(curname).arg(prename).arg(prename).arg(curname);
    qDebug()<<data;
    QSqlQuery query;
    query.exec(data);

    if(query.next()){return 2; } //说明两人已经是好友了
    else
    {
    data = QString("select online from usrInfo where name='\%1\'").arg(prename);
    query.exec(data);

    if(query.next())
    {
        return query.value(0).toInt(); //判断是否在线
    }
    else
    {
        return 3; //不存在该用户
    }
    }
}

void OpeDB::handleUpdateFriends(const char *curname, const char *prename)
{
    if(curname == NULL || prename == NULL) return;
    //查询
    QString data1 =QString("select id from usrInfo where name = '\%1\' ").arg(curname);
    qDebug() << data1;
    QSqlQuery query;
    query.exec(data1);
    query.next();
    QString id = query.value(0).toString();
    QString data2 =QString("select id from usrInfo where name = '\%1\' ").arg(prename);
    query.exec(data2);
    query.next();
    QString friendId = query.value(0).toString();
    qDebug() << id << " " << friendId;
    QString data=QString("insert into friend(id,friendID) values( '\%1\','\%2\')").arg(id).arg(friendId);
    query.exec(data);
}

QStringList OpeDB::handleFlushFriends(const char *name)
{
    QStringList resList;
    resList.clear();
    if(name == NULL) return resList;
    QString data = QString("select name from usrInfo where online=1 and"
                           " id in (select id from friend where friendId in"
                           " (select id from usrInfo where name=\'%1\'))").arg(name);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    while(query.next())
    {
        resList.append(query.value(0).toString());
        qDebug() << query.value(0).toString();
    }
    data = QString("select name from usrInfo where online=1 and"
                   " id in (select friendId from friend where id in"
                   " (select id from usrInfo where name=\'%1\'))").arg(name);
    qDebug() << data;
    query.exec(data);
    while(query.next())
    {
        resList.append(query.value(0).toString());
        qDebug() << query.value(0).toString();
    }
    return resList;
}

void OpeDB::handleDelFriend(const char *resName, const char *friName)
{
    qDebug() << resName <<" " << friName;
    if(NULL == resName || NULL == friName) return;
    QString data = QString("delete from friend where id=(select id from usrInfo where name = '\%1\') and friendId = (select id from usrInfo where name = \'%2\') ").arg(resName).arg(friName);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    data = QString("delete from friend where friendId=(select id from usrInfo where name = '\%1\') and id = (select id from usrInfo where name = \'%2\') ").arg(resName).arg(friName);
    qDebug() << data;
    query.exec(data);
}

