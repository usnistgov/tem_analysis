#ifndef __H_INTERFACE__
#define __H_INTERFACE__

#include "project.h"
#include "metadata.h"

class QPlainTextEdit;

//////////////////////////////////////////////////////////////////////////

/**
 * FilterInterface must be a pure-virtual completely-abstract interface class.
 */
class FilterInterface
{
public:
   virtual ~FilterInterface() {}
   virtual void execute(const QVector<QMap<QString, QVariant> >&) = 0;

   virtual void setProject(Project*) = 0;
   virtual Project* getProject() = 0;

   virtual void setMetaData(MetaData*) = 0;
   virtual MetaData* getMetaData() = 0;

   virtual void setLogsWidget(QPlainTextEdit* logs) = 0;
};

Q_DECLARE_INTERFACE(FilterInterface, "hpcvg.plugin.filterinterface/1.1");

//////////////////////////////////////////////////////////////////////////

#endif // __H_INTERFACE__
