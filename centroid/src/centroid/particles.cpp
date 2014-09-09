#include "particles.h"
#include <QDir>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QtGui>
#include <iostream>
#include <math.h>

#include "viewer_graphics_scene.h"
#include "particle_item.h"

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

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))


//////////////////////////////////////////////////////////////////////////

Particles::Particles()
   : particle_time_sustained(5),
     particle_distance_threshold(0.75f),
     particle_size_threshold(50.0f),
     maxParticleTag (-1000000)
{
   file_list_images_layers = new QFileInfoList;

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

   QStringList filters_layer;
   filters_layer << "*.layer";
   dir.setNameFilters(filters_layer);   
   file_list_images_layers->clear();
   file_list_images_layers->append(dir.entryInfoList());

   QStringList filters;
   filters << "*.apos";
   dir.setNameFilters(filters);   
   QFileInfoList list = dir.entryInfoList();

   particleFileList.clear();
   particleFileList.append (list);

   maxParticleTag = -1000000;

   // std::cout << "Number of files: " << list.size() << std::endl;

   if (list.size() == 0) return;

   // One entry in list for each frame; each entry is a list of particles
   list_all_particles.resize(list.size());

   // if (list.size()>0)
   // {
   //    timeScrollBar->setRange(0, list.size()-1);
   // }

   // std::cout << "Reading particle data:" << std::endl;
   writeLog ("\nReading atom position files  ");
   qDebug() << "Reading particle data:" ;

   for (int i=0; i<list.size(); ++i) 
   {
      writeLog (".");

      QFileInfo fileInfo = list.at(i);

      list_all_particles[i] = new particles;

      QFile file(fileInfo.absoluteFilePath());
      if (!file.open(QIODevice::ReadOnly)) 
      {
         qDebug() << file.errorString() << fileInfo.absoluteFilePath() << endl;
      }

      QTextStream in(&file);

      // loop over lines in apos file
      while (!in.atEnd()) 
      {

/*
JGH: ask Wes about this.  Note that p is a local variable
to this while loop, but it is being pushed onthe the particles list
*/

         // for each line, create a particle and add it to the 
         // particle list at the current entry in list_all_particles
         QString line = in.readLine();    
         QStringList fields = line.split(QRegExp("\\s+")); 

#if 0
         // note that field 0 is always empty
        std::cout << "line: " << line.toStdString() << std::endl;
        std::cout << "fields: ";
        std::cout << " 0 |" << fields[0].toStdString() << "|    ";
        std::cout << " 1 |" << fields[1].toStdString() << "|    ";
        std::cout << " 2 |" << fields[2].toStdString() << "|    ";
        std::cout << " 3 |" << fields[3].toStdString() << "|    ";
        std::cout << " 4 |" << fields[4].toStdString() << "|    ";
        std::cout << " 5 |" << fields[5].toStdString() << "|    ";
        std::cout << " 6 |" << fields[6].toStdString() << "|    ";
        std::cout << std::endl << std::flush;
#endif

         particle p;
         p.active = true;
         p.tag = fields[1].toInt();
         p.n_pixels = fields[2].toInt();
         p.x = fields[3].toFloat();
         p.y = fields[4].toFloat();
         p.maxWeight = fields[5].toFloat();
         p.avgWeight = fields[6].toFloat();
         p.index_closest_in_next_frame = -1;
         p.index_closest_in_previous_frame = -1;      
         p.dist_smallest_next_frame = 1000000.0f;
         p.dist_smallest_previous_frame = 1000000.0f;
         p.is_sustained = false;
         p.is_selected = false;
         list_all_particles[i]->push_back(p);
         qDebug() << fields;


         maxParticleTag = MAX (maxParticleTag, p.tag);

      }  // end of loop over lines in input particle file   

   }  // end of loop over input particle files   

   maxParticleTag++;  // skip one tag just for the heck of it

   writeLog ("  Done reading atom position files.\n");

   // if (list_all_particles.size() > 0) isDataRead = true;
}

//////////////////////////////////////////////////////////////////////////
void Particles::writeActiveAtomPosFile (const int frame, const QString& outFN)
{

   // check if frame is in range
    if (frame < list_all_particles.size())
    {

        // open output file

        QFile outFile (outFN);
        if ( ! outFile.open (QIODevice::WriteOnly) )
        {
            writeLog ("ERROR: Cannot open file " + outFN + " for writing.\n");
            return;
        }

        QTextStream outStream(&outFile);
        // outStream.setFieldAlignment (QTextStream::AlignRight);
        // outStream.setFieldWidth (12);
        outStream.setRealNumberNotation (QTextStream::SmartNotation);
        outStream.setRealNumberPrecision (8);

        particles *ps = list_all_particles[frame];

        // loop through each particle in this frame
        for (int i=0; i<ps->size(); i++)
        {  
         
            if (ps->at(i).active)
            {
#if 0
                // outStream.setFieldWidth (4);
                outStream   
                            << " "
                            << ps->at(i).tag
                            << "   "
                            << ps->at(i).n_pixels
                            << " ";
                    
                // outStream.setFieldWidth (12);
                outStream   
                            << "          "
                            << ps->at(i).x
                            << "   "
                            << ps->at(i).y
                            << "          "
                            << ps->at(i).maxWeight
                            << "   "
                            << ps->at(i).avgWeight;
                // outStream.setFieldWidth (1);
                outStream   << "\n";
#else
                QString line;
                outStream << 
                    line.sprintf (
                        "%5d    %5d       %11.5f  %11.5f      %.8g  %.8g\n",
                        ps->at(i).tag, ps->at(i).n_pixels,
                        ps->at(i).x, ps->at(i).y, 
                        ps->at(i).maxWeight, ps->at(i).avgWeight);

#endif

            }
        }  // end of loop over particles

        outFile.close ();

    }  // end of if frame num is valid

} // end void Particles::writeActiveAtomPosFile (const int f, const QString& fn)
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
         if (ps0->at(i).active)
         {
            float x0 = ps0->at(i).x; 
            float y0 = ps0->at(i).y;
            float dist0 = ps0->at(i).dist_smallest_next_frame;
            int j0 = -1;

            for (j=0; j<ps1->size(); j++)
            {
                if (ps1->at(j).active)
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
                }  // end of if active
            }  // end of inner loop over particles

            particle *p = ps0[0].data();
            p[i].dist_smallest_next_frame = dist0;
            p[i].index_closest_in_next_frame = j0;
         }  // end of if particle is active
      }  // end of outer loop over particles

      // 
      //
      // finds particle index and closest distance to particles (from previous frame)

      m = k+1;

      ps0 = list_all_particles[m];
      ps1 = list_all_particles[m-1];

      for (i=0; i<ps0->size(); i++)
      {
        if (ps0->at(i).active)
        {

            float x0 = ps0->at(i).x; 
            float y0 = ps0->at(i).y;
            float dist0 = ps0->at(i).dist_smallest_previous_frame;
            int j0 = -1;

            for (j=0; j<ps1->size(); j++)
            {
                if (ps1->at(j).active)
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

                }  // end of if active


            }  // end of inner loop over partices

            particle *p = ps0[0].data();
            p[i].dist_smallest_previous_frame = dist0;
            p[i].index_closest_in_previous_frame = j0; 
        }  // end of if active
      }  // end of outer loop over partices

      //
      //
   }  // end of loop over frames   

}  // end of void Particles::process_particles()

//////////////////////////////////////////////////////////////////////////

// JGH: really need to think about this......

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

         if (ps0->at(i).active && 
             (ps0->at(i).dist_smallest_next_frame<particle_distance_threshold) )
         {
            int index_next = ps0->at(i).index_closest_in_next_frame;
            isSustained = 1;
            for (j=0; j<particle_time_sustained-1; j++)
            {
               particles *ps1 = list_all_particles[k+j+1];
               if ( ps1->at(index_next).active && 
                    (ps1->at(index_next).dist_smallest_next_frame < 
                                            particle_distance_threshold) )
               {
                  isSustained++;
                  int ii = index_next;
                  index_next = ps1->at(ii).index_closest_in_next_frame;
               }
               else
               {
                    break;
               }
            }  // end of loop over subsequent frames

         }  // end of if dist threshold test

         if (isSustained == particle_time_sustained) 
         {
            particle *p = ps0[0].data();
            p[i].is_sustained = true;
         }

      }  // end of outer loop over particles

    }  // end of loop over frames

  }  // end of test on # of frames
}

//////////////////////////////////////////////////////////////////////////

// JGH: need to reevaluation this in terms of active flag
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
      }
   }
}


//////////////////////////////////////////////////////////////////////////

int Particles::get_number_of_frames()
{
   return list_all_particles.size();
}

//////////////////////////////////////////////////////////////////////////

void Particles::draw_particles(ViewerGraphicsScene *scene, int frame)
{
   if (get_number_of_frames() == 0) return;

   // draw particles in this frame
   // 
   // check if frame is in range
   if (frame < list_all_particles.size())
   {
      particles *ps = list_all_particles[frame];

       // loop through each particle in this frame
      for (int i=0; i<ps->size(); i++)
      {
         
         if (ps->at(i).active)
         {
            // create a ParticleItem; this is a QGraphicsItem subclass
            ParticleItem *pi = 
            new ParticleItem(QPointF(ps->at(i).x, ps->at(i).y), i, frame, this);

            pi->set_is_positive_selection_on(is_positive_selection_on);

            // add the graphics item to the scene
            scene->addItem(pi);
         }
      }
   }
}

//////////////////////////////////////////////////////////////////////////

void Particles::draw_triangles(ViewerGraphicsScene *scene, int frame, bool selectedParticlesOnly )
{

   // Use VTK to derive triangles for the current frame

   if (get_number_of_frames() == 0) return;

   particles *ps0 = list_all_particles[frame];

   // Create a grid of points (height/terrian map)
   //
   vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

   // for each particle, add a point to the vtkPoints object
   double xx, yy, zz;
   for (int i=0; i<ps0->size(); i++)
   {
      if (ps0->at(i).active)
      {
        if ( ( ! selectedParticlesOnly ) || (ps0->at(i).is_selected) )
        {
            xx = ps0->at(i).x;
            yy = ps0->at(i).y;
            zz = vtkMath::Random(-.5, .5);  // ??? to avoid dups?
            points->InsertNextPoint(xx, yy, zz);
        }
      }
   }

   // Add the grid points to a polydata object
   vtkSmartPointer<vtkPolyData> inputPolyData = 
                    vtkSmartPointer<vtkPolyData>::New();

   inputPolyData->SetPoints(points);
 
   // Triangulate the grid points
   vtkSmartPointer<vtkDelaunay2D> delaunay = 
                    vtkSmartPointer<vtkDelaunay2D>::New();
   
   #if VTK_MAJOR_VERSION <= 5
      delaunay->SetInput(inputPolyData);
   #else
      delaunay->SetInputData(inputPolyData);
   #endif
   delaunay->Update();

   // the output is the triangulation
   vtkPolyData* outputPolyData = delaunay->GetOutput();

   // std::cout << "Number of polys: " << outputPolyData->GetNumberOfPolys() << std::endl;

   vtkIdType npts, *pts;
   outputPolyData->GetPolys()->InitTraversal();
   
   // Loop through each cell (triangle) in the triangulation and draw edges
   // (Note that this scheme draws almost all edges twice because
   //  almost all triangle edges are shared by two triangles.....)
   while (outputPolyData->GetPolys()->GetNextCell(npts, pts))
   {
      // std::cout << "triangle [" << pts[0] << " " << pts[1] << " " << pts[2] << "]" << std::endl;

      // get the three vertices of the triangle
      double p0[3];
      outputPolyData->GetPoint(pts[0], p0);

      double p1[3];
      outputPolyData->GetPoint(pts[1], p1);

      double p2[3];
      outputPolyData->GetPoint(pts[2], p2);
      
      // add a line segment for each edge of the triangle

      QPen redOutlinePen(Qt::darkRed);
      redOutlinePen.setWidth(0.1);

      scene->addLine(p0[0], p0[1], p1[0], p1[1], redOutlinePen);
      scene->addLine(p1[0], p1[1], p2[0], p2[1], redOutlinePen);
      scene->addLine(p2[0], p2[1], p0[0], p0[1], redOutlinePen);
   }

}

//////////////////////////////////////////////////////////////////////////

void Particles::set_selected(int frame, int id, bool selected)
{
   particles *ps = list_all_particles[frame];
   particle *p = ps[0].data();
   // do we need to worry about active flag here?
   p[id].is_selected = selected;
}




void Particles::set_selected (int frame, 
                              const QPointF& pp0, const QPointF& pp1, 
                              bool selected)
{


   // Set the is_selected flag for all particles in the indicated
   // frame that are inside the box specified by pp0 and pp1.

   QPointF p0 (MIN(pp0.x(), pp1.x()), MIN (pp0.y(), pp1.y()));
   QPointF p1 (MAX(pp0.x(), pp1.x()), MAX (pp0.y(), pp1.y()));

   particles *ps = list_all_particles[frame];
   QMutableVectorIterator<particle> i(*ps);
   while (i.hasNext())
      {
         float x = i.peekNext().x;
         float y = i.peekNext().y;
         particle *p = &i.next();
         if ((x > p0.x()) && (x<p1.x()))
         {
            if ((y > p0.y()) && (y<p1.y()))
            {
               // do we need to worry about active flag here?
               p->is_selected = selected;               
            }
         }
      }

}  // end of set_selected







//////////////////////////////////////////////////////////////////////////

// JGH: I believe that these selection methods are now obselete, so I'm
//      not going to worry about the active flag unless we begin to 
//      use these methods again.

void Particles::set_selected_global(const QPointF& pp0, const QPointF& pp1, bool selected)
{

   // Set the is_selected flag for all particles in all
   // frames that are inside the box specified by pp0 and pp1.

   QPointF p0 (MIN(pp0.x(), pp1.x()), MIN (pp0.y(), pp1.y()));
   QPointF p1 (MAX(pp0.x(), pp1.x()), MAX (pp0.y(), pp1.y()));

   int j, k;

   for (k=0; k<list_all_particles.size(); k++)
   {
      particles *ps0 = list_all_particles[k];
      QMutableVectorIterator<particle> i(*ps0);
      while (i.hasNext())
      {
         float x = i.peekNext().x;
         float y = i.peekNext().y;
         particle *p = &i.next();
         if ((x > p0.x()) && (x<p1.x()))
         {
            if ((y > p0.y()) && (y<p1.y()))
            {
               p->is_selected = selected;               
            }
         }
      }
   }   
}


//////////////////////////////////////////////////////////////////////////

bool Particles::is_selected(int frame, int id)
{
   particles *ps = list_all_particles[frame];
   return ps->at(id).is_selected;
}

//////////////////////////////////////////////////////////////////////////
//
// Remove all particles selected from all frames
//

void Particles::remove_all_particles_selected()
{

#if 0
   int j, k;

   for (k=0; k<list_all_particles.size(); k++)
   {
      particles *ps0 = list_all_particles[k];

      QMutableVectorIterator<particle> i(*ps0);
      while (i.hasNext())
      {
         if (i.next().is_selected)
         {
            // i.remove();
            
         }
      }
   }
#else

   // here's where we're setting a flag rather than actually removing a particle

   int i, k;

   for (k=0; k<list_all_particles.size(); k++)
   {
        particles *ps0 = list_all_particles[k];

        for (i=0; i<ps0->size(); i++)
        {
            if ((*ps0)[i].is_selected)
            {
                (*ps0)[i].active = false;
            }
        }
   }

#endif




}


//////////////////////////////////////////////////////////////////////////
//
// deselect atoms
//

void Particles::deselect_all_particles ()
{
   int i, k;

   for (k=0; k<list_all_particles.size(); k++)
   {
        particles *ps0 = list_all_particles[k];

        for (i=0; i<ps0->size(); i++)
        {
            // should we worry about active flag?
            (*ps0)[i].is_selected = false;
        }
   }
}

void Particles::deselect_all_particles (int frame)
{
   int i;

    particles *ps0 = list_all_particles[frame];

    for (i=0; i<ps0->size(); i++)
    {
         // should we worry about active flag?
        (*ps0)[i].is_selected = false;
    }
}






//////////////////////////////////////////////////////////////////////////
//
// Invert particle selection across all frames
//

void Particles::invert_particle_selection_all ()
{
   int i, k;

   for (k=0; k<list_all_particles.size(); k++)
   {
        particles *ps0 = list_all_particles[k];

        for (i=0; i<ps0->size(); i++)
        {
            // should we worry about active flag?
            (*ps0)[i].is_selected = !  (*ps0)[i].is_selected;
        }
   }
}



void Particles::invert_particle_selection (int frame)
{
    int i;

    particles *ps0 = list_all_particles[frame];

    for (i=0; i<ps0->size(); i++)
    {
        // should we worry about active flag?
        (*ps0)[i].is_selected = !  (*ps0)[i].is_selected;
    }
}






#if 0

JGH: I don t think that this is used

//////////////////////////////////////////////////////////////////////////
//
// Remove all particles selected from selected region in all frames
//

void Particles::remove_all_particles_in_rect(const QPointF& pp0, const QPointF& pp1)
{

   // JGH: I believe that this method is not used.

   QPointF p0 (MIN(pp0.x(), pp1.x()), MIN (pp0.y(), pp1.y()));
   QPointF p1 (MAX(pp0.x(), pp1.x()), MAX (pp0.y(), pp1.y()));

   int j, k;

   for (k=0; k<list_all_particles.size(); k++)
   {
      particles *ps0 = list_all_particles[k];

      QMutableVectorIterator<particle> i(*ps0);
      while (i.hasNext())
      {
         float x = i.peekNext().x;
         float y = i.peekNext().y;
         if (i.next().is_selected)
         {
            if ((x > p0.x()) && (x<p1.x()))
            {
               if ((y > p0.y()) && (y<p1.y()))
               {
                  i.remove();
               }
            }
         }
      }
   }
}
#endif

//////////////////////////////////////////////////////////////////////////



/*
      QBrush blackBrush(QColor(0, 0, 0, 127)); //Qt::black,);
      QBrush redBrush(Qt::red);
      QBrush greenBrush(Qt::green);
      QPen blackOutlinePen(QColor(0, 0, 0, 0));
      QPen redOutlinePen(Qt::red);
      QPen greenOutlinePen(Qt::green);
      blackOutlinePen.setWidth(1);
      redOutlinePen.setWidth(1);
      greenOutlinePen.setWidth(1);
*/
      // std::cout << "Number of particles " << ps->size() << " in frame " << frame << std::endl;
      // int i;


      // very messy drawing prototype
      //
/*
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
               if (ps->at(i).n_pixels < particle_size_threshold)
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
               if (ps->at(i).n_pixels < particle_size_threshold)
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
*/
      // current_frame = frame;
//   }

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

}
*/

//////////////////////////////////////////////////////////////////////////

void Particles::add_particle (float x, float y, int frame)
{

   if (get_number_of_frames() == 0) return;

    if (frame == -1) // means do all frames
    {
        for (int k=0; k<list_all_particles.size(); k++)
        {
            // recurse
            add_particle (x, y, k);
        }

    }
    else if (frame < list_all_particles.size())
    {
         particle p;
         p.active = true;
         p.x = x;
         p.y = y;
         p.n_pixels = -1;
         p.tag = ++maxParticleTag;
         p.maxWeight = -1;
         p.avgWeight = -1;
         p.index_closest_in_next_frame = -1;
         p.index_closest_in_previous_frame = -1;      
         p.dist_smallest_next_frame = 1000000.0f;
         p.dist_smallest_previous_frame = 1000000.0f;
         p.is_sustained = false;
         p.is_selected = false;
         list_all_particles[frame]->push_back(p);
    }

}  // end of add_particle 


QString Particles::get_frame_filename (int frame)
{
    if ( (frame >= 0) && frame < list_all_particles.size() )
        {
        return particleFileList.at(frame).absoluteFilePath();
        }

    return "";
}

