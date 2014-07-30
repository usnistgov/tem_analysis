#include "graphics_items.h"


//////////////////////////////////////////////////////////////////////////

GraphicsParticleItem::GraphicsParticleItem(const QPointF& start_p, const QPointF& end_p, QGraphicsItem *parent)
   : QGraphicsItem(parent)
{
   start_point = start_p;
   end_point = end_p;

   setFlag(QGraphicsItem::ItemIsMovable, false);
   setFlag(QGraphicsItem::ItemIsSelectable, true);
   setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
}

//////////////////////////////////////////////////////////////////////////

QRectF GraphicsParticleItem::boundingRect() const
{
   return QRectF(start_point.x(), start_point.y(), end_point.x()-start_point.x(), end_point.y()-start_point.y()); 
}

//////////////////////////////////////////////////////////////////////////

void GraphicsParticleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   painter->drawRect(start_point.x(), start_point.y(), end_point.x()-start_point.x(), end_point.y()-start_point.y());
}

//////////////////////////////////////////////////////////////////////////
