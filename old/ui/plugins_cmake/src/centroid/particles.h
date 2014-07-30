#ifndef __H_PARTICLES__
#define __H_PARTICLES__

#include <QVector>

class ViewerGraphicsScene;

//////////////////////////////////////////////////////////////////////////

typedef struct 
{
   // unsigned int id; // meaningless
   // position
   float x;
   float y;
   // area
   unsigned int no_pixels;
   // closest particles
   int index_closest_in_next_frame;
   float dist_smallest_next_frame;

   int index_closest_in_previous_frame;
   float dist_smallest_previous_frame;

   bool is_sustained;
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

   void draw_frame(ViewerGraphicsScene *scene, int frame);

private:
   QVector<particles*> list_all_particles; 

   int particle_time_sustained;
   float particle_distance_threshold;  
   float particle_size_threshold;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_PARTICLES__
