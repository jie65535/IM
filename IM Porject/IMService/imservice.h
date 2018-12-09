#ifndef IMSERVICE_H
#define IMSERVICE_H

#include <QObject>
#include <QtNetwork>
#include <QMap>

/***********************************
 *
 * Class IMService
 * IM服务端类
 *
 * 用于与客户端交互的封装类
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

class IMService : public QObject
{
    Q_OBJECT

// 公开成员函数
public:
    explicit IMService(QObject *parent = nullptr);

    ~IMService();

    /**
     * @brief closeService 关闭服务 断开所有Socket连接
     */
    void closeService();
// 信号
signals:


// 槽
public slots:
    /**
     * @brief newConnection 当有新连接进入时
     */
    void newConnection();

    /**
     * @brief disconnected 当连接断开时触发
     */
    void disconnected();

    /**
     * @brief readyRead 当接收到数据时触发
     */
    void readyRead();

// 私有成员函数
private:
    // 发送数据到socket对象中
    /**
     * @brief sendData 发送数据到socket对象中
     * @param socket 指定socket对象
     * @param data 要发送的数据
     */
    void sendData(QTcpSocket *socket, QString data);

    /**
     * @brief userLogin 用户登录
     * @param name 用户昵称
     * @param socket socket对象
     */
    void userLogin(QString name, QTcpSocket *socket);

    /**
     * @brief sendPrivateMessage 发送私聊消息
     * @param fromName 发送者昵称
     * @param toName 接收者昵称
     * @param content 内容
     */
    void sendPrivateMessage(QString fromName, QString toName, QString content);

    /**
     * @brief sendGroupMessage 发送群聊消息
     * @param fromName 发送者昵称
     * @param content 内容
     */
    void sendGroupMessage(QString fromName, QString content);

    /**
     * @brief userOnline 用户上线
     * @param name 用户昵称
     */
    void userOnline(QString name);

    /**
     * @brief userOffline 用户离线
     * @param name 用户昵称
     */
    void userOffline(QString name);

// 私有成员变量
private:
    /**
     * @brief m_server 服务端的Tcp Server对象
     */
    QTcpServer *m_server;

    // 客户端的Tcp Socket连接列表
    // QMap是一个键值对容器，在这里key是用户的昵称，value是用户的Socket连接对象
    // 每当一个新用户上线，将会添加到该容器中
    // 当用户下线时则从容器中删除
    /**
     * @brief m_clientSocket 客户端的Tcp Socket连接列表
     */
    QMap<QString, QTcpSocket *> *m_clientSocket;
    /**
     * @brief m_clientNames 客户端的昵称列表
     */
    QMap<QTcpSocket *, QString> *m_clientNames;
};

#endif // IMSERVICE_H
