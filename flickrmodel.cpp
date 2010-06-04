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
const QString FLICKR_UPLOAD_URL = "http://api.flickr.com/services/upload/";

const QString API_API_KEY = "api_key";
const QString API_SIG_KEY = "api_sig";
const QString API_AUTH_TOKEN_KEY = "auth_token";
const QString API_PHOTO_KEY = "photo";

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
    m_strFrob = "";
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

void FlickrModel::uploadPhoto(QByteArray data)
{
/*
photo:  The file to upload.
title (optional):   The title of the photo.
description (optional): A description of the photo. May contain some limited HTML.
tags (optional): A space-seperated list of tags to apply to the photo.
is_public, is_friend, is_family (optional): Set to 0 for no, 1 for yes. Specifies who can view the photo.
safety_level (optional): Set to 1 for Safe, 2 for Moderate, or 3 for Restricted.
content_type (optional): Set to 1 for Photo, 2 for Screenshot, or 3 for Other.
hidden (optional): Set to 1 to keep the photo in global search results, 2 to hide from public searches.

What the request should look like:

POST /services/upload/ HTTP/1.1
Content-Type: multipart/form-data; boundary=---------------------------7d44e178b0434
Host: api.flickr.com
Content-Length: 35261

-----------------------------7d44e178b0434
Content-Disposition: form-data; name="api_key"

3632623532453245
-----------------------------7d44e178b0434
Content-Disposition: form-data; name="auth_token"

436436545
-----------------------------7d44e178b0434
Content-Disposition: form-data; name="api_sig"

43732850932746573245
-----------------------------7d44e178b0434
Content-Disposition: form-data; name="photo"; filename="C:\test.jpg"
Content-Type: image/jpeg

{RAW JFIF DATA}
-----------------------------7d44e178b0434--


*/
    QStringList params;
    params.append(API_API_KEY + m_apiKey);
    params.append(API_AUTH_TOKEN_KEY + m_strToken);
    QString apiSig = getMd5(params);

#if 1
    const QString BOUNDARY_STRING = "-----------------------------7d44e178b0434";
    QByteArray dataToSend; // byte array to be sent in POST

    QString strData("");
    strData += QString(BOUNDARY_STRING + "\r\nContent-Disposition: form-data; name=\"" + API_API_KEY + "\"\r\n\r\n" + m_apiKey + "\r\n");
    strData += QString(BOUNDARY_STRING + "\r\nContent-Disposition: form-data; name=\"" + API_AUTH_TOKEN_KEY + "\"\r\n\r\n" + m_strToken + "\r\n");
    strData += QString(BOUNDARY_STRING + "\r\nContent-Disposition: form-data; name=\"" + API_SIG_KEY + "\"\r\n\r\n" + apiSig + "\r\n");
    strData += QString(BOUNDARY_STRING + "\r\nContent-Disposition: form-data; name=\"" + API_PHOTO_KEY + "\"; filename=\"C:\test.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n");
    strData += data.data();
    strData += QString("\r\n" + BOUNDARY_STRING);

    dataToSend = strData.toAscii(); // convert data string to byte array for request

    // request init
    QNetworkRequest request;
    request.setUrl(QUrl(FLICKR_UPLOAD_URL));

    request.setRawHeader("Content-Type", QString("multipart/form-data; boundary=" + BOUNDARY_STRING).toAscii());      // really need double-quotes?
    request.setRawHeader("Host", "api.flickr.com");
    request.setHeader(QNetworkRequest::ContentLengthHeader, dataToSend.size());

    QNetworkReply* reply=iNwManager->post(request, dataToSend); // perform POST request

    // connections
    connect(reply,SIGNAL(uploadProgress(qint64,qint64) ),SLOT(uploadProgress(qint64,qint64)));
    connect(reply,SIGNAL(finished()), SLOT(replyFinishedUpload())); // reply finished - close file

#else


    QNetworkReply *reply = iNwManager->post(req, data);
    connect(reply, SIGNAL( uploadProgress(qint64, qint64) ), this, SLOT( progress(qint64,qint64) ) );
#endif
}

void FlickrModel::replyFinishedUpload()
{
    qDebug() << "upload finished!!";
}

void FlickrModel::uploadProgress(qint64 x, qint64 y)
{
    qDebug() << "progress... " << x << "," << y;
}

QString FlickrModel::makeRowHeader(QString strKey, QString strValue)
{
    const QString BOUNDARY_STRING = "-----------------------------7d44e178b0434";
    return QString(BOUNDARY_STRING + "\r\nContent-Disposition: form-data; name=\"" + strKey + "\"\r\n\r\n" + strValue + "\r\n");
}

#if 0
void test()
{
    const QString BOUNDARY_STRING = "-----------------------------7d44e178b0434";
    QString strData;
    QString crlf;
//    QString fileByteSize;
    QByteArray dataToSend; // byte array to be sent in POST

    // data boundary declerations
    crlf=0x0d;
    crlf+=0x0a;

    makeRowHeader(API_API_KEY, m_)

    strData = BOUNDARY_STRING;
    strData += "Content-Disposition: form-data; name=\"";
    strData += API_API_KEY;
    strData += "\"";
               ; filename=\"ContactList.csv\";"+crlf;
    strData += "Content-Type: text/csv"+crlf+crlf+inputFile.readAll();
    strData += BOUNDARY_STRING;
    dataToSend = strData.toAscii(); // convert data string to byte array for request


    // request init
    QNetworkRequest request(QUrl(FLICKR_UPLOAD_URL));
    request.setRawHeader("Content-Type", "multipart/form-data; boundary=\"" + BOUNDARY_STRING + "\"");      // really need double-quotes?
    request.setRawHeader("Host", "api.flickr.com");
    request.setHeader(QNetworkRequest::ContentLengthHeader, dataToSend.size());

    reply=manager.post(request,dataToSend); // perform POST request

    // connections
    connect(reply,SIGNAL(uploadProgress(qint64,qint64) ),SLOT(mySetValue(qint64,qint64)));
    connect(reply,SIGNAL(finished()),SLOT(replyFinished())); // reply finished - close file
}

#endif
