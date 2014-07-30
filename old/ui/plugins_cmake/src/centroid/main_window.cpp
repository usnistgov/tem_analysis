#include <iostream>

#include <QtWidgets>
#include <QPluginLoader>
#include <QDebug>

#include "main_window.h"
#include "modules_manager.h"

#include "module_button.h"

#include "main_viewer.h"


//////////////////////////////////////////////////////////////////////////

void MainWindow::addModuleToDock(Block *block)
{
   QTreeWidgetItem *category = new QTreeWidgetItem();

   treeWidget->addTopLevelItem(category);
   treeWidget->setItemWidget(category, 0, new ModuleButton(block->getBlockName(), treeWidget, category)); 
   category->setExpanded(false);

   QFrame* frame = new QFrame(treeWidget);
   frame->setStyleSheet("QFrame { border: 0px solid white; }");
   QBoxLayout* layout = new QVBoxLayout(frame);

   for (unsigned int i=0; i<block->getNumberOfParameters(); ++i)
   {
      QString pn = block->getParameterName(i);
      QString type = block->getTypeOfParameter(pn);
      QString tooltip = block->getToolTip(pn);

      if (type == "file")
      {
         QHBoxLayout *fileLayout = new QHBoxLayout();

         MyLineEdit *line_edit = new MyLineEdit(block->getValueOfParameter(pn));
         line_edit->setBlock(block);
         line_edit->setID(i);
         line_edit->setToolTip(tooltip); 

         //QPushButton *btn_file = new QPushButton("..");
         DirUpdateButton *btn_file = new DirUpdateButton("..");
         btn_file->setToolTip(tooltip); 

         btn_file->setBlock(block);
         btn_file->setID(i);
         btn_file->setLineEdit(line_edit);
         btn_file->setMaximumWidth(30);
         
         fileLayout->addWidget(line_edit);
         fileLayout->addWidget(btn_file);

         layout->addWidget(new QLabel(block->getFormNameOfParameter(pn)));
         layout->addLayout(fileLayout);
      }
      else if (type == "int")
      {
         layout->addWidget(new QLabel(block->getFormNameOfParameter(pn)));

         MyLineEdit *line_edit = new MyLineEdit(block->getValueOfParameter(pn));
         line_edit->setBlock(block);
         line_edit->setID(i);
         line_edit->setToolTip(tooltip);

         layout->addWidget(line_edit);
      }
      else if (type == "float")
      {
         layout->addWidget(new QLabel(block->getFormNameOfParameter(pn)));
         MyLineEdit *line_edit = new MyLineEdit(block->getValueOfParameter(pn));
         line_edit->setBlock(block);
         line_edit->setID(i);
         line_edit->setToolTip(tooltip);

         layout->addWidget(line_edit);
      }
      else if (type == "bool")
      {
         QHBoxLayout *hLayout = new QHBoxLayout();

         // add check box and set it's value to checked or unchecked
         MyCheckBox *cb = new MyCheckBox(block->getFormNameOfParameter(pn), this);
         cb->setBlock(block);
         cb->setID(i);
         cb->setToolTip(tooltip);
         // set 
         if (block->getValueOfParameter(pn) == "1")
         {
            cb->setCheckState(Qt::Checked);
         } else cb->setCheckState(Qt::Unchecked);
         // setup views
         hLayout->addWidget(cb);
         layout->addLayout(hLayout);
      }

   }

   QPushButton *btn_execute = new QPushButton("Execute");
   connect(btn_execute, SIGNAL(released()), block, SLOT(execute()));
   btn_execute->setMaximumWidth(120);

   QHBoxLayout *exeLayout = new QHBoxLayout();
   exeLayout->addSpacerItem(new QSpacerItem(100, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
   exeLayout->addWidget(btn_execute);
   layout->addLayout(exeLayout);

   QTreeWidgetItem* container = new QTreeWidgetItem();
   container->setDisabled(false);
   category->addChild(container);
   treeWidget->setItemWidget(container, 0, frame);      
}

//////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
{
   setupUi(this); 

   ModulesManager::instance()->init(plainLogsTextEdit);
   ModulesManager::instance()->setupAllFilterBlocks(blocks);

   // adding loaded modules to Dock
   //
   treeWidget->setRootIsDecorated(false);
   treeWidget->setIndentation(0);

   QVectorIterator<Block*> i(blocks);
   while (i.hasNext())
   {
      addModuleToDock(i.next());
   }

   // add action allowsing to close/view docking widgets
   // 
   menuView->addAction(dockModulesWidget->toggleViewAction());
   menuView->addAction(dockLogsWidget->toggleViewAction());

   // set central widget  
   //
   main_viewer_form = new MainViewerForm;
   main_viewer_form->setStatusBar(statusbar);
   setCentralWidget(main_viewer_form);

   // disable viewer's icons initially
   //
   viewer_icons_show(false);

   // default mode for interaction with the viewer
   //
   action_ScrollHandDrag->setChecked(true);

   setUnifiedTitleAndToolBarOnMac(true);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ReadData_triggered()
{
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_NextFrame_triggered()
{
   main_viewer_form->frame_next();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_PreviousFrame_triggered()
{
   main_viewer_form->frame_previous();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ZoomIn_triggered()
{
   main_viewer_form->zoom_in();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ZoomOut_triggered()
{
   main_viewer_form->zoom_out();   
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_PlayFrames_triggered()
{
   if (main_viewer_form->get_number_of_frames() > 1)
   {
      main_viewer_form->animate_start_stop();
   } 
   else
   {
      action_PlayFrames->setChecked(false);
   }
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::viewer_icons_show(bool is_enabled)
{
   action_PlayFrames->setEnabled(is_enabled);
   action_PreviousFrame->setEnabled(is_enabled);
   action_NextFrame->setEnabled(is_enabled);
   action_ZoomIn->setEnabled(is_enabled);
   action_ZoomOut->setEnabled(is_enabled);
   action_ScrollHandDrag->setEnabled(is_enabled);
   action_SelectionMode->setEnabled(is_enabled);  
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ScrollHandDrag_triggered()
{
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); 
   action_ScrollHandDrag->setChecked(true);
   action_SelectionMode->setChecked(false);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectionMode_triggered()
{
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::RubberBandDrag); 
   action_ScrollHandDrag->setChecked(false);
   action_SelectionMode->setChecked(true);   
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ParticlesViewer_triggered()
{
   QString directory = QFileDialog::getExistingDirectory(this, "Particles Viewer - Specify Directory", QDir::currentPath());
   main_viewer_form->particles_from_dir(directory);
   if (main_viewer_form->get_number_of_frames() > 0)
   {
      viewer_icons_show(true);
   }
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ImageViewer_triggered()
{
   QString directory = QFileDialog::getExistingDirectory(this, "Image Viewer - Specify Directory", QDir::currentPath());
   main_viewer_form->images_from_dir(directory);
   if (main_viewer_form->get_number_of_frames() > 0)
   {
      viewer_icons_show(true);
   }
}

//////////////////////////////////////////////////////////////////////////

