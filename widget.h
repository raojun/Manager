#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDomDocument>

class QStandardItemModel;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    enum DateTimeType{Time,Date,DateTime};
    QString getDateTime(DateTimeType type);//获取当前时间和日期

private slots:
    void on_sellTypeComboBox_currentIndexChanged(QString type);

    void on_sellBrandComboBox_currentIndexChanged(QString brand);

    void on_sellNumSpinBox_valueChanged(int value);

    void on_sellCancelBtn_clicked();

    void on_sellOkBtn_clicked();

    void on_typeComboBox_currentIndexChanged(QString type);

    void on_uptBtn_clicked();

    void on_manageBtn_clicked();

    void on_chartBtn_clicked();

private:
    Ui::Widget *ui;

    QDomDocument doc;

    bool docRead();
    bool docWrite();
    void writeXml();//将销售商品信息保存到XML文件中
    void createNodes(QDomElement &date);
    void showDailyList();

    QStandardItemModel *chartModel;
    void createChartModelView();
    void showChart();
};

#endif // WIDGET_H
