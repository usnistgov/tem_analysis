#include "metadata.h"

#include <QJsonObject>

//////////////////////////////////////////////////////////////////////////

MetaData::MetaData()
{
}

bool MetaData::load(QVariantMap const& data)
{
    if (data.contains("name")) name = data.value("name").toString();
    if (data.contains("seq")) sequenceNumber = data.value("seq").toInt();
    if (data.contains("version")) version = data.value("version").toString();
    if (data.contains("shortTag")) shortTag = data.value("shortTag").toString();
    if (data.contains("type")) moduleType = data.value("type").toString();
    if (shortTag == "") shortTag = name;
}

bool MetaData::load(QJsonObject const& data)
{
   return load(data.value("MetaData").toObject().toVariantMap());
}

QDebug operator<<(QDebug dbg, MetaData const& m)
{
    dbg.nospace() << "("
        << m.getName() << ","
        << m.getShortTag() << ","
        << m.getSequenceNumber() << ","
        << m.getVersion() << ","
        << m.getModuleType() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, MetaData const* m)
{
    dbg.nospace() << "("
        << m->getName() << ","
        << m->getShortTag() << ","
        << m->getSequenceNumber() << ","
        << m->getVersion() << ","
        << m->getModuleType() << ")";
    return dbg.space();
}
