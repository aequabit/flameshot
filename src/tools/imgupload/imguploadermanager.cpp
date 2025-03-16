// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Yurii Puchkov & Contributors
//

#include "imguploadermanager.h"
#include "src/utils/confighandler.h"
#include <QPixmap>
#include <QWidget>

// TODO - remove this hard-code and create plugin manager in the future, you may
// include other storage headers here
#include "storages/custom/customuploader.h"
#include "storages/imgur/imguruploader.h"

ImgUploaderManager::ImgUploaderManager(QObject* parent)
  : QObject(parent)
  , m_imgUploaderBase(nullptr)
{
    // TODO - implement ImgUploader for other Storages and selection among them
    m_imgUploaderPlugin = IMG_UPLOADER_STORAGE_DEFAULT;
    init();
}

void ImgUploaderManager::init()
{
    // TODO - implement ImgUploader for other Storages and selection among them,
    // example:
    // if (uploaderPlugin().compare("s3") == 0) {
    //    m_qstrUrl = ImgS3Settings().value("S3", "S3_URL").toString();
    //} else {
    //    m_qstrUrl = "https://imgur.com/";
    //    m_imgUploaderPlugin = "imgur";
    //}
    if (!ConfigHandler().uploadCustomUrl().isEmpty()) {
        m_urlString = ConfigHandler().uploadCustomUrl();
        m_imgUploaderPlugin = "custom";
        return;
    } else {
        m_urlString = "https://imgur.com/";
        m_imgUploaderPlugin = "imgur";
    }
}

ImgUploaderBase* ImgUploaderManager::uploader(const QPixmap& capture,
                                              QWidget* parent)
{
    // TODO - implement ImgUploader for other Storages and selection among them,
    // example:
    // if (uploaderPlugin().compare("s3") == 0) {
    //    m_imgUploaderBase =
    //      (ImgUploaderBase*)(new ImgS3Uploader(capture, parent));
    //} else {
    //    m_imgUploaderBase =
    //      (ImgUploaderBase*)(new ImgurUploader(capture, parent));
    //}
    if (!ConfigHandler().uploadCustomUrl().isEmpty()) {
        m_imgUploaderBase = (ImgUploaderBase*)(new CustomUploader(capture, parent));
    } else {
        m_imgUploaderBase = (ImgUploaderBase*)(new ImgurUploader(capture, parent));
    }
    if (m_imgUploaderBase && !capture.isNull()) {
        m_imgUploaderBase->upload();
    }
    return m_imgUploaderBase;
}

ImgUploaderBase* ImgUploaderManager::uploader(const QString& imgUploaderPlugin)
{
    m_imgUploaderPlugin = imgUploaderPlugin;
    init();
    return uploader(QPixmap());
}

const QString& ImgUploaderManager::uploaderPlugin()
{
    return m_imgUploaderPlugin;
}

const QString& ImgUploaderManager::url()
{
    return m_urlString;
}
