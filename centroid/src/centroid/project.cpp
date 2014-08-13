#include "project.h"

#include <QSettings>

//////////////////////////////////////////////////////////////////////////

Project::Project()
{
}

bool Project::save(QString const& fileName)
{
    projectFileName = fileName;
    baseDirectory = QFileInfo(projectFileName).absoluteDir();

    QSettings file(fileName, QSettings::IniFormat, this);
    file.setValue("shortTag", shortTag);

    return true;
}

bool Project::load(QString const& fileName)
{
    QSettings file(fileName, QSettings::IniFormat, this);
    if (!file.contains("shortTag")) return false;

    shortTag = file.value("shortTag").toString();
    projectFileName = fileName;
    baseDirectory = QFileInfo(projectFileName).absoluteDir();

    return true;
}

QDebug operator<<(QDebug dbg, Project const& p)
{
    dbg.nospace() << "("
        << p.getShortTag() << ","
        << p.getProjectFileName() << ","
        << p.getBaseDirectory() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, Project const* p)
{
    dbg.nospace() << "("
        << p->getShortTag() << ","
        << p->getProjectFileName() << ","
        << p->getBaseDirectory() << ")";
    return dbg.space();
}
