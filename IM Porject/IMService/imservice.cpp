#include "imservice.h"
#include "protocol.h"
#include <QDebug>

// 构造函数
IMService::IMService(QObject *parent)
    : QObject(parent),
      m_server(new QTcpServer),
      m_clientSocket(new QMap<QString, QTcpSocket *>),
      m_clientNames(new QMap<QTcpSocket *, QString>)
{
    connect(this->m_server, &QTcpServer::newConnection, this, &IMService::newConnection);
    if (this->m_server->listen(QHostAddress::Any, 9876))
        qDebug() << "Service open SUCCESS!";
    else
        qDebug() << this->m_server->errorString(); //错误信息

}

IMService::~IMService()
{
    m_server->close();
    delete m_server;
    delete m_clientSocket;
    delete m_clientNames;
    qDebug() << "Service Close!";
}

void IMService::closeService()
{
    // 遍历并关闭所有Socket连接
    for (auto it : *(this->m_clientSocket))
        it->close();
    // 清空表
    this->m_clientSocket->clear();
    this->m_clientNames->clear();
}

// 当有新连接进入时
void IMService::newConnection()
{
    // 获取这个连接的Socket
    QTcpSocket *socketTemp = this->m_server->nextPendingConnection();

    // 当接收到数据时触发readyRead
    connect(socketTemp, &QTcpSocket::readyRead, this, &IMService::readyRead);

    // 当连接断开时触发disconnected
    connect(socketTemp, &QTcpSocket::disconnected, this, &IMService::disconnected);

    qDebug() << "newConnection!";
}

// 当连接断开时触发
void IMService::disconnected()
{
    qDebug() << "disconnected!";

    // 获取断开连接的对象
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    // 释放内存
    sender->deleteLater();

    // 先查一下这个连接是否存在表中，如果没在，说明这个连接没上线，直接return即可
    if (!this->m_clientNames->contains(sender))
        return;

    // 否则说明这是一个在线用户断开连接
    // 记录这个用户的昵称
    QString senderName = this->m_clientNames->value(sender);

    // 将这个用户从表中移除
    this->m_clientSocket->remove(senderName);
    this->m_clientNames->remove(sender);

    // 通知其他人该用户离线
    this->userOffline(senderName);
}

// 当接收到数据时触发
void IMService::readyRead()
{
    // 获取接收到数据的对象
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    // 获取数据
    QByteArray data = sender->readAll();
    QTextStream in(data, QIODevice::ReadOnly);
    qDebug() << "readyread:" << data;


    // 进行协议分析与任务调度
    int functionID = 0;
    in >> functionID;

    // 如果是登录的功能码
    if (functionID == ClientFunctionCode::Login)
    {
        QString name;
        in >> name;
        // 执行登录
        this->userLogin(name, sender);
    }
    // 检测这个连接有没有登录
    else if (this->m_clientNames->contains(sender))
    {
        // 如果是私聊消息
        if (functionID == ClientFunctionCode::SendPrivateMessage)
        {
            // 获取要发送的用户昵称
            QString toName;
            in >> toName;
            in.seek(in.pos() + 1);

            // 将剩下的内容全部发送
            this->sendPrivateMessage(this->m_clientNames->value(sender), toName, in.readAll());
        }
        // 否则如果是群聊消息
        else if (functionID == ClientFunctionCode::SendGroupMessage)
        {
            in.seek(in.pos() + 1);
            this->sendGroupMessage(this->m_clientNames->value(sender), in.readAll());
        }
    }
}

void IMService::sendData(QTcpSocket *socket, QString data)
{
    if (socket != nullptr && socket->isOpen())
        socket->write(data.toUtf8());
}

/*
enum ServerFunctionCode {
    // 私聊消息
    PrivateMessage = 1,

    // 群聊消息
    GroupMessage = 2,

    // 用户上线
    UserOnline = 3,

    // 用户下线
    UserOffline = 4,

    // 登录结果
    LoginResult = 10
};
*/


// 用户登录
// 参数：name    用户昵称
void IMService::userLogin(QString name, QTcpSocket *socket)
{
    qDebug() << "userLogin():   user name:" << name << "\tsocket:" << socket;
    // 如果这个昵称或者连接已经登录了
    if (this->m_clientSocket->contains(name) || this->m_clientNames->contains(socket))
    {
        qDebug() << "Login failed!";
        // 发送登录结果：登录失败
        this->sendData(this->m_clientSocket->value(name), QString("%1 1").arg(ServerFunctionCode::LoginResult));
    }
    else
    {
        qDebug() << "Login success!";
        // 获取当前在线的用户昵称
        QList<QString> temp = this->m_clientSocket->keys();
        this->m_clientSocket->insert(name, socket);
        this->m_clientNames->insert(socket, name);
        // 将所有在线用户的昵称组合
        QString ret = QString("%1 0 %2").arg(ServerFunctionCode::LoginResult).arg(temp.length());
        QTextStream ts(&ret);
        for (QString it : temp)
            ts << ' ' << it;
        qDebug() << "ret:" << ret;
        // 发送登录结果：登录成功！ 并返回当前在线人员数据
        this->sendData(this->m_clientSocket->value(name), ret);

        // 通知其他人改用户上线
        this->userOnline(name);
    }
}

// 发送私聊消息
// 参数:fromName 发送者昵称
// 参数:toName   接收者昵称
// 参数:content  内容
void IMService::sendPrivateMessage(QString fromName, QString toName, QString content)
{
    qDebug() << "sendPrivateMessage():  fromName:" << fromName << "\ttoName" << toName << "\tcontent" << content;
    // 如果该用户存在才发送
    if (this->m_clientSocket->contains(toName))
        this->sendData(this->m_clientSocket->value(toName), QString("%1 %2 %3").arg(ServerFunctionCode::PrivateMessage).arg(fromName).arg(content));
}

// 发送群聊消息
// 参数:fromName 发送者昵称
// 参数:content  内容
void IMService::sendGroupMessage(QString fromName, QString content)
{
    qDebug() << "sendGroupMessage():  fromName:" << fromName << "\tcontent" << content;
    QString data = QString("%1 %2 %3").arg(ServerFunctionCode::GroupMessage).arg(fromName).arg(content);
    for (auto it = this->m_clientSocket->begin(); it != this->m_clientSocket->end(); it++)
        if (it.key() != fromName)
            this->sendData(it.value(), data);
}

// 用户上线
// 参数:name     用户昵称
void IMService::userOnline(QString name)
{
    qDebug() << "userOnline():  name:" << name;
    QString data = QString("%1 %2").arg(ServerFunctionCode::UserOnline).arg(name);
    for (auto it = this->m_clientSocket->begin(); it != this->m_clientSocket->end(); it++)
        if (it.key() != name)
            this->sendData(it.value(), data);
}

// 用户离线
// 参数:name     用户昵称
void IMService::userOffline(QString name)
{
    qDebug() << "userOffline():  name:" << name;
    QString data = QString("%1 %2").arg(ServerFunctionCode::UserOffline).arg(name);
    for (auto it = this->m_clientSocket->begin(); it != this->m_clientSocket->end(); it++)
        if (it.key() != name)
            this->sendData(it.value(), data);
}
