#include "main_viewer.h"
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QStatusBar>

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkCropImageFilter.h"

//////////////////////////////////////////////////////////////////////////

MainViewerForm::MainViewerForm(QWidget *parent) 
   : QWidget(parent),
     current_view_mode(0),
     current_frame(0),
     number_of_frames(0),
     is_animating(false),
     is_triangulation_on(false),
     is_positive_selection_on(true),
     is_selection_global(false)
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

typedef float FloatPixelType;
typedef itk::Image< FloatPixelType, 2 > FloatImageType;
typedef unsigned char UCharPixelType;
typedef itk::Image< UCharPixelType, 2 > UCharImageType;
typedef itk::ImageFileReader< FloatImageType > FloatReaderType;

static int
readAndRescaleTo8Bit(const char *inFN, UCharImageType::Pointer & img)
{
   FloatReaderType::Pointer reader = FloatReaderType::New();
   reader->SetFileName( inFN );

   // remap pixel values from input max,min to 0,255.
   typedef itk::RescaleIntensityImageFilter<FloatImageType, UCharImageType > RescaleFilterType;

   RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
   rescaler->SetOutputMinimum(   0 );
   rescaler->SetOutputMaximum( 255 );

   rescaler->SetInput ( reader->GetOutput() );

   img = rescaler->GetOutput();

   img->Update();

   return 0;
}  // end of readAndRescaleTo8Bit

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
            if (image->load(file_list_images->at(current_frame).absoluteFilePath()));
            {
               if (image->format() == QImage::Format_Invalid)
               {
                  UCharImageType::Pointer img;
                  readAndRescaleTo8Bit (file_list_images->at(current_frame).absoluteFilePath().toStdString().c_str(), img);
                  const UCharImageType::SizeType & imgSize = img->GetLargestPossibleRegion().GetSize();
                  unsigned char *imgData = img->GetBufferPointer();

                  QImage *imageBW = new QImage(imgSize[0], imgSize[1], QImage::Format_RGB32);

                  for (unsigned int i=0; i<imgSize[0]; i++)
                  {
                     for(unsigned int j=0; j<imgSize[1]; j++)
                     {
                        unsigned int pv= imgData[i+j*imgSize[0]];
                        imageBW->setPixel(i, j, pv+(pv<<8)+(pv<<16));
                     }
                  }
                  QPixmap pixmap = QPixmap::fromImage(*imageBW);
                  scene->addPixmap(pixmap); 
                  delete(image);
               }
               else
               {
                  QPixmap pixmap = QPixmap::fromImage(*image);
                  scene->addPixmap(pixmap); 
                  delete(image);
               }
            }
         }
         break;
      case VIEW_MODE_PARTICLES:
            particles->set_is_positive_selection_on(is_positive_selection_on);
            particles->draw_background_image(scene, frame);
            if (is_triangulation_on)
            {
               particles->draw_triangles(scene, frame);
            }
            particles->draw_particles(scene, frame);
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
   if (particles->get_number_of_frames() == 0)
   {
      particles->process_particles();
      particles->process_particles_sustained();
      particles->process_particles_doubles();
   }

   number_of_frames = particles->get_number_of_frames();

   // file_list_images->clear();
   // file_list_images->append(dir.entryInfoList());

   draw_frame(0);
}

/////////////////////////////////////////////////////////////////////////

bool MainViewerForm::get_is_triangulation_on()
{
   return is_triangulation_on;
}

/////////////////////////////////////////////////////////////////////////

void MainViewerForm::set_is_triangulation_on(bool state)
{
   is_triangulation_on = state;
}

/////////////////////////////////////////////////////////////////////////

void MainViewerForm::draw_current_frame()
{
   draw_frame(current_frame);
}
   
/////////////////////////////////////////////////////////////////////////

void MainViewerForm::remove_all_particles_selected()
{
   particles->remove_all_particles_selected();
}

/////////////////////////////////////////////////////////////////////////

bool MainViewerForm::get_is_selection_global()
{
   return is_selection_global;
}

/////////////////////////////////////////////////////////////////////////

void MainViewerForm::set_is_selection_global(bool state)
{
   is_selection_global = state;
}

/////////////////////////////////////////////////////////////////////////

void MainViewerForm::select_all_particles_global(const QPointF& p0, const QPointF& p1, bool state)
{
   particles->set_selected_global(p0, p1, state);
}

/////////////////////////////////////////////////////////////////////////





