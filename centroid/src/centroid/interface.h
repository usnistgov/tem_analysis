#ifndef __H_INTERFACES__
#define __H_INTERFACES__

#include <QString>
#include <QPlainTextEdit>

#include "modules_manager.h"
#include "project.h"

//////////////////////////////////////////////////////////////////////////

class ModuleInterface
{
public:
   ModuleInterface() : logs(NULL), project(NULL), sequenceNumber(1000000) {}
   virtual ~ModuleInterface() {};

   void setName(const QString& str) 
   {
      name = str;
   }

   virtual void setMetaData( const QJsonObject & loaderMetaData );

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
         logs->repaint (); // make the output appear immediately
                           // do we need something like: qApp->processEvents()?
         logs->viewport()->update();
      }
   }

   void setProject(Project* proj) { project = proj; }
   const Project* const getProject() const { return project; }


protected:

   QString name;
   QString shortTag;
   int sequenceNumber;
   QString version;
   QString moduleType;

   QPlainTextEdit *logs;
   Project *project;


private:


};

//////////////////////////////////////////////////////////////////////////

class FilterInterface : public ModuleInterface
{
public:
   virtual ~FilterInterface() {};
   virtual void execute(const QVector<QMap<QString, QVariant> >& parameters) = 0; 
   virtual void setMetaData( const QJsonObject & loaderMetaData );


protected:

    void writeParameters (const QVector<QMap<QString, QVariant> > & parameters, 
                          const QString & dirName);


private:

};

Q_DECLARE_INTERFACE(FilterInterface, "hpcvg.plugin.filterinterface/1.0");

//////////////////////////////////////////////////////////////////////////

#endif // __H_INTERFACES__
