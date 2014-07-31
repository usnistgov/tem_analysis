#ifndef __H_BLOCK__
#define __H_BLOCK__

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QString>

class QStatusBar;

#define BLOCK_TYPE_UNDEFINED 0
#define BLOCK_TYPE_IMAGING 1


class Block : public QObject
{  
   Q_OBJECT

public:
   Block();

   void setInstance(QObject* pObj);  
   void setType(unsigned int value);

   QString getBlockName();

   void addParameter(QMap<QString, QVariant> parameter);

   // helper function for UI construction
   QString getFormNameOfParameter(const QString& name);
   QString getTypeOfParameter(const QString& name);
   QString getParameterName(int index);
   QString getValueOfParameter(const QString& name);
   QString getToolTip(const QString& name);

   void setValueOfParameter(int index, const QString& value);
   void setStatusBar(QStatusBar* sb);

   unsigned int getNumberOfParameters();

public slots:
   void execute();

private:
   // function call 
   //
   QObject* instance; 

   // type of the function
   //
   unsigned int type;

   // all parameters
   //
   QVector<QMap<QString, QVariant> > parameters;

   QStatusBar *statusBar;
};

#endif // __H_BLOCK__
