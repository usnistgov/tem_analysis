#ifndef __H_MAIN_VIEWER__
#define __H_MAIN_VIEWER__

#include "ui_main_viewer.h"
#include <QFileInfoList>
#include "particles.h"
#include "viewer_graphics_scene.h"


class QStatusBar;

//////////////////////////////////////////////////////////////////////////

#define VIEW_MODE_IMAGES_ONLY 1
#define VIEW_MODE_PARTICLES 2

//////////////////////////////////////////////////////////////////////////

class MainViewerForm : public QWidget, public Ui::MainViewerForm
{
   Q_OBJECT

public:
   MainViewerForm(QWidget *parent = 0);
   void setStatusBar(QStatusBar *statusBar);

   void clearScene();
   void draw_frame(int frame);

   void images_from_dir(const QString& dir);
   void particles_from_dir(const QString& dir);

   void frame_next();
   void frame_previous();
   void zoom_in();
   void zoom_out();

   void animate_start_stop();

   int get_number_of_frames();

private slots:
   void animate_loop();

private:
   ViewerGraphicsScene *scene;
   unsigned int current_view_mode;

   int current_frame;
   int number_of_frames;

   // data
   //
   QFileInfoList *file_list_images;
   Particles *particles;

   bool is_animating;
   QTimer *timer;

   QStatusBar *status_bar;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_MAIN_VIEWER__
