#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "sharefile.h"


Book::Book(QWidget *parent)
    : QWidget{parent}
{
    clearDirName();
    setIsDownload(false);
    //最左侧书列表widget
    m_pBookWidget = new  QListWidget;
    //中间一列文件操作按钮
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDIrPB = new QPushButton("删除文件夹");
    m_pRenameFilePB = new QPushButton("重命名文件");
    m_pFlushDirPB = new QPushButton("刷新文件夹");
    QVBoxLayout  *DirVLayout = new QVBoxLayout;
    DirVLayout->addWidget(m_pReturnPB);
    DirVLayout->addWidget(m_pCreateDirPB);
    DirVLayout->addWidget(m_pDelDIrPB);
    DirVLayout->addWidget(m_pRenameFilePB);
    DirVLayout->addWidget(m_pFlushDirPB);
    //右侧一列文件操作
    m_pUploadFilePB = new QPushButton("上传文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pDownloadFilePB = new QPushButton("下载文件");
    m_pShareFilePB = new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectFilePB = new QPushButton("选择文件");
    m_pSelectFilePB->setEnabled(false);

    QVBoxLayout  *FileVLayout = new QVBoxLayout;
    FileVLayout->addWidget(m_pUploadFilePB);
    FileVLayout->addWidget(m_pDelFilePB);
    FileVLayout->addWidget(m_pDownloadFilePB);
    FileVLayout->addWidget(m_pShareFilePB);
    FileVLayout->addWidget(m_pMoveFilePB);
    FileVLayout->addWidget(m_pSelectFilePB);

    QHBoxLayout *p_main = new QHBoxLayout;
    p_main->addWidget(m_pBookWidget);
    p_main->addItem(DirVLayout);
    p_main->addItem(FileVLayout);

    setLayout(p_main);

    timer = new QTimer();
    connect(m_pCreateDirPB,SIGNAL(clicked(bool)),this,SLOT(createDir()));
    connect(m_pFlushDirPB,SIGNAL(clicked(bool)),this,SLOT(flushDir()));
    connect(m_pDelDIrPB,SIGNAL(clicked(bool)),this,SLOT(delDir()));
    connect(m_pRenameFilePB,SIGNAL(clicked(bool)),this,SLOT(renameDir()));
    connect(m_pBookWidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB,SIGNAL(clicked(bool)),this,SLOT(returnPre()));
    connect(m_pUploadFilePB,SIGNAL(clicked(bool)),this,SLOT(uploadFile()));
    connect(timer,SIGNAL(timeout()),this,SLOT(uploadFileData()));
    connect(m_pDelFilePB,SIGNAL(clicked(bool)),this, SLOT(delFile()));
    connect(m_pDownloadFilePB,SIGNAL(clicked(bool)),this,SLOT(downloadFile()));
    connect(m_pShareFilePB,SIGNAL(clicked(bool)),this,SLOT(shareFile()));
    connect(m_pMoveFilePB,SIGNAL(clicked(bool)),this,SLOT(moveFile()));
    connect(m_pSelectFilePB,SIGNAL(clicked(bool)),this,SLOT(selectFile()));
}

void Book::updateFileList(PDU *pdu)
{
    if(pdu == NULL) return;
    m_pBookWidget->clear();
    int num_file = pdu->uiMsgLen/sizeof(FileInfo);
    qDebug() << num_file;
    qDebug() << "FileInfoSize: " <<sizeof(FileInfo);
    qDebug() << "MsgSize: " << pdu->uiMsgLen;
    for(int i=0; i<num_file; i++)
    {

        QListWidgetItem *newItem = new QListWidgetItem;
        FileInfo* ptr=(FileInfo*)(pdu->caMsg)+i;
        qDebug() << ptr->fileType << " " << ptr->caName;
        //memcpy(ptr,(FileInfo*)(pdu->caMsg)+i,sizeof(FileInfo));
         qDebug() << "client_flush_dir2.1";
        if(ptr->fileType == 0) newItem->setIcon(QIcon(QPixmap(":/cion/dir.svg")));
        else if(ptr->fileType == 1) newItem->setIcon(QIcon(QPixmap(":/cion/reg.svg")));
        newItem->setText(ptr->caName);

        m_pBookWidget->addItem(newItem);
    }
}

void Book::clearDirName()
{
    strDirName.clear();
}

QString Book::getDirName()
{
    return strDirName;
}

bool Book::getIsDownload()
{
    return is_download;
}

void Book::setIsDownload(bool flags)
{
    is_download = flags;
}

QString Book::getFileName()
{
    return m_fileName;
}

void Book::createDir()
{
    QString usrName = TcpClient::getInstance().getUsrName();
    QString curPath = TcpClient::getInstance().getCurPath();
    qDebug() <<"client curPath: " << curPath;
    QString newDirName = QInputDialog::getText(this,"新建文件夹","新建文件夹名称");
    qDebug() << "new dir name: " << newDirName;
    if(newDirName.size() > 32){ QMessageBox::warning(this, "新建文件夹","新文件夹的名字不能超过32字节"); }

    PDU* pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
    memcpy(pdu->caData,usrName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,newDirName.toStdString().c_str(),32);
    memcpy((char*)(pdu->caMsg),curPath.toStdString().c_str(),curPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::flushDir()
{
    QString curPath = TcpClient::getInstance().getCurPath();
    if(curPath.isEmpty()) QMessageBox::warning(this,"刷新文件","路径为空！！！");
    PDU* pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_REQUEST;
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

void Book::delDir()
{
    QString curPath = TcpClient::getInstance().getCurPath();
    QString dirName = m_pBookWidget->currentItem()->text();
    if(curPath.isEmpty() || dirName.isEmpty())
    {
        QMessageBox::warning(this,"删除文件夹","路径或者文件名称为空！！！");
    }
    else
    {
        PDU* pdu = mkPDU(curPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
        memcpy(pdu->caData,dirName.toStdString().c_str(),dirName.size());
        memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::renameDir()
{
    QString pPath = TcpClient::getInstance().getCurPath();
    QString fileName = m_pBookWidget->currentItem()->text();
    QString newName = QInputDialog::getText(this,"重命名文件","文件名");
    if(fileName.isEmpty() || pPath.isEmpty() || newName.isEmpty())
    {
        QMessageBox::warning(this,"重命名","路径为空或未选择文件或新输入的名称为空！！");
    }
    else
    {
        PDU *pdu = mkPDU(pPath.size()+1);
        memcpy(pdu->caData,fileName.toStdString().c_str(),32);
        memcpy(pdu->caData+32,newName.toStdString().c_str(),32);
        memcpy((char*)pdu->caMsg,pPath.toStdString().c_str(),pPath.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_DIR_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString curPath = TcpClient::getInstance().getCurPath();
    QString dirName = index.data().toString();
    strDirName = dirName;
    PDU *pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    memcpy(pdu->caData,dirName.toStdString().c_str(),32);
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::returnPre()
{
    QString curPath = TcpClient::getInstance().getCurPath();
    QString root_path = QString("./%1").arg(TcpClient::getInstance().getUsrName());
    if(curPath == root_path)
    {
        QMessageBox::warning(this,"返回上一层","该文件已经位于最顶层，无法继续返回");
    }
    else
    {
        int index = curPath.lastIndexOf('/');
        QString prePath = curPath.remove(index,curPath.size()-index);
        qDebug()<<"prePath: "<< prePath;
        TcpClient::getInstance().setCurPath(prePath);
        clearDirName();
        flushDir();
    }
}

void Book::uploadFile()
{
    QString pPath = TcpClient::getInstance().getCurPath();
    fileAbsPath = QFileDialog::getOpenFileName();
    if(fileAbsPath.isEmpty())
    {
        QMessageBox::warning(this,"上传文件","上传文件路径为空！！！");
        return;
    }
    int index = fileAbsPath.lastIndexOf('/');
    QString fileName = fileAbsPath.right(fileAbsPath.size()-index-1);
    qDebug() << fileName;
    QFile m_file(fileAbsPath);
    qint64 file_size = m_file.size();

    PDU* pdu = mkPDU(pPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
    // memcpy(pdu->caData,fileName.toStdString().c_str(),32);
    // memcpy((qint64*)pdu->caData,&file_size,sizeof(qint64));
    sprintf(pdu->caData,"%s %lld",fileName.toStdString().c_str(),file_size);
    memcpy((char*)pdu->caMsg,pPath.toStdString().c_str(),pPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;


    timer->start(1000); //计时1s后发送文件

}

void Book::uploadFileData()
{
    timer->stop();
    QFile file(fileAbsPath);
    if(file.open(QFile::ReadOnly))
    {
        char *pBuffer = new char[4096];
        while(true)
        {
            int ret = file.read(pBuffer,4096);
            if(ret>0 && ret<=4096)
            {
                TcpClient::getInstance().getTcpSocket().write(pBuffer,ret);
                //qDebug() << "client ret " << ret;
            }
            else if(ret == 0){break;}
            else {return;}
        }
        file.close();
        delete [] pBuffer;
    }
    else
    {
        QMessageBox::warning(this,"读取文件","读取文件失败！！");
    }
}

void Book::delFile()
{
    QString curPath = TcpClient::getInstance().getCurPath();
    QString dirName = m_pBookWidget->currentItem()->text();
    if(curPath.isEmpty() || dirName.isEmpty())
    {
        QMessageBox::warning(this,"删除文件","路径或者文件名称为空！！！");
    }
    else
    {
        PDU* pdu = mkPDU(curPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
        memcpy(pdu->caData,dirName.toStdString().c_str(),dirName.size());
        memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    QString curPath = TcpClient::getInstance().getCurPath();
    QString fileName = m_pBookWidget->currentItem()->text();
    saveFilePath = QFileDialog::getSaveFileName();
    if(saveFilePath.isEmpty())
    {
        QMessageBox::warning(this,"下载文件","所选择的保存路径为空！！！");
    }
    else
    {

    }

    PDU* pdu = mkPDU(curPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    memcpy(pdu->caData,fileName.toStdString().c_str(),32);
    memcpy((char*)pdu->caMsg,curPath.toStdString().c_str(),curPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;


}

void Book::shareFile()
{
    QListWidget* friendList = OpeWidget::getInstance().getFriend()->getFriendListWidget();
    ShareFile::getInstance().updateFriend(friendList);
    if(ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }
    if( m_pBookWidget->currentItem() == NULL)
    {
        QMessageBox::warning(this,"共享文件","共享文件失败：未选择任何文件");
        return;
    }
    m_fileName = m_pBookWidget->currentItem()->text();
}

void Book::moveFile()
{
    if(m_pBookWidget->currentItem()!=NULL)
    {
        QString fileName = m_pBookWidget->currentItem()->text();
        m_mvfileName = fileName;
        QString oldPath = TcpClient::getInstance().getCurPath();
        m_oldFileName = oldPath+"/"+fileName;
        m_pSelectFilePB->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this,"移动文件","请选择你要移动的文件！！！");
    }
}

void Book::selectFile()
{
    if(m_pBookWidget->currentItem()!=NULL)
    {
        qDebug() << "select file begin";
        QString fileName = m_pBookWidget->currentItem()->text();
        QString newPath = TcpClient::getInstance().getCurPath();
        m_newFileName = newPath+"/"+fileName;
        m_pSelectFilePB->setEnabled(false);
        int len2 = m_newFileName.size();
        int len1 = m_oldFileName.size();
        //qDebug() <<len1 << " "<<len2 << " " <<m_oldFileName;
        int msgLen = len1+len2+2;
        PDU* pdu = mkPDU(msgLen);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%s %d %d",m_mvfileName.toStdString().c_str(), len1,len2) ;
        memcpy((char*)pdu->caMsg,m_oldFileName.toStdString().c_str(),len1);
        memcpy((char*)(pdu->caMsg)+len1+1,m_newFileName.toStdString().c_str(),len2);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::warning(this,"移动文件","请选择你要移动到的文件夹！");
    }
}
