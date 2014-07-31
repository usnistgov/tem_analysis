#include "block.h"
#include <QApplication>
#include <QStatusBar>
#include <QDebug>
#include "interface.h"


//////////////////////////////////////////////////////////////////////////

Block::Block() :
   type(BLOCK_TYPE_UNDEFINED)
{
   parameters.clear();  
}

//////////////////////////////////////////////////////////////////////////

void Block::setInstance(QObject* pObj)
{
   if (pObj)
   {
      instance = pObj;
   }
}

//////////////////////////////////////////////////////////////////////////

void Block::addParameter(QMap<QString, QVariant> parameter)
{
   parameters.push_back(parameter);
}

//////////////////////////////////////////////////////////////////////////

void Block::setType(unsigned int value)
{
   type = value;
}

//////////////////////////////////////////////////////////////////////////

QString Block::getFormNameOfParameter(const QString& name)
{
   for (int i=0; i<parameters.size(); ++i)
   {
      if (name == parameters[i]["name"].toString())
      {
         return parameters[i]["form"].toString();        
      }
   }
   return "";
}

void Block::setStatusBar(QStatusBar* sb)
{
    statusBar = sb;
}

//////////////////////////////////////////////////////////////////////////

unsigned int Block::getNumberOfParameters()
{
   return parameters.size();
}

//////////////////////////////////////////////////////////////////////////

void Block::execute()
{
   switch(type)
   {
      case BLOCK_TYPE_IMAGING:
         statusBar->showMessage("Executing " + getBlockName() + "...");
         QApplication::setOverrideCursor(Qt::WaitCursor);

         FilterInterface *fi = (FilterInterface*)instance;                 
         fi->execute(parameters);

         QApplication::restoreOverrideCursor();
         statusBar->clearMessage();
         break;
   }
}

//////////////////////////////////////////////////////////////////////////

QString Block::getBlockName()
{
   if (type == BLOCK_TYPE_IMAGING)
   {
      FilterInterface *fi = (FilterInterface*)instance;                 
      return fi->getName();
   }
   return "";
}

//////////////////////////////////////////////////////////////////////////

QString Block::getParameterName(int index)
{
   if (index > parameters.size())
   {
      return "";
   }
   return parameters[index]["name"].toString();
}

//////////////////////////////////////////////////////////////////////////

QString Block::getTypeOfParameter(const QString& name)
{
   for (int i=0; i<parameters.size(); ++i)
   {
      if (name == parameters[i]["name"].toString())
      {
         return parameters[i]["type"].toString();        
      }
   }
   return "";  
}

//////////////////////////////////////////////////////////////////////////

QString Block::getToolTip(const QString& name)
{
   for (int i=0; i<parameters.size(); ++i)
   {
      if (name == parameters[i]["name"].toString())
      {
         return parameters[i]["tooltip"].toString();        
      }
   }
   return "";  
}

//////////////////////////////////////////////////////////////////////////

QString Block::getValueOfParameter(const QString& name)
{
   for (int i=0; i<parameters.size(); ++i)
   {
      if (name == parameters[i]["name"].toString())
      {
         return parameters[i]["value"].toString();        
      }
   }
   return "";  
}

//////////////////////////////////////////////////////////////////////////

void Block::setValueOfParameter(int index, const QString& value)
{
   parameters[index]["value"] = value;
}

//////////////////////////////////////////////////////////////////////////
