#include <iostream>

#include <QtWidgets>
#include <QPluginLoader>
#include <QDebug>

#include "main_window.h"
#include "modules_manager.h"

#include "module_button.h"

#include "main_viewer.h"
#include "new_project.h"


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

   file_dialog = new QFileDialog(this);

   action_Quit->setShortcut(QKeySequence::Quit);
   action_NewProject->setShortcut(QKeySequence::New);
   action_OpenProject->setShortcut(QKeySequence::Open);

   read_settings();

   ModulesManager::instance()->init(plainLogsTextEdit);
   connect(this, SIGNAL(projectLoaded(Project*)),
           ModulesManager::instance(), SLOT(initProject(Project*)));

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
   viewer_particle_icons_show(false);

   // default mode for interaction with the viewer
   //
   action_ScrollHandDrag->setChecked(true);

   setUnifiedTitleAndToolBarOnMac(true);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_Quit_triggered()
{
    close();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_NewProject_triggered()
{
    Project* project = NULL;
    QSettings settingsFile;

    NewProjectDialog newProjectDialog(this);
    newProjectDialog.setDirectory(file_dialog->directory());
    newProjectDialog.setModal(true);

    int result = newProjectDialog.exec();
    switch(result)
    {
    case QDialog::Accepted:
        file_dialog->setDirectory(newProjectDialog.getDirectory());
        project = newProjectDialog.getProject();

        file_dialog->setWindowTitle("Select new project file");
        file_dialog->setFileMode(QFileDialog::AnyFile);
        if (!file_dialog->exec()) break;

        if (!project->save(file_dialog->selectedFiles()[0])) break;
        loadProject(project);
        break;

    case QDialog::Rejected:
        break;
    }
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_OpenProject_triggered()
{
   file_dialog->setWindowTitle("Select project file");
   file_dialog->setFileMode(QFileDialog::ExistingFile);

   if (!file_dialog->exec()) return;
   QString fileName = file_dialog->selectedFiles()[0];

   Project* project = new Project;
   if (!project->load(fileName))
   {
       QMessageBox::critical(this, "Error", "Could not load " + fileName);
       return;
   }

   loadProject(project);
}

void MainWindow::loadProject(Project* project)
{
   emit projectLoaded(project);
   file_dialog->setDirectory(project->getBaseDirectory());
   setWindowTitle(project->getShortTag() + " | TEM Analysis");

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
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ReadData_triggered()
{
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

void MainWindow::closeEvent(QCloseEvent* event) {
    write_settings();
    event->accept();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::read_settings() {
    QSettings settings;
    restoreGeometry(settings.value("window/geometry").toByteArray());
    restoreState(settings.value("window/state").toByteArray());
    file_dialog->restoreState(settings.value("app/opendialogstate").toByteArray());
    file_dialog->setDirectory(settings.value("app/opendialogdirectory").toString());
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::write_settings() {
    QSettings settings;
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
    settings.setValue("app/opendialogstate", file_dialog->saveState());
    settings.setValue("app/opendialogdirectory", file_dialog->directory().path());
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::viewer_icons_show(bool is_enabled)
{
   action_ZoomIn->setEnabled(is_enabled);
   action_ZoomOut->setEnabled(is_enabled);
   action_ScrollHandDrag->setEnabled(is_enabled);
   action_Photo->setEnabled(is_enabled);
}

void MainWindow::viewer_particle_icons_show(bool is_enabled)
{
   action_SelectionMode->setEnabled(is_enabled);  
   action_SelectionModeMinus->setEnabled(is_enabled);  
   action_SelectParticles->setEnabled(is_enabled);
   action_LineSelection->setEnabled(is_enabled);
   action_ShowLines->setEnabled(is_enabled);
   action_RemoveParticles->setEnabled(is_enabled);
   action_SelectionModePlusGlobal->setEnabled(is_enabled);
   action_SelectionModeMinusGlobal->setEnabled(is_enabled);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ScrollHandDrag_triggered()
{
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); 
   action_ScrollHandDrag->setChecked(true);
   action_SelectionMode->setChecked(false);
   action_SelectionModeMinus->setChecked(false);
   // action_SelectParticles->setChecked(false);
   action_SelectionModePlusGlobal->setChecked(false);   
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectionMode_triggered()
{
   main_viewer_form->set_is_selection_global(false);
   main_viewer_form->set_is_positive_selection_on(true);
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::RubberBandDrag); 
   action_ScrollHandDrag->setChecked(false);
   action_SelectionMode->setChecked(true);   
   action_SelectionModeMinus->setChecked(false);
   action_SelectionModePlusGlobal->setChecked(false);
   action_SelectionModeMinusGlobal->setChecked(false);  
   // action_SelectParticles->setChecked(false);
   main_viewer_form->draw_current_frame();
}

//////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_SelectionModeMinus_triggered()
{
   main_viewer_form->set_is_selection_global(false);
   main_viewer_form->set_is_positive_selection_on(false);
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::RubberBandDrag); 
   action_ScrollHandDrag->setChecked(false);
   action_SelectionMode->setChecked(false);   
   action_SelectionModeMinus->setChecked(true);
   action_SelectionModePlusGlobal->setChecked(false);  
   action_SelectionModeMinusGlobal->setChecked(false);  
   // action_SelectParticles->setChecked(false);
   main_viewer_form->draw_current_frame();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectParticles_triggered()
{
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::NoDrag);   
   action_ScrollHandDrag->setChecked(false);
   action_SelectionMode->setChecked(false);   
   action_SelectionModeMinus->setChecked(false);
   action_SelectionModeMinusGlobal->setChecked(false);   
   // action_SelectParticles->setChecked(true);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ParticlesViewer_triggered()
{
   file_dialog->setWindowTitle("Select particles directory");
   file_dialog->setFileMode(QFileDialog::Directory);

   if (!file_dialog->exec()) return;
   QStringList directories = file_dialog->selectedFiles();
 
   main_viewer_form->particles_from_dir(directories[0]);
   if (main_viewer_form->get_number_of_frames() > 0)
   {
      viewer_icons_show(true);
      viewer_particle_icons_show(true);
   }
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ImageViewer_triggered()
{
   file_dialog->setWindowTitle("Select images directory");
   file_dialog->setFileMode(QFileDialog::Directory);

   if (!file_dialog->exec()) return;
   QStringList directories = file_dialog->selectedFiles();

   main_viewer_form->images_from_dir(directories[0]);
   if (main_viewer_form->get_number_of_frames() > 0)
   {
      viewer_icons_show(true);
   }
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_Photo_triggered()
{
   file_dialog->setWindowTitle("Select screenshot filename");
   file_dialog->setFileMode(QFileDialog::AnyFile);

   if (!file_dialog->exec()) return;
   QString fileName = file_dialog->selectedFiles()[0]; //"snapshot_centroid.png";

   QPixmap pm = main_viewer_form->graphicsView->grab(main_viewer_form->graphicsView->rect()); 
   pm.save(fileName);  
}

//////////////////////////////////////////////////////////////////////////
//
// Select particles along the line
//

void MainWindow::on_action_LineSelection_triggered()
{
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ShowLines_triggered()
{
   bool tri = main_viewer_form->get_is_triangulation_on();
   tri = !tri;
   main_viewer_form->set_is_triangulation_on(tri);
   main_viewer_form->draw_current_frame();
}

//////////////////////////////////////////////////////////////////////////
//
// Remove all selected particles 
//

void MainWindow::on_action_RemoveParticles_triggered()
{
   main_viewer_form->remove_all_particles_selected();
   main_viewer_form->draw_current_frame();   
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ImageViewerMulti_triggered()
{

}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectionModePlusGlobal_triggered()
{
   main_viewer_form->set_is_selection_global(true);
   main_viewer_form->set_is_positive_selection_on(true);
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::RubberBandDrag); 
   action_ScrollHandDrag->setChecked(false);
   action_SelectionMode->setChecked(false);   
   action_SelectionModeMinus->setChecked(false);
   action_SelectionModePlusGlobal->setChecked(true);
   action_SelectionModeMinusGlobal->setChecked(false);
   main_viewer_form->draw_current_frame();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectionModeMinusGlobal_triggered()
{
   main_viewer_form->set_is_selection_global(true);
   main_viewer_form->set_is_positive_selection_on(false);
   main_viewer_form->graphicsView->setDragMode(QGraphicsView::RubberBandDrag); 
   action_ScrollHandDrag->setChecked(false);
   action_SelectionMode->setChecked(false);   
   action_SelectionModeMinus->setChecked(false);
   action_SelectionModePlusGlobal->setChecked(false);
   action_SelectionModeMinusGlobal->setChecked(true);
   main_viewer_form->draw_current_frame();
}

//////////////////////////////////////////////////////////////////////////

