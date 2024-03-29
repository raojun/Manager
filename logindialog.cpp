#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QSqlQuery>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setFixedSize(400,300);
    setWindowTitle(tr("登录"));
    ui->pwdLineEdit->setFocus();
    ui->loginBtn->setDefault(true);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

//登录
void LoginDialog::on_loginBtn_clicked()
{
	if(ui->pwdLineEdit->text().isEmpty())
	{
		QMessageBox::information(this,tr("请输入密码"),
									tr("请先输入密码在登录！"),QMessageBox::Ok);
		ui->pwdLineEdit->setFocus();
	}
	else
	{
		QSqlQuery query;
		query.exec("select pwd from password");
		query.next();
        if(query.value(0).toString()==ui->pwdLineEdit->text())
		{
			QDialog::accept();//返回设置值为Accepted
		}
		else
		{
			QMessageBox::warning(this,tr("密码错误"),
						tr("请输入正确的密码！"),QMessageBox::Ok);
			ui->pwdLineEdit->clear();
            ui->pwdLineEdit->setFocus();
		}
	}
}

//退出
void LoginDialog::on_quitBtn_clicked()
{
	QDialog::reject();//返回设置值为Rejected
}
