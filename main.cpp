#include "widget.h"
#include <QApplication>
#include "connection.h"
#include "logindialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //设置中文字符编码
//    QTextCodec::setCodecForTr(QTextCodec::setCodecForLocale());
//    QTextCodec::setCodecForCString(QTextCodec::codecForLocale());

    if (!createConnection()||!createXml())
    {
        return 0;
    }

    Widget w;
     LoginDialog dlg;
     if(dlg.exec()==QDialog::Accepted)
     {
        w.show();
        return a.exec();
     }
     else
     {
        return 0;
     }
}
