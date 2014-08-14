#ifndef __H_FILTER_INVERT__
#define __H_FILTER_INVERT__

#include <QObject>

#include <filter_implementation.h>


class FilterApplyRegistration : public QObject, public FilterImplementation
{
   Q_OBJECT
   Q_PLUGIN_METADATA(IID "hpcvg.plugin.filterinterface/1.1" FILE "filter_applyRegistration.json")
   Q_INTERFACES(FilterInterface)

public:
   void execute(const QVector<QMap<QString, QVariant> >& parameters);
};

#endif // __H_FILTER_INVERT__
