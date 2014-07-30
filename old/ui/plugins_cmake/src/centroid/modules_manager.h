#ifndef __H_MODULES_MANAGER__
#define __H_MODULES_MANAGER__

#include <QObject>
#include <QPluginLoader>
#include <QMap>

class QPlainTextEdit;

//////////////////////////////////////////////////////////////////////////

class FilterInterface;
class Block;

class ModulesManager : public QObject
{
   Q_OBJECT

protected:
   ModulesManager();
   ~ModulesManager();

public:
   static ModulesManager *instance();

   void init(QPlainTextEdit *logs = NULL);

   FilterInterface *getFilterInterface(const QString& name);
   
   void setupFilterBlock(Block* block, const QString& name);
   void setupAllFilterBlocks(QVector<Block*>& blocks);

   QPluginLoader *getModuleLoader(const QString& name);

   unsigned int getNOFilterInterfaces();

private:
   static ModulesManager *modules_manager_instance;

   QMap<QString, FilterInterface*> imaging_instances;
   QMap<QString, QPluginLoader*> loaders;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_MODULES_MANAGER__
