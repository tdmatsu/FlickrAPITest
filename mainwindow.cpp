#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

#include "flickrmodel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_FlickrModel = new FlickrModel(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

const QString FLICKR_API_KEY_URL = "http://www.flickr.com/services/api/keys/";



void MainWindow::on_btnAuth_clicked()
{
    if (!m_FlickrModel->hasApiKey()){
        QMessageBox msg;
        msg.setText("Either API Key or secret key is not set. Please get it <a href='" + FLICKR_API_KEY_URL + "'>here</a>.");
        msg.exec();
        return;
    }

    m_FlickrModel->getFrob();
}

void MainWindow::on_btnToken_clicked()
{
    if(!m_FlickrModel->hasFrob()){
        QMessageBox msg;
        msg.setText("Frob not set. Please authenticate first.");
        msg.exec();
        return;
    }

    m_FlickrModel->getToken();
}

void MainWindow::on_txtApiKey_editingFinished()
{
    qDebug() << "on_txtApiKey_editingFinished";
    m_FlickrModel->setApiKey(ui->txtApiKey->text(), ui->txtSecret->text());
}

void MainWindow::on_txtSecret_editingFinished()
{
    qDebug() << "on_txtSecret_editingFinished";
    m_FlickrModel->setApiKey(ui->txtApiKey->text(), ui->txtSecret->text());
}
