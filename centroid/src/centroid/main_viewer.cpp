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
     current_frame(0),
     number_of_frames(0),
     animate_forward(true),
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
   viewer_icons_show(false);

   connect(frameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(set_frame(int)));
   connect(frameSlider, SIGNAL(valueChanged(int)), this, SLOT(set_frame(int)));

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
   if (frame > get_number_of_frames() || frame < 0) return;

   current_frame = frame;
   frameSpinBox->setValue(current_frame+1);
   frameSlider->setValue(current_frame+1);
   scene->clear();

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

   if (particles->get_number_of_frames() > 0)
   {
      particles->set_is_positive_selection_on(is_positive_selection_on);
      if (is_triangulation_on)
      {
         particles->draw_triangles(scene, frame);
      }
      particles->draw_particles(scene, frame);
   }
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

void MainViewerForm::on_action_JumpToFirst_triggered()
{
   draw_frame(0);
   update();
}

void MainViewerForm::on_action_PlayBackward_triggered()
{
   if (get_number_of_frames() > 1 && !timer->isActive())
   {
      animate_forward = false;
      timer->start(100);
   }
}

void MainViewerForm::on_action_StepBackward_triggered()
{
   if (current_frame > 0)
   {
      draw_frame(current_frame-1);
      update();
   }
}

void MainViewerForm::on_action_Pause_triggered()
{
   timer->stop();
}

void MainViewerForm::on_action_StepForward_triggered()
{
   if (current_frame < (get_number_of_frames()-1))
   {
      draw_frame(current_frame+1);
      update();
   }
}

void MainViewerForm::on_action_PlayForward_triggered()
{
   if (get_number_of_frames() > 1 && !timer->isActive())
   {
      animate_forward = true;
      timer->start(100);
   }
}

void MainViewerForm::on_action_JumpToLast_triggered()
{
   draw_frame(get_number_of_frames()-1);
   update();
}

void MainViewerForm::set_frame(int frame)
{
   draw_frame(frame-1);
   update();
}

void MainViewerForm::animate_loop()
{
   if (animate_forward)
   {
       draw_frame((current_frame+1) % get_number_of_frames());
   }
   else
   {
       int frame = current_frame-1;
       if (frame < 0) frame = get_number_of_frames()-1;
       draw_frame(frame);
   }

   update();
}

void MainViewerForm::viewer_icons_show(bool is_enabled)
{
   action_JumpToFirst->setEnabled(is_enabled);
   jumpToFirstButton->setEnabled(is_enabled);
   action_PlayBackward->setEnabled(is_enabled);
   playBackwardButton->setEnabled(is_enabled);
   action_StepBackward->setEnabled(is_enabled);
   stepBackwardButton->setEnabled(is_enabled);
   action_Pause->setEnabled(is_enabled);
   pauseButton->setEnabled(is_enabled);
   action_StepForward->setEnabled(is_enabled);
   stepForwardButton->setEnabled(is_enabled);
   action_PlayForward->setEnabled(is_enabled);
   playForwardButton->setEnabled(is_enabled);
   action_JumpToLast->setEnabled(is_enabled);
   jumpToLastButton->setEnabled(is_enabled);
   frameSlider->setEnabled(is_enabled);
   frameSpinBox->setEnabled(is_enabled);
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
   frameSpinBox->setMaximum(number_of_frames);
   frameSlider->setMaximum(number_of_frames);

   draw_frame(0);
   viewer_icons_show(true);
}

//////////////////////////////////////////////////////////////////////////

void MainViewerForm::particles_from_dir(const QString& str)
{
   // reset viewing mode
   //
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
   viewer_icons_show(true);
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


void MainViewerForm::select_all_particles_current_frame (const QPointF& p0, const QPointF& p1, bool state)
{
   particles->set_selected (current_frame, p0, p1, state);
}

/////////////////////////////////////////////////////////////////////////





