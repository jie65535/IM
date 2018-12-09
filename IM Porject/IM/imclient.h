#ifndef IMCLIENT_H
#define IMCLIENT_H

#include <QObject>
#include <QtNetwork>
#include <QString>
#include <QVector>
#include "immessage.h"

/***********************************
 *
 * Class IMClient
 * IM客户端类
 *
 * 用于与服务端交互的封装类
 *
 * 公开方法有：
 * login                登录
 * sendPrivateMessage   发送私聊消息
 * sendGroupMessage     发送群聊消息
 *
 * 发出的信号有：
 * receivedPrivateMessage   接收到私聊消息信号
 * receivedGroupMessage     接收到群聊消息信号
 * userOnline               用户上线信号
 * userOffline              用户下线信号
 * serverClose              服务器关闭信号
 *
 **********************************/
/**
 * @brief IM客户端类
 */
class IMClient : public QObject
{
    Q_OBJECT
// 公开的成员函数
protected:
    explicit IMClient(QObject *parent = nullptr);

public:
    /**
     * @brief instance 单例对象
     * @return 单例对象指针
     */
    static IMClient *instance();

    ~IMClient();

    /**
     * @brief connectServer 尝试连接服务器
     * @param hostName 连接名
     */
    void connectServer(QString hostName);

    /**
     * @brief login 登录
     * @param name 用户名
     */
    void login(QString name);

    /**
     * @brief sendPrivateMessage 发送私聊消息
     * @param toName 发送给谁
     * @param content 消息内容
     */
    void sendPrivateMessage(QString toName, QString content);

    /**
     * @brief sendGroupMessage 发送群聊消息
     * @param content 消息内容
     */
    void sendGroupMessage(QString content);

    /**
     * @brief isOpen 连接是否打开
     * @return
     */
    bool isOpen()
    {
        return this->m_socket != nullptr
                && this->m_socket->state() != QAbstractSocket::UnconnectedState;
    }


public:
    const QVector<IMMessage> *getChatRecord(QString name);
    const QVector<IMMessage> *getGroupChatRecord() { return &m_gruopChatRecord; }
    const QVector<QString> *getOnlineList() { return &m_online; }
    const QVector<QString> *getOfflineList() { return &m_offline; }
    QString getName() { return m_name; }

// 信号
signals:
    /**
     * @brief loginResult 登录结果
     * @param isSuccess 是否登录成功
     */
    void loginResult(bool isSuccess);

    /**
     * @brief receivedPrivateMessage 接收到私聊消息信号
     * @param msg 消息内容
     */
    void receivedPrivateMessage(IMMessage msg);

    /**
     * @brief receivedGroupMessage 接收到群聊消息信号
     * @param msg 消息内容
     */
    void receivedGroupMessage(IMMessage msg);

    /**
     * @brief userOnline 用户上线信号
     * @param fromName 上线者昵称
     */
    void userOnline(QString fromName);

    /**
     * @brief userOffline 用户下线信号
     * @param fromName 下线者昵称
     */
    void userOffline(QString fromName);

    /**
     * @brief serverConnected 服务器连接成功信号
     */
    void serverConnected();

    /**
     * @brief serverClose 服务器关闭信号
     */
    void serverClose();

    /**
     * @brief connectError 连接发生错误信号
     * @param ErrorInfo 错误信息文本说明
     */
    void connectError(QString ErrorInfo);

// 槽
public slots:
    /**
     * @brief connected 连接成功时触发
     */
    void connected();

    /**
     * @brief readyread 当接收到数据时触发
     */
    void readyread();

    /**
     * @brief disconnected 当连接断开时触发
     */
    void disconnected();

    /**
     * @brief error 当连接发生错误时触发
     * @param socketError 错误信息
     */
    void error(QAbstractSocket::SocketError socketError);

// 私有成员函数
private:
    /**
     * @brief sendData 发送数据到服务器
     * @param data 数据
     */
    void sendData(QString data);

// 私有的成员变量
private:

    /**
     * @brief 当前登录的用户昵称
     */
    QString m_name;

    /**
     * @brief 客户端的Tcp Socket对象
     */
    QTcpSocket *m_socket;

    /**
     * @brief 在线人员昵称列表
     */
    QVector<QString> m_online;
    /**
     * @brief 离线人员昵称列表
     */
    QVector<QString> m_offline;

    /**
     * @brief 私聊的聊天记录
     */
    QMap<QString, QVector<IMMessage> > m_chatRecord;

    /**
     * @brief 群聊的聊天记录
     */
    QVector<IMMessage> m_gruopChatRecord;
};

#endif // IMCLIENT_H
