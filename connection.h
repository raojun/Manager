#ifndef CONNECTION_H
#define CONNECTION_H

#include <QtSql>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QtXml>
#include <QDomProcessingInstruction>
static bool createConnection()
{
    // 创建一个数据库连接，使用“connection1”为连接名
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "connection1");

    db.setHostName("localhost");

    //如果已经设定DSN，可以写为QString::fromLocal8Bit("DSN数据源名称")
    //如果未设定DSN，则应写为：QString dsn = QString::fromLocal8Bit(
    //"DRIVER={SQL SERVER};SERVER=localhost;DATABASE=test");
    QString dsn=QString::fromLocal8Bit("SQL Server");

    db.setDatabaseName(dsn);
    db.setUserName("QtManager");
    db.setPassword("123");
    if(!db.open())
    {
        QMessageBox::critical(0,QObject::tr("Database Error"),db.lastError().text());
        return false;
    }
    QSqlQuery query(db);
    //创建分类表
    query.exec("create table type(id varchar(5) primary key,name varchar(20))");
    query.exec("insert into type values('00000','请选择类型')");
    query.exec("insert into type values('00001','电视')");
    query.exec("insert into type values('00002','空调')");

    // 创建品牌表
    query.exec("create table brand(id varchar(5) primary key, name varchar(20), "
               "type varchar(10), price int, sum int, sell int, last int)");
    query.exec("insert into brand values('01', '海信', '电视', 3699, 50, 10, 40)");
    query.exec("insert into brand values('02', '创维', '电视', 3499, 20, 5, 15)");
    query.exec("insert into brand values('03', '海尔', '电视', 4199, 80, 40, 40)");
    query.exec("insert into brand values('04', '王牌', '电视', 3999, 40, 10, 30)");
    query.exec("insert into brand values('05', '海尔', '空调', 2899, 60, 10, 50)");
    query.exec("insert into brand values('06', '格力', '空调', 2799, 70, 20, 50)");

    //创建密码表
    query.exec("create table password(pwd varchar(18) primary key)");
    query.exec("insert into password values('123456')");
    return true;
}

//创建XML文件
static bool createXml()
{
    QFile file("data.xml");//创建data.xml文件
    if(file.exists())//判断该文件是否已经被创建
    {
        return true;
    }
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        return false;
    }

    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction=doc.createProcessingInstruction(
                    "xml","version=\"1.0\"enconding=\"UTF-8\"");
    doc.appendChild(instruction);
    QDomElement root=doc.createElement(QString("日销售清单"));
    doc.appendChild(root);
    QTextStream out(&file);
    doc.save(out,4);
    file.close();
    return true;
}

#endif // CONNECTION_H
