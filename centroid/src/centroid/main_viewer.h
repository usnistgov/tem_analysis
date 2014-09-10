#ifndef __H_MAIN_VIEWER__
#define __H_MAIN_VIEWER__

#include "ui_main_viewer.h"
#include <QFileInfoList>
#include "particles.h"
#include "viewer_graphics_scene.h"

#include <QtGlobal>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

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

   enum TriangulationMode {
        Triangulate_All,
        Triangulate_Selected
   };
        
   void setStatusBar(QStatusBar *statusBar);

   void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

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

   void set_triangulation_visibility (bool onOff);
   void set_particle_visibility (bool onOff);
   void set_image_visibility (bool onOff);

   void addAtom (float x, float y, bool allFrames);


   bool get_is_selection_global();
   void set_is_selection_global(bool state);

   void set_is_positive_selection_on(bool state) { is_positive_selection_on = state; }
   bool get_is_positive_selection_on() { return is_positive_selection_on; }

   void remove_all_particles_selected();

   void invert_particle_selection_all ();
   void invert_particle_selection_curr ();

   void deselect_all_particles ();
   void deselect_all_particles_curr ();

   void select_all_particles_global(const QPointF& p0, const QPointF& p1, bool state);
   void select_all_particles_current_frame (const QPointF& p0, 
                                            const QPointF& p1, 
                                            bool state);

    
   void set_interactionMode (InteractionMode iMode);
   InteractionMode get_interactionMode ();

   void set_triangulationMode (TriangulationMode tMode);
   TriangulationMode get_triangulationMode ();

    QString get_frame_image_filename (int);
    QString get_frame_particle_filename (int);

    void writeActiveAtomPosFile (const int frame, const QString fileName);

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
   void on_actionLoop_toggled(bool);


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
   TriangulationMode triangulationMode;

   // data
   //
   QFileInfoList *file_list_images;
   Particles *particles;

   bool animate_forward;
   QTimer *timer;

   QStatusBar *status_bar;

   bool image_visible;
   bool particles_visible;
   bool triangulation_visible;

   bool loopAnimation;
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
