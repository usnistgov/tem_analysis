#ifndef __H_FILTER_INVERT__
#define __H_FILTER_INVERT__

#include <QObject>

#include <filter_implementation.h>

// Definition of the class for the atom correlation plugin.

class FilterAtomCorrelation : public QObject, public FilterImplementation
{
   Q_OBJECT
   Q_PLUGIN_METADATA(IID "hpcvg.plugin.filterinterface/1.1" FILE "filter_atomCorrelation.json")
   Q_INTERFACES(FilterInterface)

public:

   // This is the method that we are actually providing.
   void execute(const QVector<QMap<QString, QVariant> >& parameters);

};

#endif // __H_FILTER_INVERT__
