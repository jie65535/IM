#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>
#include <QString>
#include <QDebug>
#include "imdal.h"
#include "immessage.h"



IMDAL *IMDAL::instance()
{
    static IMDAL imDAL;
    return &imDAL;
}

void IMDAL::initDatabase(QString name)
{
    //打印Qt支持的数据库驱动
    qDebug()<<QSqlDatabase::drivers();

    QSqlDatabase database;
    // 检测默认连接是否已经存在
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
    {
        // 存在就直接用
        database = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    }
    else
    {
        // 不存在就添加一个数据库驱动引擎SQLite
        database = QSqlDatabase::addDatabase("QSQLITE");
        // 然后打开指定用户的数据库文件
        database.setDatabaseName(QString("./msgsave_%1.db").arg(name));
        // database.setUserName("root");
        // database.setPassword("123456");
    }

    //打开数据库
    if(!database.open())
    {
        // 如果打开失败 退出程序
        qDebug()<<database.lastError();
        qFatal("failed to connect.");
    }
    else
    {
        qDebug() << "Open database success!";
        QStringList tables = database.tables();  //获取数据库中的表
        qDebug() << QString("表的个数： %1").arg(tables.count()); //打印表的个数
        // 如果表的数量小于3，说明这是一个新用户登录
        if (tables.count() < 3)
        {
/*
  SQL语句：
CREATE TABLE user (
    id   INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR NOT NULL
                 UNIQUE
);

INSERT INTO user (
    name
)
VALUES (
    'xxx'
);

CREATE TABLE message (
    id       INTEGER  PRIMARY KEY AUTOINCREMENT,
    fromID INTEGER  NOT NULL
                      REFERENCES user ( id )  ON DELETE CASCADE
                                              ON UPDATE CASCADE,
    content  TEXT     NOT NULL,
    time     DATETIME NOT NULL
                      DEFAULT ( datetime( 'now', 'localtime' )  ),
    io       CHAR     NOT NULL
);
io这个字段代表是发送还是接受：i收到消息 o发出消息 g群聊消息
*/
            database.transaction();
            QSqlQuery query;
            // 创建表并将用户昵称插入表中作为第一条数据存在，id为1
            query.exec("CREATE TABLE user ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "name VARCHAR NOT NULL UNIQUE);");
            query.exec(QString("INSERT INTO user(name) values('%1');").arg(name));
            query.exec("CREATE TABLE message ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       "fromID INTEGER NOT NULL REFERENCES user(id) ON DELETE CASCADE ON UPDATE CASCADE,"
                       "content TEXT NOT NULL,"
                       "time DATETIME NOT NULL DEFAULT(datetime('now','localtime')),"
                       "io CHAR NOT NULL);");
            if (!database.commit())
                qDebug()<<database.lastError();
        }
    }
}

void IMDAL::addPrivateMessage(QString name, IMMessage msg)
{
    if (!QSqlDatabase::database().isOpen())
        return;
    // 获得用户ID
    int userID = getUserID(name);
    if (userID == 0)
        return;
    // 插入这条私聊消息
    QSqlQuery query;
    query.prepare("INSERT INTO message(fromID, content, time, io) "
                  "VALUES(?, ?, ?, ?)");
    query.addBindValue(userID);
    query.addBindValue(msg.content);
    query.addBindValue(msg.time);
    query.addBindValue(msg.fromName);
    if (!query.exec())
        qDebug()<<query.lastError();
}

void IMDAL::addGroupMessage(IMMessage msg)
{
    if (!QSqlDatabase::database().isOpen())
        return;

    // 获取用户ID
    int userID = getUserID(msg.fromName);
    if (userID == 0)
        return;
    // 插入这条群聊消息
    QSqlQuery query;
    query.prepare("INSERT INTO message(fromID, content, time, io) "
                  "VALUES(?, ?, ?, 'g')");
    query.addBindValue(userID);
    query.addBindValue(msg.content);
    query.addBindValue(msg.time);
    if (!query.exec())
        qDebug()<<query.lastError();
}

QVector<IMMessage> IMDAL::getPrivateMessage(QString name)
{
    QVector<IMMessage> msgList;
    int userID = 0;
    if (!QSqlDatabase::database().isOpen())
        return msgList;
    QSqlQuery query;
    query.prepare("SELECT id FROM user WHERE name = ?");
    query.addBindValue(name);
    if(!query.exec())
    {
        qDebug()<<query.lastError();
        return msgList;
    }
    // 获取这个昵称的id
    while(query.next())
        userID = query.value(0).toInt();
    // 如果没有这个人的记录，直接返回空
    if (userID == 0)
        return msgList;

    // 否则查找这个用户的消息
    query.prepare("SELECT io, content, time FROM message WHERE fromID = ? AND io != 'g'");
    query.addBindValue(userID);
    if(!query.exec())
    {
        qDebug()<<query.lastError();
        return msgList;
    }
    // 将所有消息添加到列表中
    while(query.next())
        msgList.append(IMMessage(query.value(0).toString(), query.value(1).toString(), query.value(2).toDateTime()));

    return msgList;
}

QVector<IMMessage> IMDAL::getGroupMessage()
{
    QVector<IMMessage> msgList;
    if (!QSqlDatabase::database().isOpen())
        return msgList;
    QSqlQuery query;
    if(!query.exec("SELECT name, content, time FROM message,user WHERE io == 'g' AND user.id = fromID"))
    {
        qDebug()<<query.lastError();
        return msgList;
    }
    // 将所有消息添加到列表中
    while(query.next())
        msgList.append(IMMessage(query.value(0).toString(), query.value(1).toString(), query.value(2).toDateTime()));

    return msgList;
}

QVector<QString> IMDAL::getUserList()
{
    qDebug() << "getUserList()";
    QVector<QString> names;
    if (!QSqlDatabase::database().isOpen())
        return names;
    QSqlQuery query;
    if (!query.exec("SELECT name FROM user"))
    {
        qDebug()<<query.lastError();
    }
    else
    {
        while (query.next())
            names.append(query.value(0).toString());
    }
    qDebug() << names;
    return names;
}

int IMDAL::getUserID(QString name)
{
    QSqlQuery query;
    int userID = 0;
    query.prepare("SELECT id FROM user WHERE name = ?");
    query.addBindValue(name);
    if(!query.exec())
    {
        qDebug()<<query.lastError();
        return 0;
    }
    // 获取这个昵称的id
    while(query.next())
        userID = query.value(0).toInt();
    if (userID)
        return userID;

    // 如果没有这个人的记录，那么就创建这个人的ID
    query.prepare("INSERT INTO user(name) VALUES(?)");
    query.addBindValue(name);
    if(!query.exec())
    {
        qDebug()<<query.lastError();
        return 0;
    }
    return query.lastInsertId().toInt();
}
