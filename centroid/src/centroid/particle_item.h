#ifndef __H_PARTICLE_ITEM__
#define __H_PARTICLE_ITEM__

#include <QGraphicsItem>
#include <QPainter>

#include "particles.h"


//////////////////////////////////////////////////////////////////////////

class ParticleItem : public QGraphicsItem
{
public:
   ParticleItem(const QPointF& pos, int id, int frame, Particles *particles, QGraphicsItem *parent = 0);
   QRectF boundingRect() const;
   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

   void setSelected(bool setSelected);

   void set_is_positive_selection_on(bool state) { is_positive_selection_on = state; }

protected:
   void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
   void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);

   QVariant itemChange(GraphicsItemChange change, const QVariant &value);  

private:
   int id;

   QPointF position;
   QPointF start_point;
   QPointF end_point;

   bool is_positive_selection_on;     

   Particles *particles_all;
   int current_frame;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_PARTICLE_ITEM__


