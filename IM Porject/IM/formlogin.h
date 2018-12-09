#ifndef FORMLOGIN_H
#define FORMLOGIN_H

#include <QWidget>

namespace Ui {
class FormLogin;
}

class FormLogin : public QWidget
{
    Q_OBJECT

public:
    explicit FormLogin(QWidget *parent = nullptr);
    ~FormLogin();
// 槽
private slots:
    // 服务器连接成功时触发
    void serverConnected();
    // 服务器关闭时触发
    void serverClose();
    // 连接发生错误时触发
    void connectError(QString ErrorInfo);

    // 登录结果
    // 参数：isSuccess      是否登录成功
    void loginResult(bool isSuccess);


    void on_btnLogin_clicked();

    void on_btnRetry_clicked();

private:
    Ui::FormLogin *ui;
};

#endif // FORMLOGIN_H
