#include <QCoreApplication>
#include <QTextCodec>
#include "imservice.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    static IMService imService;
    return a.exec();
}
