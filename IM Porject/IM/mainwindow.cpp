#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDebug>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(IMClient::instance(), &IMClient::receivedPrivateMessage, this, &MainWindow::receivedPrivateMessage);
    connect(IMClient::instance(), &IMClient::receivedGroupMessage, this, &MainWindow::receivedGroupMessage);
    connect(IMClient::instance(), &IMClient::userOnline, this, &MainWindow::userOnline);
    connect(IMClient::instance(), &IMClient::userOffline, this, &MainWindow::userOffline);
    connect(IMClient::instance(), &IMClient::serverClose, this, &MainWindow::serverClose);
    // 初始化好友列表
    ui->listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    updateUserList();

    // 设置窗口标题
    this->setWindowTitle("IM:" + IMClient::instance()->getName());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 当发送按钮被点击时
void MainWindow::on_btnSend_clicked()
{
    // 获取输入框的内容
    QString text = ui->plainTextEdit->toPlainText();
    if (text.isEmpty())
        return;
    // 如果要发送的文本大于255，提示超出范围并返回
    if (text.length() > 255)
    {
        QMessageBox::warning(this, "警告", "您输入的文本太长");
        return;
    }

    // 如果当前没有选中任何人，提示并返回
    if (ui->listWidget->currentItem() == nullptr)
    {
        QMessageBox::information(this, "提示", "请选择对话对象！");
        return;
    }
    // 验证通过后清空输入框
    ui->plainTextEdit->clear();

    // 用自己的名字组装消息结构
    IMMessage msg(IMClient::instance()->getName(), text, QDateTime::currentDateTime());
    // 添加到对话框中
    this->addMessage(msg);
    // 如果当前选中的是群聊，发送群聊消息，否则发送私聊消息
    if (ui->listWidget->currentItem() == this->pGruopItem)
        IMClient::instance()->sendGroupMessage(text);
    else
        IMClient::instance()->sendPrivateMessage(ui->listWidget->currentItem()->text(), text);
}

// 接收到私聊消息时
void MainWindow::receivedPrivateMessage(IMMessage msg)
{
    // 如果当前没有选中发消息这个人，直接返回（优化的话就是新消息提醒）
    if (ui->listWidget->currentItem() == nullptr)
        return;
    if (ui->listWidget->currentItem() == this->pGruopItem)
        return;
    if (ui->listWidget->currentItem()->text() != msg.fromName)
        return;

    // 将消息添加到对话框中
    this->addMessage(msg);
}

void MainWindow::receivedGroupMessage(IMMessage msg)
{
    if (ui->listWidget->currentItem() == this->pGruopItem)
        this->addMessage(msg);
}

void MainWindow::userOnline(QString fromName)
{
    updateUserList();
    QMessageBox::information(nullptr, "提示", fromName + " 已上线");
}

void MainWindow::userOffline(QString fromName)
{
    updateUserList();
    QMessageBox::information(nullptr, "提示", fromName + " 已下线");
}

void MainWindow::serverClose()
{
    QMessageBox::warning(this, "警告", "服务器已经关闭，程序即将退出！");
    QApplication::quit();
}

// 更新用户列表
void MainWindow::updateUserList()
{
    ui->listWidget->setCurrentItem(nullptr);
    int n = ui->listWidget->count();
    // 清空表，重新加载
    for (int i = 0; i < n; i++)
        delete ui->listWidget->takeItem(0);

    // 添加群聊项在表中
    pGruopItem = new QListWidgetItem(QIcon(":icons/images/group.png"), "群聊");
    ui->listWidget->addItem(pGruopItem);

    // 添加在线人员在表中
    const QVector<QString> *pUserList = IMClient::instance()->getOnlineList();
    for (int i = 0; i < pUserList->length(); i++)
        ui->listWidget->addItem(new QListWidgetItem(QIcon(":icons/images/userOnline.png"), pUserList->at(i)));

    // 添加离线人员到表中
    pUserList = IMClient::instance()->getOfflineList();
    for (int i = 0; i < pUserList->length(); i++)
        ui->listWidget->addItem(new QListWidgetItem(QIcon(":icons/images/userOffline.png"), pUserList->at(i)));
}

void MainWindow::setChatRecord(const QVector<IMMessage> *chatRecord, const QString *name)
{
    // 清空聊天框
    ui->textBrowser->clear();
    // 如果是空的直接返回
    if (chatRecord == nullptr)
        return;
    // 如果name为空表示是群聊消息
    if (name == nullptr)
    {
        // 群聊消息直接添加即可
        for (int i = 0; i < chatRecord->length(); ++i)
            this->addMessage(chatRecord->at(i));
    }
    else
    {
        // 私聊消息需要经过转换，因为私聊消息的fromName使用i和o来代表接收或者发出
        for (int i = 0; i < chatRecord->length(); ++i)
        {
            IMMessage msg = chatRecord->at(i);
            // 如果是i表示接收的消息，用传进来的名字代替，否则是接收的消息，用自己的名字来代替
            msg.fromName = msg.fromName == "i" ? *name : IMClient::instance()->getName();
            this->addMessage(msg);
        }
    }
}

void MainWindow::addMessage(const IMMessage &msg)
{
    QString timeStr;
    // 选择时间的格式  如果是同一天，那么时间就按 12:00:00 这种格式显示
    if (msg.time.date() > QDateTime::currentDateTime().date().addDays(-1))
        timeStr = msg.time.toString("HH:mm:ss");
    // 否则如果超过一天，但是小于一年，则显示月份日期  12/10 12:00:00
    else if (msg.time.date() > QDateTime::currentDateTime().date().addYears(-1))
        timeStr = msg.time.toString("MM/dd HH:mm:ss");
    // 否则显示年份
    else
        timeStr = msg.time.toString("yyyy/MM.dd HH:mm:ss");

    // 显示文本
    ui->textBrowser->append(QString("<font color=\"%1\">%2 %3</font>").arg(msg.fromName == IMClient::instance()->getName() ? "#008040" : "#0000ff").arg(msg.fromName).arg(timeStr));
    ui->textBrowser->insertPlainText(QString("\n  %1\n").arg(msg.content));
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    // 如果选中项为空，那就清空对话框
    if (item == nullptr)
        this->setChatRecord(nullptr);

    // 如果选中了群聊选项，加载群聊的聊天记录
    if (item == this->pGruopItem)
        this->setChatRecord(IMClient::instance()->getGroupChatRecord());
    else
    {
        // 否则就是选中了用户，读取这个用户的聊天记录
        QString name = item->text();
        this->setChatRecord(IMClient::instance()->getChatRecord(name), &name);
    }
}
