#ifndef __H_PARTICLES__
#define __H_PARTICLES__

#include <QVector>
#include <QFileInfoList>

#include "main.h"

class ViewerGraphicsScene;

//////////////////////////////////////////////////////////////////////////

typedef struct 
{

   /////////
   // These are the fields that are present in the apos files

   int tag; // just a tag that is unique within a given frame

   // position
   float x;
   float y;

   // area
   unsigned int n_pixels;

   float maxWeight;
   float avgWeight;


   /////////
   // These are derived fields

   // I'm revising the "deletion / removal" operation so that
   // we don't actually remove particles, we just just flag
   // them as active or inactive. This enables us to use particle
   // indices that remain stable.
   // This could also enable us to restore "deleted" particles.
   // All code that uses particles must honor this flag.  (JGH)
   bool active;  

   bool is_selected;

   bool is_sustained;


   // the following aren't really used yet.....
   // closest particles
   int index_closest_in_next_frame;
   float dist_smallest_next_frame;
   int index_closest_in_previous_frame;
   float dist_smallest_previous_frame;

} particle;


typedef QVector<particle> particles;

//////////////////////////////////////////////////////////////////////////

class Particles
{

public:
   Particles();

   void read_data_from_dir(const QString& str);

   void process_particles();
   void process_particles_sustained();
   void process_particles_doubles();

   int get_number_of_frames();

   void draw_particles(ViewerGraphicsScene *scene, int frame);
   void draw_triangles(ViewerGraphicsScene *scene, int frame, 
                                        bool selectedParticlesOnly);

   void set_selected(int frame, int id, bool selected);
   void set_selected (int frame, const QPointF& p0, const QPointF& p1,
                                    bool selected);

   void set_selected_global(const QPointF& p0, const QPointF& p1, bool selected);
   bool is_selected(int frame, int id);
   void set_is_positive_selection_on (bool state) 
                { is_positive_selection_on = state; }

   void remove_all_particles_selected();

   // void remove_all_particles_in_rect(const QPointF& p0, const QPointF& p1);

   void invert_particle_selection_all ();
   void invert_particle_selection (int frame);

   void deselect_all_particles ();
   void deselect_all_particles (int frame);

   void add_particle (float x, float y, int frame);

   QString get_frame_filename (int frame);

   void writeActiveAtomPosFile (const int frame, const QString& fn);

private:
   QVector<particles*> list_all_particles; 
   QFileInfoList particleFileList;



/*
typedef QPair<float,float> F_Edge;  // coords of edge
typedef QPair<int,int> ID_Edge;     // ids of edge end points 
typedef QPair<F_Edge, ID_Edge> Edge;  // F_Edge for drawing, ID_Edge for output

typedef QVector<Edge> EdgeList
QVector<EdgeList> frameEdgeList;  // one edge list for each pair

JGH: 
I think that we need to store the data for the triangulation
at each frame:
    * Need another QVector here to hold the edge lists; 
        one edge list for each frame ; 

    * maybe the edge list should be a map, which would give
        us a simple way to avoid duplication of edges; the key
        would be pair of indices and what would data be?

    * Or maybe use QSet to avoid dups or maybe QList::contains
        The QList approach seems lighter-weight.
        Need an edge class with == , could be QPair<int, int> if
        we always ensure that the first < second.
*/



   int particle_time_sustained;
   float particle_distance_threshold;  
   float particle_size_threshold;

   bool is_positive_selection_on;   

   // layers
   QFileInfoList *file_list_images_layers;

   int maxParticleTag;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_PARTICLES__
