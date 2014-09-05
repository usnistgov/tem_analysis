#ifndef __H_MAIN_VIEWER__
#define __H_MAIN_VIEWER__

#include "ui_main_viewer.h"
#include <QFileInfoList>
#include "particles.h"
#include "viewer_graphics_scene.h"

#include <QtGlobal>
#include <QDebug>

class QStatusBar;

//////////////////////////////////////////////////////////////////////////

class MainViewerForm : public QWidget, public Ui::MainViewerForm
{
   Q_OBJECT

public:

   MainViewerForm(QWidget *parent = 0);

   enum InteractionMode {
        HandDrag,
        SelectAtomsCurrFrame,
        DeselectAtomsCurrFrame,
        SelectAtomsAllFrames,
        DeselectAtomsAllFrames,
        AddAtomCurrFrame,
        AddAtomAllFrames
   };
        
   void setStatusBar(QStatusBar *statusBar);

   void clearScene();
   void draw_frame(int frame);
   void draw_current_frame();
   int get_current_frame();
   int get_number_of_frames();

   void images_from_dir(const QString& dir);
   void particles_from_dir(const QString& dir);

   void zoom_in();
   void zoom_out();

   bool get_is_triangulation_on();
   void set_is_triangulation_on(bool state);
   bool get_is_selection_global();
   void set_is_selection_global(bool state);

   void set_is_positive_selection_on(bool state) { is_positive_selection_on = state; }
   bool get_is_positive_selection_on() { return is_positive_selection_on; }

   void remove_all_particles_selected();
   void select_all_particles_global(const QPointF& p0, const QPointF& p1, bool state);
   void select_all_particles_current_frame (const QPointF& p0, 
                                            const QPointF& p1, 
                                            bool state);

   void set_interactionMode (InteractionMode iMode);
   InteractionMode get_interactionMode ();

private slots:

   void on_action_JumpToFirst_triggered();
   void on_action_PlayBackward_triggered();
   void on_action_StepBackward_triggered();
   void on_action_Pause_triggered();
   void on_action_StepForward_triggered();
   void on_action_PlayForward_triggered();
   void on_action_JumpToLast_triggered();
   void on_action_ZoomIn_triggered();
   void on_action_ZoomOut_triggered();

   void set_frame(int);
   void animate_loop();

private:
   void viewer_icons_show(bool is_enabled);

   ViewerGraphicsScene *scene;
   bool is_triangulation_on;

   // modifications to be carried out on every frame (if global) 
   // or current frame (of not global)
   bool is_selection_global;

   bool is_positive_selection_on;

   int current_frame;
   int number_of_frames;

   InteractionMode interactionMode;

   // data
   //
   QFileInfoList *file_list_images;
   Particles *particles;

   bool animate_forward;
   QTimer *timer;

   QStatusBar *status_bar;

};

//////////////////////////////////////////////////////////////////////////

inline QDebug operator<<(QDebug dbg, MainViewerForm::InteractionMode const& iMode) {
    switch(iMode) {
    case MainViewerForm::HandDrag:
        dbg.nospace() << "HandDrag";
        break;
    case MainViewerForm::SelectAtomsCurrFrame:
        dbg.nospace() << "SelectAtomsCurrFrame";
        break;
    case MainViewerForm::DeselectAtomsCurrFrame:
        dbg.nospace() << "DeselectAtomsCurrFrame";
        break;
    case MainViewerForm::SelectAtomsAllFrames:
        dbg.nospace() << "SelectAtomsAllFrames";
        break;
    case MainViewerForm::DeselectAtomsAllFrames:
        dbg.nospace() << "DeselectAtomsAllFrames";
        break;
    case MainViewerForm::AddAtomCurrFrame:
        dbg.nospace() << "AddAtomCurrFrame";
        break;
    case MainViewerForm::AddAtomAllFrames:
        dbg.nospace() << "AddAtomAllFrames";
        break;
    }

    return dbg.space();
}

#endif // __H_MAIN_VIEWER__
