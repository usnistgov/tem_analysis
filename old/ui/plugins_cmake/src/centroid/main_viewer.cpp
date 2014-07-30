#include "main_viewer.h"
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QStatusBar>


//////////////////////////////////////////////////////////////////////////

MainViewerForm::MainViewerForm(QWidget *parent) 
   : QWidget(parent),
     current_view_mode(0),
     current_frame(0),
     number_of_frames(0),
     is_animating(false)
{
   setupUi(this);

   // data
   //
   file_list_images = new QFileInfoList;
   particles = new Particles;

   scene = new ViewerGraphicsScene(this);
   graphicsView->setScene(scene);
   graphicsView->setRenderHint(QPainter::Antialiasing);
   graphicsView->setMouseTracking(true);
   graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
   graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); 
   graphicsView->setInteractive(true);

   scene->addText("Particles Viewer"); 

   timer = new QTimer(parent);
   connect(timer, SIGNAL(timeout()), this, SLOT(animate_loop())); 
   timer->stop();
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::setStatusBar(QStatusBar *statusBar)
{
   status_bar = statusBar;
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::clearScene() 
{
   scene->clear();
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::draw_frame(int frame)
{
   if (frame > get_number_of_frames()) return;

   current_frame = frame;

   QString statusStr;
   statusStr.append("Frame ");
   statusStr.append(QString::number(frame+1));
   statusStr.append(" of ");
   statusStr.append(QString::number(get_number_of_frames()));
   status_bar->showMessage(statusStr);

   scene->clear();

   switch(current_view_mode)
   {
      case VIEW_MODE_IMAGES_ONLY:

         if (file_list_images->size() > 0)
         {
            QImage *image = new QImage;
            image->load(file_list_images->at(current_frame).absoluteFilePath());
            QPixmap pixmap = QPixmap::fromImage(*image);
            scene->addPixmap(pixmap); 
            delete(image);
         }
         break;
      case VIEW_MODE_PARTICLES:
         particles->draw_frame(scene, frame);
         break;
   }
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::frame_next()
{
   if (current_frame < (get_number_of_frames()-1)) ++current_frame;
   draw_frame(current_frame);
   update();
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::frame_previous()
{
   if (current_frame > 0) --current_frame;
   draw_frame(current_frame);
   update();
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::zoom_in()
{
   graphicsView->scale(1.2f, 1.2f);   
   graphicsView->update();   
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::zoom_out()
{
   graphicsView->scale(1.0f/1.2f, 1.0f/1.2f);      
   graphicsView->update();   
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::animate_loop()
{
   ++current_frame;
   if (current_frame == get_number_of_frames()) current_frame = 0;
   draw_frame(current_frame);
   update();
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::animate_start_stop()
{
   if (!is_animating)
   {
      timer->start(100);
   } 
   else 
   {
      timer->stop();
   }
   is_animating = !is_animating;   
}

//////////////////////////////////////////////////////////////////////////

int MainViewerForm::get_number_of_frames()
{
   return number_of_frames;
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::images_from_dir(const QString& str)
{
   // reset viewing mode
   //
   current_view_mode = VIEW_MODE_IMAGES_ONLY;
   current_frame = 0;

   // prepare list with of files
   //
   QDir dir;
   dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
   dir.setSorting(QDir::Name);
   dir.setPath(str);
   QStringList filters;
   filters << "*.jpg" << "*.png" << "*.tif" << "*.tiff" << "*.bmp";
   dir.setNameFilters(filters);

   file_list_images->clear();
   file_list_images->append(dir.entryInfoList());

   number_of_frames = file_list_images->size();

   draw_frame(0);
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::particles_from_dir(const QString& str)
{
   // reset viewing mode
   //
   current_view_mode = VIEW_MODE_PARTICLES;
   current_frame = 0;

   // read particles data
   //
   particles->read_data_from_dir(str);
   particles->process_particles();
   particles->process_particles_sustained();
   particles->process_particles_doubles();

   number_of_frames = particles->get_number_of_frames();

   // file_list_images->clear();
   // file_list_images->append(dir.entryInfoList());

   draw_frame(0);
}

/////////////////////////////////////////////////////////////////////////



