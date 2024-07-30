#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include <QFile>
#include <QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(PDU* pdu);
    void clearDirName();
    QString getDirName();
    bool getIsDownload();
    void setIsDownload(bool flags);
    QString getFileName();


    QString saveFilePath;

signals:

public slots:
    void createDir();
    void flushDir();
    void delDir();
    void renameDir();
    void enterDir(const QModelIndex &index);
    void returnPre();
    void uploadFile();
    void uploadFileData();
    void delFile();
    void downloadFile();
    void shareFile();
    void moveFile();
    void selectFile();

private:
    QListWidget *m_pBookWidget;

    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDIrPB;
    QPushButton *m_pRenameFilePB;
    QPushButton *m_pFlushDirPB;

    QPushButton *m_pUploadFilePB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pDownloadFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectFilePB;

    QString strDirName;
    QString fileAbsPath;
    QTimer *timer;

    QString m_fileName;
    QString m_oldFileName;
    QString m_newFileName;
    QString m_mvfileName;

    bool is_download;
};

#endif // BOOK_H
