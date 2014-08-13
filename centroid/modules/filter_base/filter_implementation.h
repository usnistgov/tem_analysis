#ifndef __H_FILTER_IMPLEMENTATION__
#define __H_FILTER_IMPLEMENTATION__

#include "filter_interface.h"

#include <QVector>
#include <QMap>
#include <QString>
#include <QVariant>

class QPlainTextEdit;

//////////////////////////////////////////////////////////////////////////

class FilterImplementation : public FilterInterface
{
public:
   FilterImplementation();
   virtual ~FilterImplementation() {}

   virtual void setProject(Project* proj);
   virtual Project* getProject();

   virtual void setMetaData(MetaData* data);
   virtual MetaData* getMetaData();

   virtual void setLogsWidget(QPlainTextEdit* logs);

   void writeLog ( const QString & message );

   void writeParameters ( const QVector<QMap<QString, QVariant> > & parameters,
                          const QString & dirName );

private:
   Project *project;
   MetaData *metaData;
   QPlainTextEdit* logs;
};

inline void FilterImplementation::setProject(Project* proj)
{
    project = proj;
}

inline Project* FilterImplementation::getProject()
{
    return project;
}

inline void FilterImplementation::setMetaData(MetaData* data)
{
    metaData = data;
}

inline MetaData* FilterImplementation::getMetaData()
{
    return metaData;
}

inline void FilterImplementation::setLogsWidget(QPlainTextEdit* ptr)
{
    logs = ptr;
}

//////////////////////////////////////////////////////////////////////////

#endif // __H_FILTER_IMPLEMENTATION__
