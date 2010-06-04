#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QSettings>

#include "flickrmodel.h"

const QString SETTINGS_ORGANIZATION_NAME = "Tadaaki Matsumoto";
const QString SETTINGS_DOMAIN = "tadaaki.matsumoto";
const QString SETTINGS_APP_NAME = "Flickr API Test";

const QString SETTINGS_KEY_API_KEY = "api/apiKey";
const QString SETTINGS_KEY_SECRET = "api/secret";



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationName(SETTINGS_ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(SETTINGS_DOMAIN);
    QCoreApplication::setApplicationName(SETTINGS_APP_NAME);

    loadSettings();

    m_FlickrModel = new FlickrModel(this);
}

MainWindow::~MainWindow()
{
    saveSettings();

    delete ui;
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue(SETTINGS_KEY_API_KEY, ui->txtApiKey->text());
    settings.setValue(SETTINGS_KEY_SECRET, ui->txtSecret->text());
}

void MainWindow::loadSettings()
{
    QSettings settings;
    ui->txtApiKey->setText(settings.value(SETTINGS_KEY_API_KEY).toString());
    ui->txtSecret->setText(settings.value(SETTINGS_KEY_SECRET).toString());
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
