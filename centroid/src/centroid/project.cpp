#include "project.h"

#include <QSettings>

//////////////////////////////////////////////////////////////////////////

Project::Project()
{
}

bool Project::save(QString const& fileName)
{
    projectFileName = fileName;
    QSettings file(fileName, QSettings::IniFormat, this);
    file.setValue("shortTag", shortTag);
    _baseDirectory = QFileInfo(projectFileName).absoluteDir();
    return true;
}

bool Project::load(QString const& fileName)
{
    QSettings file(fileName, QSettings::IniFormat, this);
    if (!file.contains("shortTag")) return false;

    shortTag = file.value("shortTag").toString();
    projectFileName = fileName;
    _baseDirectory = QFileInfo(projectFileName).absoluteDir();
    return true;
}

