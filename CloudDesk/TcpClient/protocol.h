#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef unsigned int uint;

#define REGIST_OK "regist success"
#define REGIST_FAILED "regist failed : name existed"
#define LOGIN_OK "login success"
#define LOGIN_FAILED "login failed : name error or pwd error or relogin"

#define USR_NO_EXIST "user not exist"
#define USR_ONLINE  "user is online"
#define USR_OFFLINE "user is offline"

#define INPUT_ERROR "input error"
#define HAS_BEEN_FRIEND "he has been your friend"

#define REJECTED   "has rejected you"
#define AGGREED    "has aggreed you"

#define DELETE_SUCCESSED  "delete successed"

#define PATH_NO_EXISTE   "path not exist!"
#define DIR_EXIST        "dir has exist!"
#define CREATE_SUCCESSD   "dir create successd"


#define DIR_DELETE_SUCCESSED "you have deleted dir succeed"
#define FILE_IS_REG          "deleted failed: the file you want to delete is reg file"

#define DIR_RENAME_SUCCESSED "file renamed ok!"
#define DIR_RENAME_FAILED    "file renamed failed!"

#define OPEN_FILE_FAILED     "open file failed!!"
#define UPLOAD_FILE_SUCCESSED "upload file successed!!"
#define UPLOAD_FILE_FAILED    "upload file failed!! "



enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST, //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND, //注册回复
    ENUM_MSG_TYPE_LOGIN_REQUEST,  //登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND,  //登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST, //在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND, //在线用户回复
    ENUM_MSG_TYPE_SEARCH_USR_REQUEST, //搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND, //搜索用户回复
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST, //添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND, //添加好友回复
    ENUM_MSG_TYPE_ADD_AGGREE_REQUEST,    //同意添加好友请求
    ENUM_MSG_TYPE_ADD_AGGREE_RESPOND,    //同意添加好友回复
    ENUM_MSG_TYPE_ADD_REFUSE_REQUEST,    //拒绝添加好友请求
    ENUM_MSG_TYPE_ADD_REFUSE_RESPOND,    //拒绝添加好友回复
    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,  //刷新好友列表请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,  //刷新好友列表回复
    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,  //删除好友请求
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,  //删除好友回复
    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,   //私聊请求
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,   //私聊回复
    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,     //群聊请求
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,     //群聊回复

    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,     //创建新文件夹请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,     //创建新文件夹回复
    ENUM_MSG_TYPE_FLUSH_DIR_REQUEST,      //刷新文件夹请求
    ENUM_MSG_TYPE_FLUSH_DIR_RESPOND,      //刷新文件夹回复
    ENUM_MSG_TYPE_DELETE_DIR_REQUEST,     //删除文件夹请求
    ENUM_MSG_TYPE_DELETE_DIR_RESPOND,     //删除文件夹回复
    ENUM_MSG_TYPE_RENAME_DIR_REQUEST,     //重命名文件夹请求
    ENUM_MSG_TYPE_RENAME_DIR_RESPOND,     //重命名文件夹回复
    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,      //进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,      //进入文件夹回复

    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,    //上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,    //上传文件回复
    ENUM_MSG_TYPE_DELETE_FILE_REQUEST,    //删除文件请求
    ENUM_MSG_TYPE_DELETE_FILE_RESPOND,    //删除文件回复
    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,  //下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,  //下载文件回复
    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,     //分享文件请求
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,     //分享文件回复
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST, //分享文件注意请求
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND, //分享文件注意回复
    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,   //移动文件请求
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,   //移动文件回复

    ENUM_MSG_TYPE_MAX = 0x00ffffff
};

struct PDU
{
    uint uiPDULen; //总的协议数据单元大小
    uint uiMsgType; //消息类型；
    char caData[64];
    uint uiMsgLen; //实际消息长度
    int  caMsg[]; //实际消息
};

struct FileInfo
{
    char caName[32];
    int fileType;
};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
