// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "customuploader.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/history.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QBuffer>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QUrlQuery>

CustomUploader::CustomUploader(const QPixmap& capture, QWidget* parent)
  : ImgUploaderBase(capture, parent)
{
    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM,
            &QNetworkAccessManager::finished,
            this,
            &CustomUploader::handleReply);
}

void CustomUploader::handleReply(QNetworkReply* reply)
{
    spinner()->deleteLater();
    m_currentImageName.clear();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        setImageURL(json[QStringLiteral("url")].toString());

        QString url = json[QStringLiteral("url")].toString();
        auto deleteToken = json[QStringLiteral("delete_token")].toString();

        // save history
        m_currentImageName = imageURL().toString();
        int lastSlash = m_currentImageName.lastIndexOf("/");
        if (lastSlash >= 0) {
            m_currentImageName = m_currentImageName.mid(lastSlash + 1);
        }

        // save image to history
        History history;
        m_currentImageName =
          history.packFileName("custom", deleteToken, m_currentImageName);
        history.save(pixmap(), m_currentImageName);

        emit uploadOk(imageURL());
    } else {
        try {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            QJsonObject json = response.object();
            setInfoLabelText(json[QStringLiteral("error")].toString());
        } catch (...) {
            setInfoLabelText(reply->errorString());
        }
    }
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void CustomUploader::upload()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap().save(&buffer, "PNG");

    QUrl url(QStringLiteral("%1/upload").arg(ConfigHandler().uploadCustomUrl()));
    if (!ConfigHandler().uploadRandomFilename()) {
        QUrlQuery urlQuery;
        QString filename = FileNameHandler().parsedPattern() + ".png";
        urlQuery.addQueryItem(QStringLiteral("filename"), filename);
        url.setQuery(urlQuery);
    }
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    request.setRawHeader("X-Auth-Token",
                         ConfigHandler().uploadClientSecret().toUtf8());

    m_NetworkAM->post(request, byteArray);
}

void CustomUploader::deleteImage(const QString& fileName,
                                 const QString& deleteToken)
{
    Q_UNUSED(fileName)
    bool successful = QDesktopServices::openUrl(
      QUrl(QStringLiteral("%1/delete?filename=%2&token=%3")
             .arg(ConfigHandler().uploadCustomUrl(), fileName, deleteToken)));
    if (!successful) {
        notification()->showMessage(tr("Unable to open the URL."));
    }

    emit deleteOk();
}
