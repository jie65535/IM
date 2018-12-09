#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QVector>
#include "imclient.h"
#include "immessage.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnSend_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

public slots:
    /**
     * @brief receivedPrivateMessage 接收到私聊消息时触发
     * @param msg 消息内容
     */
    void receivedPrivateMessage(IMMessage msg);

    /**
     * @brief receivedGroupMessage 接收到群聊消息时触发
     * @param msg 消息内容
     */
    void receivedGroupMessage(IMMessage msg);

    /**
     * @brief userOnline 用户上线时触发
     * @param fromName 上线者昵称
     */
    void userOnline(QString fromName);

    /**
     * @brief userOffline 用户下线时触发
     * @param fromName 下线者昵称
     */
    void userOffline(QString fromName);

    /**
     * @brief serverClose 服务器关闭时触发
     */
    void serverClose();

private:
    /**
     * @brief updateUserList 更新用户列表
     */
    void updateUserList();

    /**
     * @brief setChatRecord 设置当前聊天记录
     * @param chatRecord 聊天记录内容
     */
    void setChatRecord(const QVector<IMMessage> *chatRecord, const QString *name = nullptr);

    /**
     * @brief addMessage 添加一条消息
     * @param msg 消息内容
     */
    void addMessage(const IMMessage &msg);
private:
    Ui::MainWindow *ui;

    /**
     * @brief pGruopItem 群聊项指针
     */
    QListWidgetItem *pGruopItem;
};

#endif // MAINWINDOW_H
