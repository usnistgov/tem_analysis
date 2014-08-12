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

   QString getShortTag() const { return shortTag; }
   void setShortTag(const QString& tag) { shortTag = tag; }

   QString getProjectFileName() const { return projectFileName; }
   QDir getBaseDirectory() const { return _baseDirectory; }

   bool save(QString const& fileName);
   bool load(QString const& fileName);

private:
   QString shortTag;
   QString projectFileName;
   QDir _baseDirectory;
};

inline QDebug operator<<(QDebug dbg, Project const& p) {
    dbg.nospace() << "(" << p.getBaseDirectory() << "," << p.getShortTag()
        << "," << p.getProjectFileName() << ")";
    return dbg.space();
}

inline QDebug operator<<(QDebug dbg, Project const* p) {
    dbg.nospace()
        << "("
        << p->getShortTag()
        << ","
        << p->getProjectFileName()
        << ","
        << p->getBaseDirectory()
        << ")";
    return dbg.space();
}

#endif // __H_PROJECT__
