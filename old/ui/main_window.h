#ifndef __H_MAIN_WINDOW__
#define __H_MAIN_WINDOW__

#include "ui_main_window.h"
#include "interface.h"
#include "block.h"

class MainViewerForm;

//////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow, private Ui_MainWindow
{
   Q_OBJECT

public:
   MainWindow();

   void addModuleToDock(Block *block);

private slots:
   void on_action_ReadData_triggered(); 
   void on_action_ImageViewer_triggered();
   void on_action_PlayFrames_triggered();
   void on_action_PreviousFrame_triggered();
   void on_action_NextFrame_triggered();
   void on_action_ZoomIn_triggered();
   void on_action_ZoomOut_triggered();
   void on_action_ScrollHandDrag_triggered();
   void on_action_SelectionMode_triggered();
   void on_action_ParticlesViewer_triggered();

private:   
   void viewer_icons_show(bool is_enabled);

private:
   QVector<Block*> blocks;
   MainViewerForm *main_viewer_form;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_MAIN_WINDOW__
