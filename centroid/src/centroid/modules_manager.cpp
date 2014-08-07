#include <iostream>

#include "modules_manager.h"
#include <QPluginLoader>
#include <QDir>
#include <QtWidgets>

#include "interface.h"

#include "block.h"


//////////////////////////////////////////////////////////////////////////

ModulesManager::ModulesManager()
{
}

//////////////////////////////////////////////////////////////////////////

ModulesManager::~ModulesManager()
{
    if (project) delete project;
}

//////////////////////////////////////////////////////////////////////////

ModulesManager *ModulesManager::instance()
{
   if (!modules_manager_instance)
   {
      modules_manager_instance = new ModulesManager;
   }
   return modules_manager_instance;
}

//////////////////////////////////////////////////////////////////////////

void ModulesManager::init(QPlainTextEdit *logsWidget)
{
   logs = logsWidget;
}

//////////////////////////////////////////////////////////////////////////

void ModulesManager::initProject(Project* proj)
{
   project = proj;
   imaging_instances.clear();
   loaders.clear();

   QDir pluginsDir = QDir(qApp->applicationDirPath());
   pluginsDir.setSorting(QDir::Time | QDir::Reversed);

   /*
   // TODO: verify whether it's true on OSX and Windows
   //
   #if defined(Q_OS_WIN)
      if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
      {
         pluginsDir.cdUp();
      }
   #elif defined(Q_OS_MAC)
      if (pluginsDir.dirName() == "MacOS") 
      {
         pluginsDir.cdUp();
         pluginsDir.cdUp();
         pluginsDir.cdUp();
      }
   #endif
   */

   pluginsDir.cd("plugins");

   foreach (QString fileName, pluginsDir.entryList(QDir::Files | QDir::NoDotAndDotDot)) 
   {     
      QString path = pluginsDir.absoluteFilePath(fileName);

      if (!QLibrary::isLibrary(path))
      {
         continue;
      }

      QString log;
      log.append("LOAD MODULE: ");
      log.append(path);
      // log.append("\n");

      QPluginLoader *loader = new QPluginLoader(path);

      // name will go to the function interface
      //
      QString module_name = loader->metaData().value("MetaData").toObject().value("name").toString();

      // type stays here, to detect which interface we use
      //
      QString module_type = loader->metaData().value("MetaData").toObject().value("type").toString();
      
      // version stays here, to detect compatibilities 
      //
      QString module_version = loader->metaData().value("MetaData").toObject().value("version").toString();

      log.append(" (");
      log.append("module_name: ");
      log.append(module_name);
      log.append(", type: ");
      log.append(module_type);
      log.append(", version: ");
      log.append(module_version);
      log.append(")\n");

      if (module_type == "imaging")
      {
         QObject *plugin = loader->instance();
         FilterInterface *img_function = qobject_cast<FilterInterface *>(plugin);

         if (img_function)
         {
            img_function->setLogsWidget(logs);
            img_function->setProject(proj);
            imaging_instances.insert(module_name, img_function);
            loaders.insert(module_name, loader);

            // set the name of the function
            //

            // img_function->setMetaData ( loader->metaData().
                    // value("MetaData").toObject().toVariantMap()  );

            img_function->setMetaData ( loader->metaData() );

            // img_function->setName(module_name);

            if (logs)
            {
               logs->insertPlainText(log); 
            }
         }
      }
      // delete loader;
   }
}

//////////////////////////////////////////////////////////////////////////

ModulesManager *ModulesManager::modules_manager_instance = NULL;

//////////////////////////////////////////////////////////////////////////

FilterInterface* ModulesManager::getFilterInterface(const QString& name)
{
   FilterInterface *fi;
   fi = imaging_instances[name];
   return fi;
}

//////////////////////////////////////////////////////////////////////////

QPluginLoader* ModulesManager::getModuleLoader(const QString& name)
{
   QPluginLoader *ldr;
   ldr = loaders[name];
   return ldr;
}

//////////////////////////////////////////////////////////////////////////

void ModulesManager::setupFilterBlock(Block* block, const QString& name)
{
   FilterInterface *fi = getFilterInterface(name);
   block->setType(BLOCK_TYPE_IMAGING);
   block->setInstance((QObject*)fi);

   QPluginLoader *loader = loaders[name];
   QVariantList parameters = loader->metaData().value("MetaData").toObject().value("parameters").toArray().toVariantList();

   foreach (QVariant item, parameters)    
   {
      QMap<QString, QVariant> p = item.toMap();
      if (p.contains("projectRelative") && p["projectRelative"].toBool() == true) {
          p["value"] = QDir::cleanPath(project->getBaseDirectory().absolutePath() + "/" + p["value"].toString());
      }
      block->addParameter(p);
   }
}

//////////////////////////////////////////////////////////////////////////

unsigned int ModulesManager::getNOFilterInterfaces()
{
   return imaging_instances.size();
}

//////////////////////////////////////////////////////////////////////////

void ModulesManager::setupAllFilterBlocks(QVector<Block*>& blocks)
{
   QMapIterator<QString, FilterInterface*> i(imaging_instances);
   i.toFront();
   while (i.hasNext())
   {
      i.next();
      Block *bl = new Block;
      setupFilterBlock(bl, i.key());
      blocks.push_back(bl);
   }
}

//////////////////////////////////////////////////////////////////////////
