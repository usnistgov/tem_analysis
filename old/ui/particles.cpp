#include "particles.h"
#include <QDir>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QtGui>
#include <iostream>
#include <math.h>

#include "viewer_graphics_scene.h"

/*
// for triangulation 
//
#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkDelaunay2D.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkXMLPolyDataWriter.h>
*/

//////////////////////////////////////////////////////////////////////////

Particles::Particles()
   : particle_time_sustained(5),
     particle_distance_threshold(0.75f),
     particle_size_threshold(50.0f)
{

}

//////////////////////////////////////////////////////////////////////////

void Particles::read_data_from_dir(const QString& str)
{
   // prepare list with of files
   //
   QDir dir;
   dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
   dir.setSorting(QDir::Name);
   dir.setPath(str);
   QStringList filters;
   filters << "*.list";
   dir.setNameFilters(filters);   

   QFileInfoList list = dir.entryInfoList();
   std::cout << "Number of files: " << list.size() << std::endl;

   list_all_particles.resize(list.size());

   // if (list.size()>0)
   // {
   //    timeScrollBar->setRange(0, list.size()-1);
   // }

   std::cout << "Reading data:" << std::endl;

   for (int i=0; i<list.size(); ++i) 
   {
      list_all_particles[i] = new particles;

      QFileInfo fileInfo = list.at(i);

      QFile file(fileInfo.absoluteFilePath());
      if (!file.open(QIODevice::ReadOnly)) 
      {
         qDebug() << file.errorString() << fileInfo.absoluteFilePath() << endl;
      }

      QTextStream in(&file);

      while (!in.atEnd()) 
      {
         QString line = in.readLine();    
         QStringList fields = line.split(QRegExp("\\s+")); 
         particle p;
         // p.id = fields[1].toInt();
         p.no_pixels = fields[2].toInt();
         p.x = fields[3].toFloat();
         p.y = fields[4].toFloat();
         p.index_closest_in_next_frame = -1;
         p.index_closest_in_previous_frame = -1;      
         p.dist_smallest_next_frame = 1000000.0f;
         p.dist_smallest_previous_frame = 1000000.0f;
         p.is_sustained = false;
         list_all_particles[i]->push_back(p);
         // qDebug() << fields;
      }   
      std::cout << "#" << std::flush;

   }   
   std::cout << std::endl << std::flush;

   // if (list_all_particles.size() > 0) isDataRead = true;
}

//////////////////////////////////////////////////////////////////////////

void Particles::process_particles()
{
   int i, j, k, m;

   for (k=0; k<list_all_particles.size()-1; k++)
   {
      //
      //
      // finds particle index and closest distance to particles (from next frame)

      particles *ps0 = list_all_particles[k];
      particles *ps1 = list_all_particles[k+1];

      for (i=0; i<ps0->size(); i++)
      {
         float x0 = ps0->at(i).x; 
         float y0 = ps0->at(i).y;
         float dist0 = ps0->at(i).dist_smallest_next_frame;
         int j0 = -1;

         for (j=0; j<ps1->size(); j++)
         {
            float x1 = ps1->at(j).x; 
            float y1 = ps1->at(j).y;

            float dx = x1-x0;
            float dy = y1-y0;

            float dist = sqrt(dx*dx + dy*dy);

            if (dist < dist0)
            {
               dist0 = dist;
               j0 = j;
            }
         }

         particle *p = ps0[0].data();
         p[i].dist_smallest_next_frame = dist0;
         p[i].index_closest_in_next_frame = j0;
      }

      // 
      //
      // finds particle index and closest distance to particles (from previous frame)

      m = k+1;

      ps0 = list_all_particles[m];
      ps1 = list_all_particles[m-1];

      for (i=0; i<ps0->size(); i++)
      {
         float x0 = ps0->at(i).x; 
         float y0 = ps0->at(i).y;
         float dist0 = ps0->at(i).dist_smallest_previous_frame;
         int j0 = -1;

         for (j=0; j<ps1->size(); j++)
         {
            float x1 = ps1->at(j).x; 
            float y1 = ps1->at(j).y;

            float dx = x1-x0;
            float dy = y1-y0;

            float dist = sqrt(dx*dx + dy*dy);

            if (dist < dist0)
            {
               dist0 = dist;
               j0 = j;
            }
         }

         particle *p = ps0[0].data();
         p[i].dist_smallest_previous_frame = dist0;
         p[i].index_closest_in_previous_frame = j0; 
      }

      //
      //
   }   
}

//////////////////////////////////////////////////////////////////////////

void Particles::process_particles_sustained()
{
   int k, i, j;

   if ((list_all_particles.size()-particle_time_sustained) > 0)
   {
   //
   for (k=0; k<list_all_particles.size()-particle_time_sustained; k++)
   {
      particles *ps0 = list_all_particles[k];

      for (i=0; i<ps0->size(); i++)
      {
         int isSustained = 0;
         int index_next = ps0->at(i).index_closest_in_next_frame;

         if (ps0->at(i).dist_smallest_next_frame < particle_distance_threshold)
         {
            isSustained = 1;
            for (j=0; j<particle_time_sustained-1; j++)
            {
               particles *ps1 = list_all_particles[k+j+1];
               if (ps1->at(index_next).dist_smallest_next_frame < particle_distance_threshold)
               {
                  isSustained++;
               }
               int ii = index_next;
               index_next = ps1->at(ii).index_closest_in_next_frame;
            }
         }
         if (isSustained == particle_time_sustained) 
         {
            particle *p = ps0[0].data();
            p[i].is_sustained = true;
         }
      }
   }
   //
   }
}

//////////////////////////////////////////////////////////////////////////

void Particles::process_particles_doubles()
{
   int i, j, k;

   for (k=0; k<list_all_particles.size(); k++)
   {
      particles *ps0 = list_all_particles[k];

      for (i=0; i<ps0->size(); i++)
      {
         int index = ps0->at(i).index_closest_in_next_frame;
         int count = 0;
         for (j=0; j<ps0->size(); j++)
         {
            if (index == ps0->at(j).index_closest_in_next_frame) count++;
         }
         // std::cout << count << " ";
      }
      // std::cout << std::endl;
   }
}


//////////////////////////////////////////////////////////////////////////

int Particles::get_number_of_frames()
{
   return list_all_particles.size();
}

//////////////////////////////////////////////////////////////////////////

void Particles::draw_frame(ViewerGraphicsScene *scene, int frame)
{
  
   if (frame < list_all_particles.size())
   {
      particles *ps = list_all_particles[frame];

      QBrush blackBrush(QColor(0, 0, 0, 127)); //Qt::black,);
      QBrush redBrush(Qt::red);
      QBrush greenBrush(Qt::green);
      QPen blackOutlinePen(QColor(0, 0, 0, 0));
      QPen redOutlinePen(Qt::red);
      QPen greenOutlinePen(Qt::green);
      blackOutlinePen.setWidth(1);
      redOutlinePen.setWidth(1);
      greenOutlinePen.setWidth(1);

      // std::cout << "Number of particles " << ps->size() << " in frame " << frame << std::endl;
      int i;

      // very messy drawing prototype
      //

bool isSustainedOnlyDisplayed = false;

      if (isSustainedOnlyDisplayed)
      {
         for (i=0; i<ps->size(); i++)
         {
            if (ps->at(i).is_sustained) {

            if ((ps->at(i).dist_smallest_next_frame > particle_distance_threshold) || (ps->at(i).dist_smallest_previous_frame > particle_distance_threshold))
            {
               scene->addEllipse(ps->at(i).x-1, ps->at(i).y-1, 2, 2, blackOutlinePen, blackBrush);
            }
            else
            {
               if (ps->at(i).no_pixels < particle_size_threshold)
               {
                  scene->addEllipse(ps->at(i).x-1, ps->at(i).y-1, 2, 2, redOutlinePen, redBrush);           
               }
               else 
               {
                  scene->addEllipse(ps->at(i).x-1, ps->at(i).y-1, 2, 2, greenOutlinePen, greenBrush);           
               }
            }
            }
         }
      }
      else
      {
         for (i=0; i<ps->size(); i++)
         {

            if ((ps->at(i).dist_smallest_next_frame > particle_distance_threshold) || (ps->at(i).dist_smallest_previous_frame > particle_distance_threshold))
            {
               scene->addEllipse(ps->at(i).x-1, ps->at(i).y-1, 2, 2, blackOutlinePen, blackBrush);
            }
            else
            {
               if (ps->at(i).no_pixels < particle_size_threshold)
               {
                  scene->addEllipse(ps->at(i).x-1, ps->at(i).y-1, 2, 2, redOutlinePen, redBrush);           
               }
               else 
               {
                  scene->addEllipse(ps->at(i).x-1, ps->at(i).y-1, 2, 2, greenOutlinePen, greenBrush);           
               }
            }
         }
      }

      // current_frame = frame;
   }

/*
// if (isDataRead)
// {
   particles *ps0 = list_all_particles[frame];

   // Create a grid of points (height/terrian map)
   vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

   double xx, yy, zz;
   for (int i=0; i<ps0->size(); i++)
   {
      xx = ps0->at(i).x;
      yy = ps0->at(i).y;
      zz = vtkMath::Random(-.5, .5);
      points->InsertNextPoint(xx, yy, zz);
   }

   // Add the grid points to a polydata object
   vtkSmartPointer<vtkPolyData> inputPolyData = vtkSmartPointer<vtkPolyData>::New();
   inputPolyData->SetPoints(points);
 
   // Triangulate the grid points
   vtkSmartPointer<vtkDelaunay2D> delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
   
   #if VTK_MAJOR_VERSION <= 5
      delaunay->SetInput(inputPolyData);
   #else
      delaunay->SetInputData(inputPolyData);
   #endif
   delaunay->Update();

   vtkPolyData* outputPolyData = delaunay->GetOutput();

   std::cout << "Number of polys: " << outputPolyData->GetNumberOfPolys() << std::endl;

   vtkIdType npts, *pts;
   outputPolyData->GetPolys()->InitTraversal();
   
   while (outputPolyData->GetPolys()->GetNextCell(npts, pts))
   {
      // std::cout << "triangle [" << pts[0] << " " << pts[1] << " " << pts[2] << "]" << std::endl;

      double p0[3];
      outputPolyData->GetPoint(pts[0], p0);

      double p1[3];
      outputPolyData->GetPoint(pts[1], p1);

      double p2[3];
      outputPolyData->GetPoint(pts[2], p2);
      
      QPen redOutlinePen(Qt::red);
      redOutlinePen.setWidth(0.1);

      scene->addLine(p0[0], p0[1], p1[0], p1[1], redOutlinePen);
      scene->addLine(p1[0], p1[1], p2[0], p2[1], redOutlinePen);
      scene->addLine(p2[0], p2[1], p0[0], p0[1], redOutlinePen);
   }
// }
*/

}

//////////////////////////////////////////////////////////////////////////

