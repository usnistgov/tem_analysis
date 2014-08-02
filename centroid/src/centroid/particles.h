#ifndef __H_PARTICLES__
#define __H_PARTICLES__

#include <QVector>
#include <QFileInfoList>

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

   bool is_selected;
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
   void draw_triangles(ViewerGraphicsScene *scene, int frame);

   void set_selected(int frame, int id, bool selected);
   void set_selected (int frame, const QPointF& p0, const QPointF& p1,
                                    bool selected);

   void set_selected_global(const QPointF& p0, const QPointF& p1, bool selected);
   bool is_selected(int frame, int id);
   void set_is_positive_selection_on(bool state) { is_positive_selection_on = state; }

   void remove_all_particles_selected();
   void remove_all_particles_in_rect(const QPointF& p0, const QPointF& p1);

private:
   QVector<particles*> list_all_particles; 

   int particle_time_sustained;
   float particle_distance_threshold;  
   float particle_size_threshold;

   bool is_positive_selection_on;   

   // layers
   QFileInfoList *file_list_images_layers;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_PARTICLES__
