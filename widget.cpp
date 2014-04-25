#include "widget.h"
#include "ui_widget.h"
#include "connection.h"
#include "pieview.h"
#include <QtSql>
#include <QtGui>
#include <QSplitter>
#include <QDomText>
#include <QTableView>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setFixedSize(750,500);
    ui->stackedWidget->setCurrentIndex(0);

    QSqlQueryModel *typeModel=new QSqlQueryModel(this);
    typeModel->setQuery("selent name from type");
    ui->sellTypeComboBox->setModel(typeModel);

    //布局
//    QSplitter *splitter=new QSplitter(ui->managePage);
//    splitter->resize(700,500);
//    splitter->move(0,50);
//    splitter->addWidget(ui->toolBox);
//    splitter->addWidget(ui->dailyList);
//    splitter->setStretchFactor(0,1);
//    splitter->setStretchFactor(1,1);

    on_sellCancelBtn_clicked();// 初始化界面
    showDailyList();

    ui->typeComboBox->setModel(typeModel);
    createChartModelView();//创建销售数据的模型和视图
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_sellTypeComboBox_currentIndexChanged(QString type)
{
    if(type=="请选择类型")
    {
        on_sellCancelBtn_clicked();
    }
    else
    {
        ui->sellBrandComboBox->setEnabled(true);
        QSqlQueryModel *model=new QSqlQueryModel(this);
        model->setQuery(QString("select name from brand where type='%1'").arg(type));
        ui->sellBrandComboBox->setModel(model);
        ui->sellCancelBtn->setEnabled(true);
    }
}

void Widget::on_sellBrandComboBox_currentIndexChanged(QString brand)
{
    ui->sellNumSpinBox->setValue(0);
    ui->sellNumSpinBox->setEnabled(false);
    ui->sellSumLineEdit->clear();
    ui->sellSumLineEdit->setEnabled(false);
    ui->sellOkBtn->setEnabled(false);

    QSqlQuery query;
    query.exec(QString("select price from brand where name='%1'and type='%2'")
                .arg(brand).arg(ui->sellTypeComboBox->currentText()));
    query.next();
    ui->sellPriceLineEdit->setEnabled(true);
    ui->sellPriceLineEdit->setReadOnly(true);
    ui->sellPriceLineEdit->setText(query.value(0).toString());

    query.exec(QString("select last from brand where name='%1'and type='%2'")
                .arg(brand).arg(ui->sellTypeComboBox->currentText()));
    query.next();
    int num=query.value(0).toInt();
    if(num==0)
    {
        QMessageBox::information(this,tr("提示"),tr("该商品已售完！"),QMessageBox::Ok);
    }
    else
    {
        ui->sellNumSpinBox->setEnabled(true);
        ui->sellNumSpinBox->setMaximum(num);
        ui->sellLastNumLabel->setText(tr("剩余数量：%1").arg(num));
        ui->sellLastNumLabel->setVisible(true);
    }
}

void Widget::on_sellNumSpinBox_valueChanged(int value)
{
    if (value==0)
    {
        ui->sellSumLineEdit->clear();
        ui->sellSumLineEdit->setEnabled(false);
        ui->sellOkBtn->setEnabled(false);
    }
    else
    {
        ui->sellSumLineEdit->setEnabled(true);
        ui->sellSumLineEdit->setReadOnly(true);
        qreal sum=value*ui->sellPriceLineEdit->text().toInt();
        ui->sellSumLineEdit->setText(QString::number(sum));
        ui->sellOkBtn->setEnabled(true);
    }
}

void Widget::on_sellCancelBtn_clicked()
{
    ui->sellTypeComboBox->setCurrentIndex(0);
    ui->sellBrandComboBox->clear();
    ui->sellBrandComboBox->setEnabled(false);
    ui->sellPriceLineEdit->clear();
    ui->sellPriceLineEdit->setEnabled(false);
    ui->sellNumSpinBox->setValue(0);
    ui->sellNumSpinBox->setEnabled(false);
    ui->sellSumLineEdit->clear();
    ui->sellSumLineEdit->setEnabled(false);
    ui->sellOkBtn->setEnabled(false);
    ui->sellCancelBtn->setEnabled(false);
    ui->sellLastNumLabel->setVisible(false);
}


void Widget::on_sellOkBtn_clicked()
{
    QString type = ui->sellTypeComboBox->currentText();
    QString name = ui->sellBrandComboBox->currentText();
    int value = ui->sellNumSpinBox->value();
    // cellNumSpinBox的最大值就是以前的剩余量
    int last = ui->sellNumSpinBox->maximum() - value;

    QSqlQuery query;

    // 获取以前的销售量
    query.exec(QString("select sell from brand where name='%1' and type='%2'")
               .arg(name).arg(type));
    query.next();
    int sell = query.value(0).toInt() + value;

    // 事务操作
    QSqlDatabase::database().transaction();
    bool rtn = query.exec(
                QString("update brand set sell=%1,last=%2 where name='%3' and type='%4'")
                .arg(sell).arg(last).arg(name).arg(type));

    if (rtn) {
        QSqlDatabase::database().commit();
        QMessageBox::information(this, tr("提示"), tr("购买成功！"), QMessageBox::Ok);
        writeXml();
        showDailyList();
        on_sellCancelBtn_clicked();
    } else {
        QSqlDatabase::database().rollback();
    }
}

//获取当前时间
QString Widget::getDateTime(Widget::DateTimeType type)
{
    QDateTime datetime=QDateTime::currentDateTime();
    QString date=datetime.toString("yyyy-MM-dd");
    QString time=datetime.toString("hh:mm");
    QString dateAndTime=datetime.toString("yyyy-MM-dd dddd hh:mm");
    if(type==Date)
    {
        return date;
    }
    else if(type==Time)
    {
        return time;
    }
    else
    {
        return dateAndTime;
    }

}

//读取XML文档
bool Widget::docRead()
{
    QFile file("data.xml");
    if(!file.open(QIODevice::ReadOnly))
    {
        return false;
    }
    if(!doc.setContent(&file))
    {
        file.close();
        return false;
    }
    file.close();
    return true;
}

//写入XML文档
bool Widget::docWrite()
{
    QFile file("data.xml");
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        return false;
    }
    QTextStream out(&file);
    doc.save(out,4);
    file.close();
    return true;
}

//将销售商品信息保存到XML文件中
void Widget::writeXml()
{
    //先从文件读取
    if(docRead())
    {
        QString currentDate=getDateTime(Date);
        QDomElement root=doc.documentElement();
        //根据是否有日期节点进行处理
        if(!root.hasChildNodes())
        {
            QDomElement date=doc.createElement(QString("日期"));
            QDomAttr curDate=doc.createAttribute("date");
            curDate.setValue(currentDate);
            date.setAttributeNode(curDate);
            root.appendChild(date);
            createNodes(date);
        }
        else
        {
            QDomElement date=root.lastChild().toElement();
            if(date.attribute("date")==currentDate)//判断是否有今日的日期节点
            {
                createNodes(date);
            }
            else
            {
                //创建今日的日期节点
                QDomElement date=doc.createElement(QString("日期"));
                QDomAttr curDate=doc.createAttribute("date");
                curDate.setValue(currentDate);
                root.appendChild(date);
                createNodes(date);
            }
        }          
        docWrite();//写入XML文档
    }
}

//创建销售商品信息的节点
void Widget::createNodes(QDomElement &date)
{
    QDomElement time=doc.createElement(QString("日期"));
    QDomAttr curTime=doc.createAttribute("time");
    curTime.setValue(getDateTime(Time));
    time.setAttributeNode(curTime);
    date.appendChild(time);
    QDomElement type=doc.createElement(QString("类型"));
    QDomElement brand=doc.createElement(QString("品牌"));
    QDomElement price=doc.createElement(QString("单价"));
    QDomElement num=doc.createElement(QString("数量"));
    QDomElement sum=doc.createElement(QString("金额"));
    QDomText text;
    text=doc.createTextNode(QString("%1")
                .arg(ui->sellTypeComboBox->currentText()));
    type.appendChild(text);
    text=doc.createTextNode(QString("%1")
                .arg(ui->sellBrandComboBox->currentText()));
    brand.appendChild(text);
    text=doc.createTextNode(QString("%1")
                .arg(ui->sellPriceLineEdit->text()));
    price.appendChild(text);
    text=doc.createTextNode(QString("%1")
                .arg(ui->sellNumSpinBox->value()));
    num.appendChild(text);
    text=doc.createTextNode(QString("%1")
                .arg(ui->sellSumLineEdit->text()));
    sum.appendChild(text);
    time.appendChild(text);
    time.appendChild(type);
    time.appendChild(brand);
    time.appendChild(price);
    time.appendChild(num);
    time.appendChild(sum);
}


//显示日销售清单
// 显示日销售清单
void Widget::showDailyList()
{
    ui->dailyList->clear();
    if (docRead()) 
    {
        QDomElement root = doc.documentElement();
        QString title = root.tagName();
        QListWidgetItem *titleItem = new QListWidgetItem;
        titleItem->setText(QString("-----%1-----").arg(title));
        titleItem->setTextAlignment(Qt::AlignCenter);
        ui->dailyList->addItem(titleItem);

        if (root.hasChildNodes()) {
            QString currentDate = getDateTime(Date);
            QDomElement dateElement = root.lastChild().toElement();
            QString date = dateElement.attribute("date");
            if (date == currentDate) {
                ui->dailyList->addItem("");
                ui->dailyList->addItem(QString("日期：%1").arg(date));
                ui->dailyList->addItem("");
                QDomNodeList children = dateElement.childNodes();
                // 遍历当日销售的所有商品
                for (int i=0; i<children.count(); i++) {
                    QDomNode node = children.at(i);
                    QString time = node.toElement().attribute("time");
                    QDomNodeList list = node.childNodes();
                    QString type = list.at(0).toElement().text();
                    QString brand = list.at(1).toElement().text();
                    QString price = list.at(2).toElement().text();
                    QString num = list.at(3).toElement().text();
                    QString sum = list.at(4).toElement().text();

                    QString str = time + " 出售 " + brand + type
                            + " " + num + "台， " + "单价：" + price
                            + "元， 共" + sum + "元";
                    QListWidgetItem *tempItem = new QListWidgetItem;
                    tempItem->setText("**************************");
                    tempItem->setTextAlignment(Qt::AlignCenter);
                    ui->dailyList->addItem(tempItem);
                    ui->dailyList->addItem(str);
                }
            }
        }
    }
}


void Widget::createChartModelView()
{
    chartModel =new QStandardItemModel(this);
    chartModel->setColumnCount(2);
    chartModel->setHeaderData(0,Qt::Horizontal,QString("品牌"));
    chartModel->setHeaderData(1,Qt::Horizontal,QString("销售数量"));

    QSplitter *splitter=new QSplitter(ui->chartPage);
    splitter->resize(640,350);
    splitter->move(40,80);

    QTableView *table=new QTableView;
    PieView *pieChart=new PieView;
    splitter->addWidget(table);
    splitter->addWidget(pieChart);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,2);

    table->setModel(chartModel);
    pieChart->setModel(chartModel);

    QItemSelectionModel *selectionModel=new QItemSelectionModel(chartModel);
    table->setSelectionModel(selectionModel);
    pieChart->setSelectionModel(selectionModel);
}

 void Widget::showChart()
 {
    QSqlQuery query;
    query.exec(QString("select name,sell from brand where type='1'")
                    .arg(ui->typeComboBox->currentText()));

    chartModel->removeRows(0,chartModel->rowCount(QModelIndex()),QModelIndex());
    int row=0;
    while(query.next())
    {
        int r=qrand()%256;
        int g=qrand()%256;
        int b=qrand()%256;

        chartModel->insertRows(row,1,QModelIndex());
        chartModel->setData(chartModel->index(row,0,QModelIndex()),
                            query.value(0).toString());
        chartModel->setData(chartModel->index(row,1,QModelIndex()),
                            query.value(1).toInt());
        chartModel->setData(chartModel->index(row,0,QModelIndex()),
                            QColor(r,g,b),Qt::DecorationRole);
        row++;
    }
 }

void Widget::on_typeComboBox_currentIndexChanged(QString type)
{
    if (type!="请选择类型")
    {
        showChart();
    }
}

void Widget::on_uptBtn_clicked()
{
    if(ui->typeComboBox->currentText()!="请选择类型")
    {
        showChart();
    }
}

//商品管理按钮
void Widget::on_manageBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

//销售统计按钮
void Widget::on_chartBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}
