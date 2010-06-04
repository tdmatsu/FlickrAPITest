#include "flickrmodel.h"

#include <QDebug>

#include <QStringList>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QCryptographicHash>
#include <QXmlStreamReader>



const QString FLICKR_API_BASE_URL = "http://www.flickr.com/services/rest/?";
const QString FLICKR_AUTH_URL = "http://flickr.com/services/auth/?";

const QString API_API_KEY = "api_key";
const QString API_SIG_KEY = "api_sig";
const QString API_AUTH_TOKEN_KEY = "auth_token";

const QString API_METHOD_KEY = "method";
const QString API_METHOD_VALUE_FROB = "flickr.auth.getFrob";
const QString API_METHOD_VALUE_GET_TOKEN = "flickr.auth.getToken";
const QString API_METHOD_VALUE_CHECK_TOKEN = "flickr.auth.checkToken";
const QString API_FORMAT_KEY = "format";
const QString API_FORMAT_VALUE_REST = "rest";
const QString API_FROB_KEY = "frob";
const QString API_PERMS_KEY = "perms";
const QString API_PERMS_VALUE_DELETE = "delete";


FlickrModel::FlickrModel(QObject*parent)
    : QObject(parent)
{
    iNwManager = new QNetworkAccessManager(this);
}




QString FlickrModel::getMd5(QStringList params)
{
    params.sort();
    QString ret = m_secret;
    for (int i=0;i < params.size();i++){
        ret += params.at(i);
    }
    return QCryptographicHash::hash(ret.toUtf8(), QCryptographicHash::Md5).toHex();
}


void FlickrModel::getFrob()
{
    QStringList params;
    params.append(API_METHOD_KEY + API_METHOD_VALUE_FROB);
    params.append(API_API_KEY + m_apiKey);

    QString apiSig = getMd5(params);

    qDebug() << "md5 = " << apiSig;

    QUrl url(FLICKR_API_BASE_URL);
    QList< QPair <QString, QString> > query;
    query.append(QPair<QString, QString>(API_METHOD_KEY, API_METHOD_VALUE_FROB));
    query.append(QPair<QString, QString>(API_API_KEY,m_apiKey));
    query.append(QPair<QString, QString>(API_SIG_KEY,apiSig));

    url.setQueryItems(query);

    qDebug () << url.toString();

    iNwManager->disconnect();
    connect(iNwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedFrob(QNetworkReply*)));

    iNwManager->get(QNetworkRequest(url));
}

#include <QDesktopServices>


void FlickrModel::openAuthenticatinWebPage()
{
    QStringList params;
    params.append(API_API_KEY + m_apiKey);
    params.append(API_FROB_KEY + m_strFrob);
    params.append(API_PERMS_KEY + API_PERMS_VALUE_DELETE);

    QString apiSig = getMd5(params);

    QUrl url(FLICKR_AUTH_URL);
    QList< QPair <QString, QString> > query;
    query.append(QPair<QString, QString>(API_API_KEY, m_apiKey));
    query.append(QPair<QString, QString>(API_PERMS_KEY, API_PERMS_VALUE_DELETE));
    query.append(QPair<QString, QString>(API_FROB_KEY, m_strFrob));
    query.append(QPair<QString, QString>(API_SIG_KEY, apiSig));

    url.setQueryItems(query);

    qDebug () << url.toString();

    QDesktopServices::openUrl(url);
}


void FlickrModel::replyFinished(QNetworkReply* reply)
{
    QString str = (QString)reply->readAll();

    qDebug() << str;
}


void FlickrModel::replyFinishedFrob(QNetworkReply* reply)
{
/*
<?xml version="1.0" encoding="utf-8" ?>
<rsp stat="ok">
<frob>XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX</frob>
</rsp>
*/

    QStringList currentElement;
    QXmlStreamReader xml (reply);
    while (!xml.atEnd()){
        xml.readNext();
        if (xml.isStartElement() || xml.isEndElement() || xml.isCharacters()){
            if (xml.isStartElement()){
                currentElement.append(xml.name().toString());
                if(xml.name().toString() == "rsp"){
                    qDebug() << "rsp: stat = " << xml.attributes().value("stat").toString();
                }
            }
            else if (xml.isEndElement()){
                if(currentElement.isEmpty()){
                    qDebug() << "End Element without Start!!";
                } else {
                    currentElement.removeLast();
                }
            }else{
                if (currentElement.last() == "frob"){
                    m_strFrob = xml.text().toString();
                    qDebug() << "Frob = " << m_strFrob;
                }
            }
        }
    }
    if (xml.hasError()){
        // do error handling
    }

    openAuthenticatinWebPage();
}

void FlickrModel::replyAuthenticationRequired(QNetworkReply * reply, QAuthenticator * authenticator)
{

}

void FlickrModel::setApiKeys(QString apiKey, QString secret)
{
    m_apiKey = apiKey;
    m_secret = secret;
}

bool FlickrModel::hasApiKeys()
{
    return (m_apiKey.size() && m_secret.size());
}

bool FlickrModel::hasFrob()
{
    return m_strFrob.size();
}

void FlickrModel::getToken()
{
    QStringList params;
    params.append(API_METHOD_KEY + API_METHOD_VALUE_GET_TOKEN);
    params.append(API_API_KEY + m_apiKey);
    params.append(API_FROB_KEY + m_strFrob);

    QString apiSig = getMd5(params);

    qDebug() << "md5 = " << apiSig;

    QUrl url(FLICKR_API_BASE_URL);
    QList< QPair <QString, QString> > query;
    query.append(QPair<QString, QString>(API_METHOD_KEY, API_METHOD_VALUE_GET_TOKEN));
    query.append(QPair<QString, QString>(API_API_KEY, m_apiKey));
    query.append(QPair<QString, QString>(API_FROB_KEY, m_strFrob));
    query.append(QPair<QString, QString>(API_SIG_KEY, apiSig));

    url.setQueryItems(query);

    qDebug () << url.toString();

    iNwManager->disconnect();
    connect(iNwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedToken(QNetworkReply*)));

    iNwManager->get(QNetworkRequest(url));
}

void FlickrModel::checkToken()
{
    QStringList params;
    params.append(API_METHOD_KEY + API_METHOD_VALUE_CHECK_TOKEN);
    params.append(API_API_KEY + m_apiKey);
    params.append(API_AUTH_TOKEN_KEY + m_strToken);

    QString apiSig = getMd5(params);

    qDebug() << "md5 = " << apiSig;

    QUrl url(FLICKR_API_BASE_URL);
    QList< QPair <QString, QString> > query;
    query.append(QPair<QString, QString>(API_METHOD_KEY, API_METHOD_VALUE_CHECK_TOKEN));
    query.append(QPair<QString, QString>(API_API_KEY, m_apiKey));
    query.append(QPair<QString, QString>(API_AUTH_TOKEN_KEY, m_strToken));
    query.append(QPair<QString, QString>(API_SIG_KEY, apiSig));

    url.setQueryItems(query);

    qDebug () << url.toString();

    iNwManager->disconnect();
    connect(iNwManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedToken(QNetworkReply*)));

    iNwManager->get(QNetworkRequest(url));
}

void FlickrModel::replyFinishedToken(QNetworkReply*reply)
{
/*
<?xml version="1.0" encoding="utf-8" ?>
<rsp stat="ok">
<auth>
        <token>XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX</token>
        <perms>delete</perms>
        <user nsid="XXXXXXXXX" username="XXXXXXXXXX" fullname="XXXXXXXXXXXX" />
</auth>
</rsp>
*/
    m_strToken = "";
    QStringList currentElement;
    QXmlStreamReader xml (reply);
    while (!xml.atEnd()){
        xml.readNext();
        if (xml.isStartElement() || xml.isEndElement() || xml.isCharacters()){
            if (xml.isStartElement()){
                currentElement.append(xml.name().toString());
                if(xml.name().toString() == "rsp"){
                    qDebug() << "rsp: stat = " << xml.attributes().value("stat").toString();
                }else if(xml.name().toString() == "user"){
                    qDebug() << "user: nsid = " << xml.attributes().value("nsid").toString();
                    qDebug() << "user: username = " << xml.attributes().value("username").toString();
                    qDebug() << "user: fullname = " << xml.attributes().value("fullname").toString();
                }
            }
            else if (xml.isEndElement()){
                if(currentElement.isEmpty()){
                    qDebug() << "End Element without Start!!";
                } else {
                    currentElement.removeLast();
                }
            }else{
                if (currentElement.last() == "token"){
                    m_strToken = xml.text().toString();
                    qDebug() << "Token = " << m_strToken;
                }
            }
        }
    }
    if (xml.hasError()){
        // do error handling
    }
}

