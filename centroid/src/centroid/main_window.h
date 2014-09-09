#ifndef __H_MAIN_WINDOW__
#define __H_MAIN_WINDOW__

#include "ui_main_window.h"
#include "main_viewer.h"
#include "filter_interface.h"
#include "block.h"

class MainViewerForm;
class QFileDialog;

//////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow, private Ui_MainWindow
{
   Q_OBJECT

public:
   MainWindow();

   void addModuleToDock(Block *block);
   void writeLog ( const QString & message );

signals:
   void aboutToLoadProject(Project* proj);

private slots:
   void on_action_About_triggered();
   void on_action_Quit_triggered();
   void on_action_NewProject_triggered();
   void on_action_OpenProject_triggered();
   void on_action_ReadData_triggered(); 
   void on_action_ImageViewer_triggered();
   void on_action_ImageViewerMulti_triggered();
   void on_action_ZoomIn_triggered();
   void on_action_ZoomOut_triggered();
   void on_action_ScrollHandDrag_triggered();
   void on_action_SelectionMode_triggered();
   void on_action_ParticlesViewer_triggered();
   void on_action_SelectParticles_triggered();

   // void on_action_Photo_triggered();
   // void on_action_PhotoAll_triggered();
   void on_actionSnapshotAllFrames_triggered();
   void on_actionSnapshot_Current_Frame_triggered();

   void on_action_LineSelection_triggered();
   void on_action_ShowLines_triggered();
   void on_action_SelectionModeMinus_triggered();
   void on_action_RemoveParticles_triggered();
   void on_action_SelectionModePlusGlobal_triggered();
   void on_action_SelectionModeMinusGlobal_triggered();
   void on_action_AddAtom_triggered();

   void on_action_Test1_triggered();
   void on_action_Test2_triggered();
   
   void on_actionShowHideImages_toggled(bool);
   void on_actionShowHideAtoms_toggled(bool);
   void on_actionShowHideTriangulation_toggled(bool);

   void on_actionSelectInAllFrames_toggled(bool);
   void on_actionAddInAllFrames_toggled(bool);

   void on_actionSave_Snapshots_triggered();
   // void on_actionSave_Atom_Positions_triggered();
   void on_actionSaveAtomPosAll_triggered();
   void on_actionSaveAtomPosCurr_triggered();
   void on_actionSave_Triangulation_triggered();
   void on_actionDeleteSelected_triggered();
   void on_actionDeleteUnSelected_triggered();
   void on_actionInvertSelection_triggered();
   void on_actionBoxSelection_triggered();
   void on_actionAddAtom_triggered();
   void on_actionHandDragMode_triggered();



    void on_actionDelAtomMode_triggered();
    void on_actionTriMode_triggered();

    void on_actionDeselectAllAtoms_triggered();

    void on_actionTriangSel_triggered();
    void on_actionClearTriang_triggered();






protected:
   virtual void closeEvent(QCloseEvent*);

private:   
   void openProject(QString fileName);
   void loadProject(Project* project);
   void read_settings();
   void write_settings();
   void viewer_icons_show(bool is_enabled);
   void viewer_particle_icons_show(bool is_enabled);
   void set_interactionMode (MainViewerForm::InteractionMode iMode);

private:
   QFileDialog *file_dialog;
   QVector<Block*> blocks;
   MainViewerForm *main_viewer_form;
   Project* current_project;
   bool selAtomsInAllFrames;
   bool addAtomsInAllFrames;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_MAIN_WINDOW__
