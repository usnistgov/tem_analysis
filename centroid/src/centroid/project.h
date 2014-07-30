#ifndef __H_PROJECT__
#define __H_PROJECT__

#include <QObject>
#include <QDir>

class Project : public QObject
{  
   Q_OBJECT

public:
   Project();

   QDir getBaseDirectory() const { return baseDirectory; }
   void setBaseDirectory(const QDir& baseDir) { baseDirectory = baseDir; }

   QString getShortTag() const { return shortTag; }
   void setShortTag(const QString& tag) { shortTag = tag; }

   bool save(QString const& fileName);
   bool load(QString const& fileName);

private:
   QDir baseDirectory;
   QString shortTag;
};

#endif // __H_PROJECT__
