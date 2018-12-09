#ifndef IMMESSAGE_H
#define IMMESSAGE_H

#include <QString>
#include <QDateTime>

struct IMMessage
{
    IMMessage(){}

    IMMessage(QString fromName, QString content, QDateTime time)
        : fromName(fromName),
          content(content),
          time(time) {}


    /**
     * @brief fromName 发送者
     */
    QString fromName;

    /**
     * @brief content 内容
     */
    QString content;

    /**
     * @brief time 时间
     */
    QDateTime time;
};

#endif // IMMESSAGE_H
