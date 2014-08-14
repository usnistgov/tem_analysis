#ifndef __H_FILTER_INVERT__
#define __H_FILTER_INVERT__

#include <QObject>

#include <filter_implementation.h>


class FilterSubset : public QObject, public FilterImplementation
{
   Q_OBJECT
   Q_PLUGIN_METADATA(IID "hpcvg.plugin.filterinterface/1.1" FILE "filter_subset.json")
   Q_INTERFACES(FilterInterface)

public:
   virtual void execute(const QVector<QMap<QString, QVariant> >& parameters);

   virtual ~FilterSubset() {}
};

#endif // __H_FILTER_INVERT__
