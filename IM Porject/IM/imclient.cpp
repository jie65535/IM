#include <QDateTime>
#include "imclient.h"
#include "protocol.h"
#include "imdal.h"

IMClient::IMClient(QObject *parent)
    : QObject(parent),
      m_socket(new QTcpSocket)
{
    connect(m_socket, &QTcpSocket::connected, this, &IMClient::connected);
    connect(m_socket, &QTcpSocket::readyRead, this, &IMClient::readyread);
    connect(m_socket, &QTcpSocket::disconnected, this, &IMClient::disconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            [this](QAbstractSocket::SocketError socketError){this->error(socketError);});
}

IMClient *IMClient::instance()
{
    static IMClient imClient;
    return &imClient;
}

IMClient::~IMClient()
{
    qDebug() << "~IMClient";
    if (m_socket != nullptr)
    {
        m_socket->close();
        delete m_socket;
    }
}

// 连接服务器
void IMClient::connectServer(QString hostName)
{
    m_socket->connectToHost(hostName, 9876, QTcpSocket::ReadWrite);
}

// 登录
void IMClient::login(QString name)
{
    this->m_name = name;
    this->sendData(QString("%1 %2").arg(ClientFunctionCode::Login).arg(name));
}

// 发送私聊消息
void IMClient::sendPrivateMessage(QString toName, QString content)
{
    // 发送消息到服务器
    this->sendData(QString("%1 %2 %3").arg(ClientFunctionCode::SendPrivateMessage).arg(toName).arg(content));
    // 构造消息对象，用o表示是发出消息
    IMMessage msg("o", content, QDateTime::currentDateTime());
    // 将消息添加到聊天记录中
    this->m_chatRecord[toName].append(msg);
    // 将消息插入到数据库中
    IMDAL::instance()->addPrivateMessage(toName, msg);
}

// 发送群聊消息
void IMClient::sendGroupMessage(QString content)
{
    // 发送消息到服务器
    this->sendData(QString("%1 %2").arg(ClientFunctionCode::SendGroupMessage).arg(content));
    // 构造消息对象
    IMMessage msg(this->m_name, content, QDateTime::currentDateTime());
    // 将消息添加到聊天记录中
    this->m_gruopChatRecord.append(msg);
    // 将消息插入到数据库中
    IMDAL::instance()->addGroupMessage(msg);
}

const QVector<IMMessage> *IMClient::getChatRecord(QString name)
{
    if (!this->m_chatRecord.contains(name))
        this->m_chatRecord[name] = IMDAL::instance()->getPrivateMessage(name);

    return &this->m_chatRecord[name];
}

// 连接成功时触发
void IMClient::connected()
{
    qDebug() << "connected";
    emit serverConnected();
}

// 当接收到数据时触发
void IMClient::readyread()
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
    switch (functionID) {
    case ServerFunctionCode::PrivateMessage:
    {
        // 如果是私聊消息，获取发送者昵称，然后发出获取到私聊消息的信号
        QString fromName;
        in >> fromName;
        in.seek(in.pos() + 1);
        // 构造一个消息对象，发送者用'i'表示是接受到的消息
        IMMessage msg("i", in.readAll(), QDateTime::currentDateTime());
        // 将它添加到聊天记录中
        this->m_chatRecord[fromName].append(msg);
        // 并且插入数据库
        IMDAL::instance()->addPrivateMessage(fromName, msg);
        // 然后将发送者改回原来的名称
        msg.fromName = fromName;
        // 通过信号将其传递出去
        emit receivedPrivateMessage(msg);
    }break;
    case ServerFunctionCode::GroupMessage:
    {
        // 如果是群聊消息，获取发送者昵称，然后发出获取到群聊消息的信号
        QString fromName;
        in >> fromName;
        in.seek(in.pos() + 1);
        // 构造一个消息对象
        IMMessage msg(fromName, in.readAll(), QDateTime::currentDateTime());
        // 添加到聊天记录中
        this->m_gruopChatRecord.append(msg);
        // 添加到数据库中
        IMDAL::instance()->addGroupMessage(msg);
        // 发出信号
        emit receivedGroupMessage(msg);
    }break;
    case ServerFunctionCode::UserOnline:
    {
        // 如果是用户上线，获取发送者昵称，然后发出用户上线信号
        QString fromName;
        in >> fromName;
        if (m_offline.contains(fromName))
            m_offline.removeOne(fromName);
        m_online.append(fromName);
        emit userOnline(fromName);
    }break;
    case ServerFunctionCode::UserOffline:
    {
        // 如果是用户下线，获取发送者昵称，然后发出用户下线信号
        QString fromName;
        in >> fromName;
        if (m_online.contains(fromName))
            m_online.removeOne(fromName);
        m_offline.append(fromName);
        emit userOffline(fromName);
    }break;
    case ServerFunctionCode::LoginResult:
    {
        // 如果是登录有结果了，先获取登录结果
        int result = 0;
        in >> result;
        if (result == 0)
        {
            // 如果登录成功了，获取当前在线人数
            int n = 0;
            in >> n;
            QVector<QString> names;
            QString name;
            for (int i = 0; i < n; i++)
            {
                in >> name;
                names.push_back(name);
            }
            this->m_online = names;

            // 初始化数据库
            IMDAL::instance()->initDatabase(this->m_name);
            // 获取数据库中有聊天记录的用户名列表
            QVector<QString> userList = IMDAL::instance()->getUserList();
            // 将没上线的用户名添加到离线列表中
            for (int i = 0; i < userList.length(); i++)
            {
                // 如果这个名字在在线列表中不存在同时又不是自己
                if (!this->m_online.contains(userList[i]) && userList[i] != this->m_name)
                {
                    // 就添加到离线列表中
                    this->m_offline.append(userList[i]);
                }
            }
            // 初始化群聊消息
            this->m_gruopChatRecord = IMDAL::instance()->getGroupMessage();
            // 最后发送登录成功消息
            emit loginResult(true);
        }
        else
        {
            // 否则就是登录失败了，发送登录失败的消息
            emit loginResult(false);
        }
    }break;
    default:
        return;
    }
}

// 当连接断开时触发
void IMClient::disconnected()
{
    qDebug() << "disconnected";
    // 发送服务器关闭信号
    emit serverClose();
}

// 当连接发生错误时触发
void IMClient::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError;
    // 发出连接错误信号
    emit connectError(this->m_socket->errorString());
}

// 向服务器发送数据方法
void IMClient::sendData(QString data)
{
    if (this->isOpen())
        this->m_socket->write(data.toUtf8());
}
