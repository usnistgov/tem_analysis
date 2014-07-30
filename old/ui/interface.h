#ifndef __H_INTERFACES__
#define __H_INTERFACES__

#include <QString>
#include <QPlainTextEdit>


//////////////////////////////////////////////////////////////////////////

class ModuleInterface
{
public:
   ModuleInterface() : logs(NULL) {}

   void setName(const QString& str) 
   {
      name = str;
   }

   QString getName() const
   {
      return name;
   }

   void setLogsWidget(QPlainTextEdit *ptr)
   {
      logs = ptr;
   }

   void writeLog(const QString& str)
   {
      if (logs)
      {
         logs->insertPlainText(str);
         QTextCursor c = logs->textCursor();
         c.movePosition(QTextCursor::End);
         logs->setTextCursor(c); 
      }
   }

private:
   QString name;
   QPlainTextEdit *logs;
};

//////////////////////////////////////////////////////////////////////////

class FilterInterface : public ModuleInterface
{
public:
   virtual ~FilterInterface() {};
   virtual void execute(const QVector<QMap<QString, QVariant> >& parameters) = 0; 

private:
};

Q_DECLARE_INTERFACE(FilterInterface, "hpcvg.plugin.filterinterface/1.0");

//////////////////////////////////////////////////////////////////////////

#endif // __H_INTERFACES__
