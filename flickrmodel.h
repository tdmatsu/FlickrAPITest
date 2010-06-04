#ifndef FLICKRMODEL_H
#define FLICKRMODEL_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QAuthenticator;

class FlickrModel : public QObject
{
    Q_OBJECT
public:
    FlickrModel(QObject*parent=0);

    void getFrob();
    void getToken();
    void setApiKeys(QString apiKey, QString secret);
    bool hasApiKeys();
    bool hasFrob();

private:
    void openAuthenticatinWebPage();

    QNetworkAccessManager *iNwManager;
    QString m_apiKey;
    QString m_secret;
    QString m_strFrob;
    QString m_strToken;
    QString getMd5(QStringList params);

private slots:
    void replyFinished(QNetworkReply*);
    void replyFinishedFrob(QNetworkReply*);
    void replyFinishedToken(QNetworkReply*);
    void replyAuthenticationRequired(QNetworkReply * reply, QAuthenticator * authenticator);

};

#endif // FLICKRMODEL_H
