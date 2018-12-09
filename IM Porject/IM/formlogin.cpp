#include "formlogin.h"
#include "ui_formlogin.h"
#include <QMessageBox>
#include "mainwindow.h"

FormLogin::FormLogin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormLogin)
{
    ui->setupUi(this);
    connect(IMClient::instance(), &IMClient::serverConnected, this, &FormLogin::serverConnected);
    connect(IMClient::instance(), &IMClient::serverClose, this, &FormLogin::serverClose);
    connect(IMClient::instance(), &IMClient::loginResult, this, &FormLogin::loginResult);
    connect(IMClient::instance(), &IMClient::connectError, this, &FormLogin::connectError);
    // 连接服务器
    IMClient::instance()->connectServer(ui->leHostName->text().trimmed());
}

FormLogin::~FormLogin()
{
    delete ui;
}

// 连接成功后将重试按钮什么的隐藏掉
void FormLogin::serverConnected()
{
    ui->lblServerConnectState->clear();
    ui->btnRetry->setHidden(true);
    ui->leHostName->setHidden(true);
}

// 如果服务器关闭，那么程序跟着关闭
void FormLogin::serverClose()
{
    QMessageBox::warning(this, "警告", "服务器已关闭，程序即将退出！");
    QCoreApplication::quit();
}

// 如果连接发生错误，提示并将重试按钮显示
void FormLogin::connectError(QString ErrorInfo)
{
    QMessageBox::critical(this, "连接失败", "错误信息:" + ErrorInfo);
    ui->lblServerConnectState->setText("服务器连接失败！");
    ui->btnRetry->setHidden(false);
    ui->leHostName->setHidden(false);
}

// 当登录有结果时
void FormLogin::loginResult(bool isSuccess)
{
    // 如果登录没有成功，将登录按钮设置为可用，并提示用户
    if (!isSuccess)
    {
        QMessageBox::critical(this, "登录失败", "该用户名或连接已经登录！");
        ui->btnLogin->setText("登  录");
        ui->btnLogin->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        return;
    }
    // 登录成功，启动主窗体
    MainWindow *w = new MainWindow;
    w->show();
    this->close();
}

// 登录按钮按下时
void FormLogin::on_btnLogin_clicked()
{
    // 获取用户名
    QString username = ui->lineEdit->text().trimmed();
    if (username.isEmpty())
    {
        QMessageBox::critical(this, "错误", "请输入用户名！");
        return;
    }
    // 检测是否存在非法字符
    bool isSpace = false;
    for (int i = 0; i < username.length(); ++i)
    {
        if (username[i].isSpace()
                || username[i] == ':'
                || username[i] == '"'
                || username[i] == '/'
                || username[i] == '*'
                || username[i] == '|'
                || username[i] == '?'
                || username[i] == '<'
                || username[i] == '>')
        {
            isSpace = true;
            break;
        }
    }
    if (isSpace)
    {
        QMessageBox::critical(this, "错误", "用户名中有非法字符！");
        return;
    }

    if (!IMClient::instance()->isOpen())
    {
        QMessageBox::critical(this, "错误", "未连接服务器！");
        return;
    }

    // 尝试登录
    IMClient::instance()->login(username);
    ui->btnLogin->setText("登录中...");
    ui->btnLogin->setEnabled(false);
    ui->lineEdit->setEnabled(false);
}

// 当重试按钮按下时
void FormLogin::on_btnRetry_clicked()
{
    // 如果连接已经打开了，就直接隐藏按钮
    if (IMClient::instance()->isOpen())
    {
        ui->lblServerConnectState->clear();
        ui->btnRetry->setHidden(true);
        ui->leHostName->setHidden(true);
    }
    else
    {
        // 否则尝试重新连接服务器
        IMClient::instance()->connectServer(ui->leHostName->text().trimmed());
    }
}
