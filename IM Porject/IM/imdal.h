#ifndef IMDAL_H
#define IMDAL_H

#include <QVector>
#include <QString>
#include "immessage.h"

// IM数据层
class IMDAL
{
public:
    /**
     * @brief instance 单例对象
     * @return 单例对象指针
     */
    static IMDAL *instance();

    /**
     * @brief initDatabase 初始化数据库
     * @param name 用户名
     */
    void initDatabase(QString name);

    /**
     * @brief addPrivateMessage 添加一条私聊消息
     * @param name 用户名
     * @param msg 消息内容
     */
    void addPrivateMessage(QString name, IMMessage msg);

    /**
     * @brief addGroupMessage 添加一条群聊消息
     * @param msg 消息内容
     */
    void addGroupMessage(IMMessage msg);

    /**
     * @brief getPrivateMessage 获取私聊消息
     * @param name 对方昵称
     * @return 消息内容
     */
    QVector<IMMessage> getPrivateMessage(QString name);

    /**
     * @brief getGroupMessage 获取群聊消息
     * @return 消息内容
     */
    QVector<IMMessage> getGroupMessage();

    /**
     * @brief getUserList 获取用户列表
     * @return 用户列表
     */
    QVector<QString> getUserList();
private:
    int getUserID(QString name);
};

#endif // IMDAL_H
