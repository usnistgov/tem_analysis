#ifndef __H_MAIN_WINDOW__
#define __H_MAIN_WINDOW__

#include "ui_main_window.h"
#include "interface.h"
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

signals:
   void projectLoaded(Project* proj);

private slots:
   void on_action_Quit_triggered();
   void on_action_NewProject_triggered();
   void on_action_OpenProject_triggered();
   void on_action_ReadData_triggered(); 
   void on_action_ImageViewer_triggered();
   void on_action_ImageViewerMulti_triggered();
   void on_action_PlayFrames_triggered();
   void on_action_PreviousFrame_triggered();
   void on_action_NextFrame_triggered();
   void on_action_ZoomIn_triggered();
   void on_action_ZoomOut_triggered();
   void on_action_ScrollHandDrag_triggered();
   void on_action_SelectionMode_triggered();
   void on_action_ParticlesViewer_triggered();
   void on_action_SelectParticles_triggered();
   void on_action_Photo_triggered();
   void on_action_LineSelection_triggered();
   void on_action_ShowLines_triggered();
   void on_action_SelectionModeMinus_triggered();
   void on_action_RemoveParticles_triggered();
   void on_action_SelectionModePlusGlobal_triggered();
   void on_action_SelectionModeMinusGlobal_triggered();

protected:
   virtual void closeEvent(QCloseEvent*);

private:   
   void loadProject(Project* project);
   void read_settings();
   void write_settings();
   void viewer_icons_show(bool is_enabled);

private:
   QFileDialog *file_dialog;
   QVector<Block*> blocks;
   MainViewerForm *main_viewer_form;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_MAIN_WINDOW__
