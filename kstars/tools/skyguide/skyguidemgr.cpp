/***************************************************************************
                          skyguidemgr.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 2015/05/06
    copyright            : (C) 2015 by Marcos Cardinot
    email                : mcardinot@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>
#include <QStandardPaths>
#include <kzip.h>

#include "kstars.h"
#include "skyguidemgr.h"

SkyGuideMgr::SkyGuideMgr()
        : m_view(new SkyGuideView())
{
    m_guidesDir = QDir(QStandardPaths::locate(QStandardPaths::DataLocation,
            "tools/skyguide/resources/guides", QStandardPaths::LocateDirectory));

    loadAllSkyGuideObjects();

    m_skyGuideWriter = new SkyGuideWriter(this, KStars::Instance());

    connect((QObject*)m_view->rootObject(), SIGNAL(addSkyGuide()), this, SLOT(slotAddSkyGuide()));
    connect((QObject*)m_view->rootObject(), SIGNAL(openWriter()), m_skyGuideWriter, SLOT(show()));
}

SkyGuideMgr::~SkyGuideMgr() {
}

void SkyGuideMgr::loadAllSkyGuideObjects() {
    QDir root = m_guidesDir;
    QDir::Filters filters = QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks;
    root.setFilter(filters | QDir::Dirs);
    QFileInfoList guidesRoot = root.entryInfoList();
    foreach (QFileInfo r, guidesRoot) {
        QDir guideDir(r.filePath());
        guideDir.setFilter(filters | QDir::Files);
        QFileInfoList guideFiles = guideDir.entryInfoList();
        foreach (QFileInfo g, guideFiles) {
            if (g.fileName() == JSON_NAME) {
                SkyGuideObject* s = buildSGOFromJson(g.absoluteFilePath());
                loadSkyGuideObject(s);
            }
        }
    }
    m_view->setModel(m_skyGuideObjects);
}

bool SkyGuideMgr::loadSkyGuideObject(SkyGuideObject* skyGuideObj) {
    if (!skyGuideObj || !skyGuideObj->isValid()) {
        return false;
    }

    // title is unique?
    foreach (QObject* sg, m_skyGuideObjects) {
        if (((SkyGuideObject*)sg)->title() == skyGuideObj->title()) {
            qWarning()  << "SkyGuideMgr: The title '"
                        << skyGuideObj->title()
                        << "' is being used already.";
            return false;
        }
    }

    m_skyGuideObjects.append(skyGuideObj);
    return true;
}

SkyGuideObject* SkyGuideMgr::buildSGOFromJson(const QString& jsonPath) {
    QFileInfo info(jsonPath);
    if (info.fileName() != JSON_NAME || !info.exists()) {
        qWarning() << "SkyGuideMgr: The JSON file is invalid or does not exist!"
                   << jsonPath;
        return NULL;
    }

    QFile jsonFile(jsonPath);
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qWarning() << "SkyGuideMgr: Couldn't open the JSON file!" << jsonPath;
        return NULL;
    }

    QJsonObject json(QJsonDocument::fromJson(jsonFile.readAll()).object());
    jsonFile.close();

    SkyGuideObject* s = new SkyGuideObject(info.absolutePath(), json.toVariantMap());
    if (!s->isValid()) {
        qWarning()  << "SkyGuideMgr: SkyGuide is invalid!" << jsonPath;
        return NULL;
    }
    return s;
}

SkyGuideObject* SkyGuideMgr::buildSGOFromZip(const QString& zipPath) {
    // try to open the SkyGuide archive
    KZip archive(zipPath);
    if (!archive.open(QIODevice::ReadOnly)) {
        qWarning() << "SkyGuideMgr: Unable to read the file!"
                   << "Is it a zip archive?" << zipPath;
        return NULL;
    }

    // check if this SkyGuide has a 'guide.json' file in the root
    const KArchiveDirectory *root = archive.directory();
    const KArchiveEntry *e = root->entry(JSON_NAME);
    if (!e) {
        qWarning() << "SkyGuideMgr: '" + JSON_NAME + "' not found!"
                   << "A SkyGuide must have a 'guide.json' in the root!";
        return NULL;
    }

    // create a clean /temp/skyguide dir
    QDir tmpDir = QDir::temp();
    if (tmpDir.cd("skyguide")) {    // already exists?
        tmpDir.removeRecursively(); // remove everything
    }
    tmpDir.mkdir("skyguide");
    tmpDir.cd("skyguide");

    //  copy files from archive to the temporary dir
    root->copyTo(tmpDir.absolutePath(), true);
    archive.close();

    return buildSGOFromJson(tmpDir.absoluteFilePath(JSON_NAME));
}

void SkyGuideMgr::slotAddSkyGuide() {
    // check if the installation dir is writable
    if (!QFileInfo(m_guidesDir.absolutePath()).isWritable()){
        qWarning() << "SkyGuideMgr: The installation directory must be writable!"
                   << m_guidesDir.absolutePath();
        return;
    }

    // open QFileDialog - select the SkyGuide
    QString desktop = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    QString path = QFileDialog::getOpenFileName(NULL, "Add SkyGuide", desktop, "Zip File (*.zip)");

    // try to load it!
    SkyGuideObject* obj = buildSGOFromZip(path);
    if (!loadSkyGuideObject(obj)) {
        return;
    }

    // rename and move the temp folder to the installation dir
    int i = 0;
    QString newPath = m_guidesDir.absolutePath() + "/" + obj->title();
    QDir dir;
    while (!dir.rename(obj->path(), newPath)) {
        newPath += QString::number(i);  // doesn't matter the number
        i++;
    }

    // fix path
    obj->setPath(newPath);

    // refresh view
    m_view->setModel(m_skyGuideObjects);
}
