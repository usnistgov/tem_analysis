#ifndef __H_MODULE_BUTTON__
#define __H_MODULE_BUTTON__

#include <QPushButton>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QDebug>

#include "block.h"

#include <iostream>


//////////////////////////////////////////////////////////////////////////
  
class ModuleButton : public QPushButton
{
   Q_OBJECT

public:
   ModuleButton(const QString& text, QTreeWidget* parent, QTreeWidgetItem* item);

private slots:
   void ButtonPressed();

private:
   QTreeWidgetItem* m_pItem;
};

//////////////////////////////////////////////////////////////////////////

class MyLineEdit : public QLineEdit
{
   Q_OBJECT

public:   
   MyLineEdit(const QString& contents, QWidget *parent = 0) : QLineEdit(contents, parent)
   {
      block = NULL;
      connect(this, SIGNAL(textEdited(QString)), this, SLOT(emitMyTextEdited(QString)));
   }

   void setBlock(Block* _block) { block = _block; }  
   void setID(int _index) { index = _index; }
   
public slots:   
   void emitMyTextEdited(QString text) 
   { 
      // emit myTextEdited(text, index); 
      if (block)
      {
         block->setValueOfParameter(index, text); 
      }
   }

signals: 
   void myTextEdited(QString, int);

private: 
   Block* block;
   int index;   
};

//////////////////////////////////////////////////////////////////////////

class MyCheckBox : public QCheckBox
{
   Q_OBJECT

public:
   MyCheckBox(const QString& text, QWidget *parent = 0) : QCheckBox(text, parent)
   {
      block = NULL;
      connect(this, SIGNAL(clicked(bool)), this, SLOT(emitMyCheckBox(bool)));
   }

   void setBlock(Block* _block) { block = _block; }  
   void setID(int _index) { index = _index; }

public slots:
   void emitMyCheckBox(bool checked)   
   {
      if (block)
      {
         if (checked)
         {
            block->setValueOfParameter(index, "1");
         } else 
         {
            block->setValueOfParameter(index, "0");
         }
      }
   }

private:
   Block* block;
   int index;   
};

//////////////////////////////////////////////////////////////////////////

class DirUpdateButton : public QPushButton
{
   Q_OBJECT

public:
   DirUpdateButton(const QString& text, QWidget *parent = 0) : QPushButton(text, parent)
   {
      block = NULL;
      connect(this, SIGNAL(released()), this, SLOT(emitDirUpdate()));
   }

   void setBlock(Block* _block) { block = _block; }  
   void setID(int _index) { index = _index; }
   void setLineEdit(MyLineEdit* _le) { lineEdit = _le; }

public slots:
   void emitDirUpdate()   
   {
      if (block)
      {

         // by default we use the current dir as our starting point
         QString startDir = QDir::currentPath();

         if (lineEdit)
         {
            QString currFileName = lineEdit->text();
            if (currFileName != "")
            {
                // if there's something in lineEdit, then use that dir
                QFileInfo currFileInfo (currFileName);
                startDir = currFileInfo.canonicalPath();
            }
         }


         QString directory = 
            QFileDialog::getExistingDirectory (this, 
                                               "Specify Directory", 
                                               startDir );
         if (directory != "")  // don't do anything if nothing specified
         {
            block->setValueOfParameter(index, directory);
            if (lineEdit)
            {
                lineEdit->setText(directory);
            }
         }
      }
   }

private:
   Block* block;
   int index;   
   MyLineEdit *lineEdit;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_MODULE_BUTTON__
