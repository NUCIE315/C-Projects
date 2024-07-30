#include "mytcpsocket.h"
#include "mytcpserver.h"
#include "protocol.h"
#include "opedb.h"
#include <QDir>
#include <QFileInfoList>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    m_upload = false;
    timer = new QTimer;

    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline()));
    connect(timer,SIGNAL(timeout()),this,SLOT(sendFileToClient()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString oldDir, QString newDir)
{
    QDir dir;
    dir.mkdir(newDir);
    dir.setPath(oldDir);
    QFileInfoList filist = dir.entryInfoList();
    for(int i=0; i<filist.count();i++)
    {
        QString newFileName = newDir+"/"+filist[i].fileName();
        QString oldFileName = oldDir+"/"+filist[i].fileName();
        qDebug() << "share dir new " << newFileName << "share dir old " << oldFileName;
        if(filist[i].isFile())
        {
            QFile::copy(oldFileName,newFileName);
        }
        else if(filist[i].isDir())
        {
             if(filist[i].fileName() == '.'||filist[i].fileName()=="..") continue;
             copyDir(oldFileName,newFileName);
        }
    }
}


void MyTcpSocket::recvMsg()
{
    if(!m_upload)
    {
    qDebug() << this->bytesAvailable(); //发送数据的大小
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen-sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char caName[32]={'\0'};
        char caPwd[32]={'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        bool ret = OpeDB::getInstance().handleRegist(caName,caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData,LOGIN_OK);
            QDir dir;
            dir.mkdir(QString("./%1").arg(caName));
        }
        else
        {
            strcpy(respdu->caData,LOGIN_FAILED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char caName[32]={'\0'};
        char caPwd[32]={'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        bool ret = OpeDB::getInstance().handleLogin(caName,caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData,LOGIN_OK);
            m_strName = caName;
        }
        else
        {
            strcpy(respdu->caData,LOGIN_FAILED);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        QStringList qslist = OpeDB::getInstance().handleAllOnline();
        uint msgLength = qslist.size()*32;
        PDU* respdu = mkPDU(msgLength);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i=0; i<qslist.size(); i++)
        {
            memcpy((char*)(respdu->caMsg)+i*32,qslist.at(i).toStdString().c_str(),qslist.at(i).size());
        }
        write((char*) respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
    {
        int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        if(ret == -1)
        {
            strcpy(respdu->caData, USR_NO_EXIST);
        }
        else if(ret == 0)
        {
            strcpy(respdu->caData, USR_OFFLINE);
        }
        else if(ret == 1)
        {
            strcpy(respdu->caData,USR_ONLINE);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu =NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char curName[32]={'\0'};
        char preName[32]={'\0'};
        strncpy(curName,pdu->caData,32);
        strncpy(preName,pdu->caData+32,32);
        int ret = OpeDB::getInstance().handleAddFriend(curName,preName);
        PDU* respdu = mkPDU(0);
        if(ret==-1)
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,INPUT_ERROR);
            write((char*)respdu,respdu->uiPDULen);
        }
        else if(ret == 0)
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,USR_OFFLINE);
            write((char*)respdu,respdu->uiPDULen);
        }
        else if(ret == 1)
        {
            //用户在线
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
            strcpy(respdu->caData,curName);
            MyTcpServer::getInstance().resend(preName,respdu);
        }
        else if(ret == 2)
        {
            respdu->uiMsgType =ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,HAS_BEEN_FRIEND);
            write((char*)respdu,respdu->uiPDULen);
        }
        else if(ret == 3)
        {
            respdu->uiMsgType =ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,USR_NO_EXIST);
            write((char*)respdu,respdu->uiPDULen);
        }
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_AGGREE_REQUEST:
    {
        PDU* rrpdu = mkPDU(0);
        rrpdu->uiMsgType = ENUM_MSG_TYPE_ADD_AGGREE_RESPOND;
        char curname[32] = {'\0'};
        char prename[32] = {'\0'};
        memcpy(curname,pdu->caData,32);
        memcpy(prename,pdu->caData+32,32);
        OpeDB::getInstance().handleUpdateFriends(curname,prename);
        memcpy(rrpdu->caData,prename,32);
        memcpy(rrpdu->caData+32,AGGREED,32);
        MyTcpServer::getInstance().resend(curname,rrpdu);

        free(rrpdu);
        rrpdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_REFUSE_REQUEST:
    {
        PDU* rrpdu = mkPDU(0);
        rrpdu->uiMsgType = ENUM_MSG_TYPE_ADD_REFUSE_RESPOND;
        char curname[32] = {'\0'};
        char prename[32] = {'\0'};
        memcpy(curname,pdu->caData,32);
        memcpy(prename,pdu->caData+32,32);
        memcpy(rrpdu->caData,prename,32);
        memcpy(rrpdu->caData+32,REJECTED,32);
        MyTcpServer::getInstance().resend(curname,rrpdu);
        free(rrpdu);
        rrpdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
    {
        char curname[32];
        memcpy(curname,pdu->caData,32);
        QStringList reList = OpeDB::getInstance().handleFlushFriends(curname);
        int msgLength = reList.size()*32;
        PDU* respdu = mkPDU(msgLength);
        respdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
        for(int i=0; i<reList.size();i++)
        {
            memcpy((char*)(respdu->caMsg)+i*32, reList.at(i).toStdString().c_str(),32);
        }
        write((char*)respdu, respdu->uiPDULen);
        qDebug() << "!!!!";
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {
        char resName[32]={'\0'};
        char friName[32]={'\0'};
        memcpy(resName,pdu->caData,32);
        memcpy(friName,pdu->caData+32,32);
        OpeDB::getInstance().handleDelFriend(resName,friName);
        //发送给请求好友方
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
        memcpy(respdu->caData,DELETE_SUCCESSED,32);
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        //发送给被删好友方
        PDU *resdelpdu = mkPDU(0);
        resdelpdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(resdelpdu->caData,resName,32);
        MyTcpServer::getInstance().resend(friName,resdelpdu);
        free(resdelpdu);
        resdelpdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        char chatName[32] = {'\0'};
        memcpy(chatName,pdu->caData+32,32);
        qDebug() << chatName;
        MyTcpServer::getInstance().resend(chatName,pdu);
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        char loName[32] = {'\0'};
        memcpy(loName,pdu->caData,32);
        QStringList list=OpeDB::getInstance().handleFlushFriends(loName);
        for(int i=0; i<list.size(); i++)
        {
            QString sname = list.at(i);
            MyTcpServer::getInstance().resend(sname,pdu);
        }
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
    {
        QDir dir;
        QString curPath = QString("%1").arg((char*) pdu->caMsg);
        qDebug()<<"curPath "<< curPath;
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
        if(!curPath.isEmpty())
        {
            char newDirName[32]={'\0'};
            memcpy(newDirName,pdu->caData+32,32);
            QString newPath=QString("%1/%2").arg(curPath).arg(newDirName);
            qDebug()<<"newPath "<< newPath;
            bool ret = dir.mkdir(newPath);
            if(ret)
            {
                strcpy(respdu->caData,CREATE_SUCCESSD);
            }
            else
            {
                strcpy(respdu->caData,DIR_EXIST);
            }
        }
        else
        {
            qDebug() << PATH_NO_EXISTE;
            strcpy(respdu->caData,PATH_NO_EXISTE);
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_DIR_REQUEST:
    {

        char *path = new char[pdu->uiMsgLen+1];
        memcpy(path,(char*)(pdu->caMsg),pdu->uiMsgLen);
        qDebug() << path;
        QDir dir(path);
        QFileInfoList ret = dir.entryInfoList();
        int file_num = ret.size();
        FileInfo* ptr = NULL;
        PDU* respdu = mkPDU(sizeof(FileInfo)*file_num);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_RESPOND;
        qDebug() << file_num;
        qDebug() << "FileInfoSize: " <<sizeof(FileInfo);
        qDebug() << "MsgSize: " << respdu->uiMsgLen;
        for(int i=0; i<file_num;i++)
        {
            QString fileName = ret.at(i).fileName();
            ptr = (FileInfo*)(respdu->caMsg) + i;
            memcpy(ptr->caName,fileName.toStdString().c_str(),fileName.size());
            if(ret.at(i).isDir())
            {
                ptr->fileType = 0;
            }
            else if(ret.at(i).isFile())
            {
                ptr->fileType = 1;
            }

        }
        qDebug() << "MsgSize11: " << respdu->uiPDULen;
        write((char*)respdu,respdu->uiPDULen);
        qDebug() << "server_flush_dir3";
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_DIR_REQUEST:
    {
        char dirName[32] = {'\0'};
        char path[pdu->uiMsgLen+1];
        memcpy(dirName,pdu->caData,32);
        memcpy(path,(char*)pdu->caMsg,pdu->uiMsgLen);
        QString pPath = QString("%1/%2").arg(path).arg(dirName);
        qDebug() << "del path: " << pPath;
        QFileInfo fileIf(pPath);
        bool ret=false;
        if(fileIf.isDir())
        {
            QDir dir;
            dir.setPath(pPath);
            ret=dir.removeRecursively();
        }
        else
        {
            ret = false;
        }
        PDU* respdu = NULL;
        if(ret == true)
        {
            uint len = strlen(DELETE_SUCCESSED)+1;
            respdu = mkPDU(len);
            memcpy((char*)respdu->caMsg,DELETE_SUCCESSED,strlen(DELETE_SUCCESSED));
        }
        else
        {
            uint len = strlen(FILE_IS_REG)+1;
            respdu = mkPDU(len);
            memcpy((char*)respdu->caMsg,FILE_IS_REG,strlen(FILE_IS_REG));
        }
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu =NULL;
        break;
    }
    case ENUM_MSG_TYPE_RENAME_DIR_REQUEST:
    {
        char fileName[32]={'\0'};
        char newName[32] = {'\0'};
        char path[pdu->uiMsgLen+1];
        memcpy(fileName,pdu->caData,32);
        memcpy(newName,pdu->caData+32,32);
        memcpy(path,(char*)pdu->caMsg,pdu->uiMsgLen);
        QString oldFilePath = QString("%1/%2").arg(path).arg(fileName);
        QString newFilePath = QString("%1/%2").arg(path).arg(newName);
        qDebug()<< "old " << oldFilePath << "new " << newFilePath;
        QDir dir;
        bool ret=dir.rename(oldFilePath,newFilePath);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_DIR_RESPOND;
        if(ret)
        {
            memcpy(respdu->caData,DIR_RENAME_SUCCESSED,strlen(DIR_RENAME_SUCCESSED));
        }
        else
        {
            memcpy(respdu->caData,DIR_RENAME_FAILED,strlen(DIR_RENAME_FAILED));
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
    {
        char dirName[32] = {'\0'};
        char curPath[pdu->uiMsgLen+1];
        memcpy(dirName,pdu->caData,32);
        memcpy(curPath,(char*)pdu->caMsg,pdu->uiMsgLen);
        QString pPath = QString("%1/%2").arg(curPath).arg(dirName);
        qDebug() << "enter path: " << pPath;
        QFileInfo fileInfo(pPath);
        PDU *respdu = NULL;
        if(fileInfo.isDir())
        {
            QDir dir(pPath);
            QFileInfoList ret = dir.entryInfoList();
            int file_num = ret.size();
            FileInfo* ptr = NULL;
            respdu = mkPDU(sizeof(FileInfo)*file_num);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_RESPOND;
            qDebug() << file_num;
            for(int i=0; i<file_num;i++)
            {
                //qDebug() << "enter dir begin: " << i;
                QString fileName = ret.at(i).fileName();
                ptr = (FileInfo*)(respdu->caMsg) + i;
                memcpy(ptr->caName,fileName.toStdString().c_str(),fileName.size());
                if(ret.at(i).isDir())
                {
                    ptr->fileType = 0;
                }
                else if(ret.at(i).isFile())
                {
                    ptr->fileType = 1;
                }
               //qDebug() << "enter dir end: " << i;
            }
        }
        else
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
            memcpy(respdu->caData,DIR_ENTER_FAILED,strlen(DIR_ENTER_FAILED));
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        qDebug() << "server ok!!";
        break;
    }
    case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
    {
        char pPath[pdu->uiMsgLen+1];
        char fileName[32];
        qint64 file_size;
        sscanf(pdu->caData,"%s %lld",fileName,&file_size);
        memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
        QString curPath = QString("%1/%2").arg(pPath).arg(fileName);
        qDebug() << "upload file "<<curPath;
        m_file.setFileName(curPath);
        if(m_file.open(QFile::WriteOnly))
        {
            m_upload = true;
            m_iTotal = file_size;
            m_iRecved = 0;
            //m_file.close();
        }
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
    {
        char dirName[32] = {'\0'};
        char path[pdu->uiMsgLen+1];
        memcpy(dirName,pdu->caData,32);
        memcpy(path,(char*)pdu->caMsg,pdu->uiMsgLen);
        QString pPath = QString("%1/%2").arg(path).arg(dirName);
        qDebug() << "del path: " << pPath;
        QFileInfo fileIf(pPath);
        bool ret=false;
        if(fileIf.isFile())
        {
            QDir dir;
            //dir.setPath();
            ret=dir.remove(pPath);
        }
        else
        {
            ret = false;
        }
        PDU* respdu = NULL;
        if(ret == true)
        {
            uint len = strlen(DELETE_SUCCESSED)+1;
            respdu = mkPDU(len);
            memcpy((char*)respdu->caMsg,DELETE_SUCCESSED,strlen(DELETE_SUCCESSED));
        }
        else
        {
            uint len = strlen(FILE_IS_REG)+1;
            respdu = mkPDU(len);
            memcpy((char*)respdu->caMsg,FILE_IS_REG,strlen(FILE_IS_REG));
        }
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu =NULL;
        break;
    }
    case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
    {
        char caName[32] = {'\0'};
        char pPath[pdu->uiMsgLen+1];
        memcpy(caName,pdu->caData,32);
        memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);
        QString filePath = QString("%1/%2").arg(pPath).arg(caName);
        QFileInfo fInfo(filePath);
        qint64 file_size = fInfo.size();
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
        sprintf(respdu->caData,"%s %lld",caName,file_size);
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        qDebug() << "downloda filePath " << filePath;
        m_file.setFileName(filePath);
        if(!m_file.open(QIODeviceBase::ReadOnly))
        {
            qDebug() << "打开文件失败";
        }
        timer->start(1000);

        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
    {
        qDebug() << "共享可达 服务器";
        int num_people;
        char reName[32] = {'\0'};
        sscanf(pdu->caData,"%s %d",reName,&num_people);
        int size=pdu->uiMsgLen-num_people*32;
        qDebug() << "server1 num_people "<< num_people;
        //qDebug() << "server1 receive caMsg " << (char*)pdu->caMsg;
        PDU* respdu = mkPDU(size+1); //仅仅需要发送路径信息即可
        respdu ->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST;
        memcpy((char*)respdu->caMsg,(char*)pdu->caMsg+num_people*32,size);
        qDebug() << "server1 send caMsg " << (char*)pdu->caMsg;
        strcpy(respdu->caData,reName);
        for(int i=0; i<num_people; i++)
        {
            char ceName[32]={'\0'};
            strcpy(ceName,(char*)pdu->caMsg+i*32);
            MyTcpServer::getInstance().resend(ceName,respdu);
        }
        respdu = NULL;
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
        memcpy(respdu->caData,"files shared successed!!",strlen("files shared successed!!"));
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
    {
        char ceName[32]={'\0'};
        char fileName[32]={'\0'};
        memcpy(ceName,pdu->caData,32);
        memcpy(fileName,pdu->caData+32,32);
        qDebug()<< "server share " <<fileName;
        QString newPath = QString("./%1%2").arg(ceName).arg(fileName);
        qDebug() << "share new path: " << newPath;
        char *oldPath = new char[pdu->uiMsgLen+1];
        memcpy(oldPath,(char*)pdu->caMsg,pdu->uiMsgLen);
        QFileInfo fileInfo(oldPath);
        if(fileInfo.isFile())
        {
            QFile::copy(oldPath,newPath);
        }
        else if(fileInfo.isDir())
        {
            copyDir(oldPath,newPath);
        }

        break;
    }
    case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
    {
        qDebug() << "I am here";
        char filName[32]={'\0'};
        int len1,len2;
        sscanf(pdu->caData,"%s%d%d",filName,&len1,&len2);
        qDebug() << len1 <<" " << len2;
        char *oldFileName = new char[len1+1];
        char *newFileName = new char[len2+1+32];
        // qDebug() << "I am here 1";
        memset(oldFileName,'\0',len1+1);
        memset(newFileName,'\0',len2+1+32);
        memcpy(oldFileName,(char*)pdu->caMsg,len1);
        memcpy(newFileName,(char*)(pdu->caMsg)+len1+1,len2);
        // qDebug() << "I am here 2";
        QFileInfo flinfo(newFileName);
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
        qDebug() << "move file old file name: "<< oldFileName <<"new file name: " << newFileName;
        if(flinfo.isDir())
        {
            newFileName = strcat(newFileName,"/");
            newFileName = strcat(newFileName,filName);
            int ret = QFile::rename(oldFileName,newFileName);
            if(ret)
            {
                strcpy(respdu->caData,REMOVE_SUCCEED);
            }
            else
            {
                strcpy(respdu->caData,COMMON_ERROR);
            }
        }
        else
        {
              strcpy(respdu->caData,NOT_IS_DIR);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        delete [] oldFileName;
        delete [] newFileName;
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
        QByteArray pbuffer = readAll();
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        //qDebug() <<"fileName: " <<m_file.fileName();
        /*if(m_file.open(QFile::WriteOnly))
        {*/

            m_file.write(pbuffer);
            m_iRecved+=pbuffer.size();
            qDebug() << "iTotal " << m_iTotal << "rec " << m_iRecved;
            if(m_iRecved == m_iTotal )
            {
                m_upload = false;
                memcpy(respdu->caData,UPLOAD_FILE_SUCCESSED,strlen(UPLOAD_FILE_SUCCESSED));
                write((char*)respdu,respdu->uiPDULen);
                m_file.close();
            }
            else if(m_iRecved > m_iTotal)
            {
                memcpy(respdu->caData,UPLOAD_FILE_FAILED,strlen(UPLOAD_FILE_FAILED));
                write((char*)respdu,respdu->uiPDULen);
            }
        //}

        free(respdu);
        respdu = NULL;
    }
    //qDebug() << caName << caPwd << pdu->uiMsgType;
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::sendFileToClient()
{
    timer->stop();
     qDebug() << "文件传送开始！";
    char *pbuffer = new char[4096];
    while(true)
    {
        int ret = m_file.read(pbuffer,4096);
        if(ret>0 && ret <= 4096) write(pbuffer,ret);
        else if(ret==0)
        {
            qDebug() << "文件传送完成！";

            break;
        }
    }
    m_file.close();
    delete [] pbuffer;
}
