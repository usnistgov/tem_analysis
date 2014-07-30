#ifndef __H_GRAPHICS_ITEMS__
#define __H_GRAPHICS_ITEMS__

#include <QGraphicsItem>
#include <QPainter>

//////////////////////////////////////////////////////////////////////////

class GraphicsParticleItem : public QGraphicsItem
{
public:
   GraphicsParticleItem(const QPointF& start_p, const QPointF& end_p, QGraphicsItem *parent = 0);

   QRectF boundingRect() const;

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
   QPointF start_point;
   QPointF end_point;


};

//////////////////////////////////////////////////////////////////////////

#endif // __H_GRAPHICS_ITEMS__
