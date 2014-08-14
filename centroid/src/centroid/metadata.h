#ifndef __H_METADATA__
#define __H_METADATA__

#include <QObject>
#include <QDir>

#include <QtGlobal>
#include <QDebug>

class MetaData : public QObject
{  
   Q_OBJECT

public:
   MetaData();

   bool load(QVariantMap const& data);
   bool load(QJsonObject const& data);

   QString getName() const;
   QString getShortTag() const;
   int getSequenceNumber() const;
   QString getVersion() const;
   QString getModuleType() const;

private:
   QString name;
   QString shortTag;
   int sequenceNumber;
   QString version;
   QString moduleType;

};

inline QString MetaData::getName() const
{
    return name;
}

inline QString MetaData::getShortTag() const
{
    return shortTag;
}

inline int MetaData::getSequenceNumber() const
{
    return sequenceNumber;
}

inline QString MetaData::getVersion() const
{
    return version;
}

inline QString MetaData::getModuleType() const
{
    return moduleType;
}

QDebug operator<<(QDebug dbg, MetaData const& m);
QDebug operator<<(QDebug dbg, MetaData const* m);

#endif // __H_METADATA__

