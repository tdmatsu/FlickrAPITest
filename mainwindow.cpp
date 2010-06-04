#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QNetworkProxy>

#include "flickrmodel.h"

const QString SETTINGS_ORGANIZATION_NAME = "Tadaaki Matsumoto";
const QString SETTINGS_DOMAIN = "tadaaki.matsumoto";
const QString SETTINGS_APP_NAME = "Flickr API Test";

// setting keys
const QString SETTINGS_KEY_API_KEY = "api/apiKey";
const QString SETTINGS_KEY_SECRET = "api/secret";
const QString SETTINGS_KEY_PROXY_ENABLED = "proxy/enabled";
const QString SETTINGS_KEY_PROXY_TYPE = "proxy/type";
const QString SETTINGS_KEY_PROXY_HOST_NAME = "proxy/hostName";
const QString SETTINGS_KEY_PROXY_PORT = "proxy/port";
const QString SETTINGS_KEY_PROXY_USER = "proxy/user";
const QString SETTINGS_KEY_PROXY_PASSWORD = "proxy/password";
const QString SETTINGS_KEY_FLICKR_FROB = "flickr/frob";
const QString SETTINGS_KEY_FLICKR_TOKEN = "flickr/token";


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationName(SETTINGS_ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(SETTINGS_DOMAIN);
    QCoreApplication::setApplicationName(SETTINGS_APP_NAME);

    // init Proxy Type combo box
    ui->cmbProxyType->addItem("Socks5", QNetworkProxy::Socks5Proxy);
    ui->cmbProxyType->addItem("HTTP", QNetworkProxy::HttpProxy);
    ui->cmbProxyType->addItem("HTTP Caching", QNetworkProxy::HttpCachingProxy);
    ui->cmbProxyType->addItem("FTP Caching", QNetworkProxy::FtpCachingProxy);
    ui->cmbProxyType->setCurrentIndex(0);


    m_FlickrModel = new FlickrModel(this);

    loadSettings();
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

    settings.setValue(SETTINGS_KEY_PROXY_ENABLED, ui->grpProxy->isChecked());
    settings.setValue(SETTINGS_KEY_PROXY_TYPE, ui->cmbProxyType->itemData(ui->cmbProxyType->currentIndex()));
    settings.setValue(SETTINGS_KEY_PROXY_HOST_NAME, ui->txtProxyHostName->text());
    settings.setValue(SETTINGS_KEY_PROXY_PORT, ui->txtProxyPort->text());
    settings.setValue(SETTINGS_KEY_PROXY_USER, ui->txtProxyUserName->text());
    settings.setValue(SETTINGS_KEY_PROXY_PASSWORD, ui->txtProxyPassword->text());

    settings.setValue(SETTINGS_KEY_FLICKR_FROB, m_FlickrModel->frob());
    settings.setValue(SETTINGS_KEY_FLICKR_TOKEN, m_FlickrModel->token());
}

void MainWindow::loadSettings()
{
    // API Keys
    QSettings settings;
    ui->txtApiKey->setText(settings.value(SETTINGS_KEY_API_KEY).toString());
    ui->txtSecret->setText(settings.value(SETTINGS_KEY_SECRET).toString());
    m_FlickrModel->setApiKeys(ui->txtApiKey->text(), ui->txtSecret->text());

    // Proxy settings
    ui->grpProxy->setChecked(settings.value(SETTINGS_KEY_PROXY_ENABLED).toBool());
    QVariant proxyType = settings.value(SETTINGS_KEY_PROXY_TYPE);
    for (int i=0; i < ui->cmbProxyType->count(); i++){
        if (proxyType == ui->cmbProxyType->itemData(i)){
            ui->cmbProxyType->setCurrentIndex(i);
            break;
        }
    }
    ui->txtProxyHostName->setText(settings.value(SETTINGS_KEY_PROXY_HOST_NAME).toString());
    ui->txtProxyPort->setText(settings.value(SETTINGS_KEY_PROXY_PORT).toString());
    ui->txtProxyUserName->setText(settings.value(SETTINGS_KEY_PROXY_USER).toString());
    ui->txtProxyPassword->setText(settings.value(SETTINGS_KEY_PROXY_PASSWORD).toString());
    setProxy(ui->grpProxy->isChecked());

    // Flickr Frob & Token
    m_FlickrModel->setFrob(settings.value(SETTINGS_KEY_FLICKR_FROB).toString());
    m_FlickrModel->setToken(settings.value(SETTINGS_KEY_FLICKR_TOKEN).toString());
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

void MainWindow::setProxy(bool enabled)
{
    if (enabled){
        QNetworkProxy proxy;
        proxy.setType((QNetworkProxy::ProxyType)ui->cmbProxyType->itemData(ui->cmbProxyType->currentIndex()).toInt());
        proxy.setHostName(ui->txtProxyHostName->text());
        proxy.setPort(ui->txtProxyPort->text().toInt());
        proxy.setUser(ui->txtProxyUserName->text());
        proxy.setPassword(ui->txtProxyPassword->text());
        QNetworkProxy::setApplicationProxy(proxy);
    }else{
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    }
}


void MainWindow::on_btnAuth_clicked()
{
    if (!m_FlickrModel->hasApiKeys()){
        QMessageBox msg(this);
        msg.setText("Either API Key or secret key is not set. Please get it <a href='" + FLICKR_API_KEY_URL + "'>here</a>.");
        msg.exec();
        return;
    }

    m_FlickrModel->getFrob();
}

void MainWindow::on_btnToken_clicked()
{
    if(!m_FlickrModel->hasFrob()){
        QMessageBox msg(this);
        msg.setText("Frob not set. Please authenticate first.");
        msg.exec();
        return;
    }

    m_FlickrModel->getToken();
}

void MainWindow::on_txtApiKey_editingFinished()
{
    qDebug() << "on_txtApiKey_editingFinished";
    m_FlickrModel->setApiKeys(ui->txtApiKey->text(), ui->txtSecret->text());
}

void MainWindow::on_txtSecret_editingFinished()
{
    qDebug() << "on_txtSecret_editingFinished";
    m_FlickrModel->setApiKeys(ui->txtApiKey->text(), ui->txtSecret->text());
}

void MainWindow::on_grpProxy_clicked(bool checked)
{
    qDebug() << "!!!MainWindow::on_grpProxy_clicked:" << checked;

    setProxy(checked);
}


void MainWindow::on_btnShowFrobToken_clicked()
{
    QMessageBox msg(this);
    msg.setText("Frob: " + m_FlickrModel->frob() + "\nToken: " + m_FlickrModel->token());
    msg.exec();
}

void MainWindow::on_btnChkToken_clicked()
{
    m_FlickrModel->checkToken();
}

#include <QFile>

void MainWindow::on_btnUploadPicture_clicked()
{
    QFile image(":/olive_jpg");
    image.open(QIODevice::ReadOnly);
    m_FlickrModel->uploadPhoto(image.readAll());
    image.close();
}
