#ifndef __H_PROJECT__
#define __H_PROJECT__

#include <QObject>
#include <QDir>

#include <QtGlobal>
#include <QDebug>

class Project : public QObject
{  
   Q_OBJECT

public:
   Project();

   QString getShortTag() const;
   void setShortTag(const QString& tag);

   QString getProjectFileName() const;
   QDir getBaseDirectory() const;

   bool save(QString const& fileName);
   bool load(QString const& fileName);

private:
   QString shortTag;
   QString projectFileName;
   QDir baseDirectory;
};

inline QString Project::getShortTag() const
{
    return shortTag;
}

inline void Project::setShortTag(const QString& tag)
{
    shortTag = tag;
}

inline QString Project::getProjectFileName() const
{
    return projectFileName;
}

inline QDir Project::getBaseDirectory() const
{
    return baseDirectory;
}

QDebug operator<<(QDebug dbg, Project const& p);
QDebug operator<<(QDebug dbg, Project const* p);

#endif // __H_PROJECT__
