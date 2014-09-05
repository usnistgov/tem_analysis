#include <iostream>

#include <QtWidgets>
#include <QPluginLoader>
#include <QDebug>

#include "main_window.h"
#include "modules_manager.h"

#include "module_button.h"

#include "main_viewer.h"
#include "new_project.h"
#include "version.h"


//////////////////////////////////////////////////////////////////////////

void MainWindow::addModuleToDock(Block *block)
{
   qDebug() << "Adding block" << block->getBlockName() << "to dock";
   QTreeWidgetItem *category = new QTreeWidgetItem();

   treeWidget->addTopLevelItem(category);
   treeWidget->
    setItemWidget( category, 0, 
                   new ModuleButton(block->getBlockName(), 
                                    treeWidget, category) ); 

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

         DirUpdateButton *btn_file = new DirUpdateButton("..", file_dialog);
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

   block->setStatusBar(statusbar);

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
    : current_project(NULL)
{
   setupUi(this); 

   file_dialog = new QFileDialog(this);

   action_Quit->setShortcut(QKeySequence::Quit);
   action_NewProject->setShortcut(QKeySequence::New);
   action_OpenProject->setShortcut(QKeySequence::Open);

   ModulesManager::instance()->init(plainLogsTextEdit);
   connect(this, SIGNAL(aboutToLoadProject(Project*)),
           ModulesManager::instance(), SLOT(initProject(Project*)));

   // add action allowsing to close/view docking widgets
   // 
   //menuView->addAction(dockModulesWidget->toggleViewAction());
   // menuView->addAction(dockModulesWidget_2->toggleViewAction());
   menuView->addAction(toolsDockWidget->toggleViewAction());
   menuView->addAction(dockLogsWidget->toggleViewAction());

   // set central widget  
   //
   main_viewer_form = new MainViewerForm;
   //main_viewer_form->setStatusBar(statusbar);
   setCentralWidget(main_viewer_form);

   // disable viewer's icons initially
   //
   viewer_icons_show(false);
   viewer_particle_icons_show(false);

   // default mode for interaction with the viewer
   //
   // action_ScrollHandDrag->setChecked(true);
   set_interactionMode (MainViewerForm::HandDrag);



   setUnifiedTitleAndToolBarOnMac(true);
   read_settings();

}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_About_triggered()
{
    QMessageBox::about(this, "TEM Analysis",
        QString("Version: %1\n\nContact: Wesley Griffin\nwesley.griffin@nist.gov").arg(TEM_ANALYSIS_VERSION));
}

void MainWindow::on_action_Quit_triggered()
{
    close();
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_NewProject_triggered()
{
    int result;
    Project* project = NULL;
    QSettings settingsFile;

    NewProjectDialog newProjectDialog(this);
    newProjectDialog.setDirectory(file_dialog->directory());
    newProjectDialog.setModal(true);

    do {
        result = newProjectDialog.exec();
        if (result == QDialog::Accepted) {
            file_dialog->setDirectory(newProjectDialog.getDirectory());
            project = newProjectDialog.saveProject();
            loadProject(project);
        }
    } while (result != QDialog::Rejected && project == NULL);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_OpenProject_triggered()
{
   file_dialog->setWindowTitle("Select project file");
   file_dialog->setFileMode(QFileDialog::ExistingFile);

   if (!file_dialog->exec()) {
       qDebug() << "OpenProject dialog cancelled";
       return;
   }

   QString fileName = file_dialog->selectedFiles()[0];
   openProject(fileName);
}

void MainWindow::openProject(QString fileName)
{
   qDebug() << "Opening project from" << fileName;
   Project* project = new Project;
   if (!project->load(fileName))
   {
       qWarning() << "Could not load" << fileName;
       QMessageBox::critical(this, "Error", "Could not load " + fileName);
       return;
   }

   loadProject(project);
}

void MainWindow::loadProject(Project* project)
{
   qDebug() << "Loading project:" << project;
   treeWidget->clear();
   blocks.clear();

   treeWidget->setRootIsDecorated(false);
   treeWidget->setIndentation(0);

   if (current_project) delete current_project;
   current_project = project;

   emit aboutToLoadProject(current_project);
   ModulesManager::instance()->setupAllFilterBlocks(blocks);

   QVectorIterator<Block*> i(blocks);
   while (i.hasNext())
   {
      addModuleToDock(i.next());
   }

   file_dialog->setDirectory(current_project->getBaseDirectory());
   setWindowTitle(current_project->getShortTag() + " | TEM Analysis");
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
    qDebug() << "Received closeEvent";
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
    if (settings.contains("app/currentproject"))
    {
        openProject(settings.value("app/currentproject").toString());
    }
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::write_settings() {
    QSettings settings;
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
    settings.setValue("app/opendialogstate", file_dialog->saveState());
    settings.setValue("app/opendialogdirectory", file_dialog->directory().path());
    if (current_project)
    {
        settings.setValue("app/currentproject", current_project->getProjectFileName());
    }
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

void MainWindow::set_interactionMode (MainViewerForm::InteractionMode iMode)
{
   qDebug() << "Set interactionMode to" << iMode;
   action_ScrollHandDrag->setChecked(false);
   action_SelectionMode->setChecked(false);
   action_SelectionModeMinus->setChecked(false);
   action_SelectionModePlusGlobal->setChecked(false);   
   action_SelectionModeMinusGlobal->setChecked(false);   
   action_AddAtom->setChecked(false);  

    switch (iMode)
        {
        case MainViewerForm::HandDrag:
            action_ScrollHandDrag->setChecked(true);
            break;

        case MainViewerForm::SelectAtomsCurrFrame:
            action_SelectionMode->setChecked(true);
            break;

        case MainViewerForm::DeselectAtomsCurrFrame:
            action_SelectionModeMinus->setChecked(true);
            break;

        case MainViewerForm::SelectAtomsAllFrames:
            action_SelectionModePlusGlobal->setChecked(true);
            break;

        case MainViewerForm::DeselectAtomsAllFrames:
            action_SelectionModeMinusGlobal->setChecked(true);
            break;

        case MainViewerForm::AddAtomCurrFrame:
            action_AddAtom->setChecked(true);
            break;

        case MainViewerForm::AddAtomAllFrames:
            // nothing here yet
            break;

        }

   main_viewer_form->set_interactionMode (iMode);

}  

void MainWindow::on_action_ScrollHandDrag_triggered()
{
   set_interactionMode (MainViewerForm::HandDrag);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectionMode_triggered()
{
   set_interactionMode (MainViewerForm::SelectAtomsCurrFrame);
}

//////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_SelectionModeMinus_triggered()
{
   set_interactionMode (MainViewerForm::DeselectAtomsCurrFrame);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectParticles_triggered()
{
   // JGH: I don't think that this function is being used

   set_interactionMode (MainViewerForm::SelectAtomsCurrFrame);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_ParticlesViewer_triggered()
{
   file_dialog->setWindowTitle("Select particles directory");
   file_dialog->setFileMode(QFileDialog::Directory);

   if (!file_dialog->exec()) {
       qDebug() << "ParticlesViewer dialog cancelled";
       return;
   }

   QString directory = file_dialog->selectedFiles()[0];
   qDebug() << "Loading particles from" << directory;
   main_viewer_form->particles_from_dir(directory);

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

   if (!file_dialog->exec()) {
       qDebug() << "ImageViewer dialog cancelled";
       return;
   }

   QString directory = file_dialog->selectedFiles()[0];
   qDebug() << "Loading images from" << directory;
   main_viewer_form->images_from_dir(directory);

   if (main_viewer_form->get_number_of_frames() > 0)
   {
      viewer_icons_show(true);
   }
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_Photo_triggered()
{
   file_dialog->setWindowTitle("Select screenshot file name");
   file_dialog->setFileMode(QFileDialog::AnyFile);

   if (!file_dialog->exec()) {
       qDebug() << "Photo dialog cancelled";
       return;
   }

   QString fileName = file_dialog->selectedFiles()[0];
   qDebug() << "Saving screenshot to" << fileName;

   // revised to grab only the viewport; omit scrollbars
   // QPixmap pm = main_viewer_form->graphicsView->grab(main_viewer_form->graphicsView->rect()); 
   QWidget * viewport = main_viewer_form->graphicsView->viewport();
   QPixmap pm = viewport->grab(viewport->rect()); 
   pm.save(fileName);  


   // test 
   // on_action_PhotoAll_triggered();
}

//////////////////////////////////////////////////////////////////////////

// This seems to work
void MainWindow::on_action_PhotoAll_triggered()
{

    // Loop through frames; grab and write image file for each

    int nFrames = main_viewer_form->get_number_of_frames ();
    int currFrame = main_viewer_form->get_current_frame ();

    QString outFN;  // to be constructed in the loop

    // Get directory where we'll write the output images
    file_dialog->setWindowTitle("Select output folder for screenshots");
    file_dialog->setFileMode(QFileDialog::Directory);

    if (!file_dialog->exec()) {
       qDebug() << "Output folder dialog cancelled.";
       return;
    }


    QString outDir = file_dialog->selectedFiles()[0];



    QWidget * viewport = main_viewer_form->graphicsView->viewport();

    QString projTag = current_project->getShortTag ();

    QString outTag = projTag + ".grab";


    // loop through all frames
    for (int i = 0; i < nFrames; i++)
        {
        // start at curr frame and wrap around
        int f = (i + currFrame) % nFrames;


        // draw it and grab the pixmap
        main_viewer_form->draw_frame (f);
        QPixmap pm = viewport->grab(viewport->rect()); 

        // construct file name based on f and outDir
        char cSeqStr[1000];
        sprintf (cSeqStr, "%06d", f);
        // QString qSeqStr (cSeqStr);
        outFN = outDir + "/" + QString (cSeqStr) + "." + outTag + ".png";
    

        //FilterSupport::makeOutFN ( "00000."+projTag+".grab.png", // not used
            // outDir, false, f, outTag, "png", outFN);

        qDebug () << "writing file " + outFN;
        // write it out
        pm.save (outFN);
        }

    // return to what was the current frame
    main_viewer_form->draw_frame (currFrame);

    return;
} // end of void MainWindow::on_action_PhotoAll_triggered()


//////////////////////////////////////////////////////////////////////////
//
// Select particles along the line
//

void MainWindow::on_action_LineSelection_triggered()
{
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_Test1_triggered()
{
    printf ("Test1 triggered\n");
}


void MainWindow::on_action_Test2_triggered()
{
    printf ("Test2 triggered\n");
}



void MainWindow::on_actionShowHideImages_toggled (bool o)
{
    printf ("show / hide Images: %s\n", o ? "ON" : "OFF");
}


void MainWindow::on_actionShowHideAtoms_toggled (bool o)
{
    printf ("show / hide Atoms: %s\n", o ? "ON" : "OFF");
}


void MainWindow::on_actionShowHideTriangulation_toggled (bool o)
{
    printf ("show / hide Tri : %s\n", o ? "ON" : "OFF");
}



void MainWindow::on_actionSelectInAllFrames_toggled(bool o)
{
    printf ("select in all : %s\n", o ? "ON" : "OFF");
}

void MainWindow::on_actionAddInAllFrames_toggled(bool o)
{
    printf ("add in all : %s\n", o ? "ON" : "OFF");
}


void MainWindow::on_actionSaveSnaps_triggered()
{
    printf ("save snaps triggered\n");
}

void MainWindow::on_actionSaveAtoms_triggered()
{
    printf ("save atoms triggered\n");
}

void MainWindow::on_actionSaveTri_triggered()
{
    printf ("save tri triggered\n");
}

void MainWindow::on_actionDeleteSelected_triggered()
{
    printf ("save delete selected triggered\n");
}

void MainWindow::on_actionDeleteUnSelected_triggered()
{
    printf ("save delete unselected triggered\n");
}

void MainWindow::on_actionInvertSelection_triggered()
{
    printf ("save invert selection triggered\n");
}

void MainWindow::on_actionBoxSelection_triggered()
{
    printf ("save box sel mode triggered\n");
}

void MainWindow::on_actionAddAtom_triggered()
{
    printf ("save add atom mode triggered\n");
}

void MainWindow::on_actionHandDragMode_triggered()
{
    printf ("save hand drag mode triggered\n");
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


void MainWindow::on_action_AddAtom_triggered()
{
   set_interactionMode (MainViewerForm::AddAtomCurrFrame);
}




//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectionModePlusGlobal_triggered()
{
   set_interactionMode (MainViewerForm::SelectAtomsAllFrames);
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::on_action_SelectionModeMinusGlobal_triggered()
{
   set_interactionMode (MainViewerForm::DeselectAtomsAllFrames);
}

