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
    file.setValue("baseDirectory", baseDirectory.absolutePath());
    file.setValue("shortTag", shortTag);
    return true;
}

bool Project::load(QString const& fileName)
{
    QSettings file(fileName, QSettings::IniFormat, this);
    if (!file.contains("baseDirectory") || !file.contains("shortTag")) return false;

    baseDirectory = QDir(file.value("baseDirectory").toString());
    shortTag = file.value("shortTag").toString();
    projectFileName = fileName;
    return true;
}

