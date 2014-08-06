#include "particle_item.h"
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


//////////////////////////////////////////////////////////////////////////

ParticleItem::ParticleItem(const QPointF& pos, int idid, int frame, Particles *particles, QGraphicsItem *parent)
   : QGraphicsItem(parent)
{
   position = pos;
   id = idid;

   particles_all = particles;
   current_frame = frame;

    // Note that there is some old code below that enabled direct
    // selection of these ParticleItems and dragging them around.
    // This has all been disabled, but the code is left for
    // reference (sometimes commented out).
    // ignore all mouse button events
    setAcceptedMouseButtons(0);

#if 0
   // all flags are disabled by default

   setFlag(QGraphicsItem::ItemIsMovable, false);
   // setFlag(QGraphicsItem::ItemIsSelectable, true);
   setFlag(QGraphicsItem::ItemIsSelectable, false);
   setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
   setFlag(QGraphicsItem::ItemIsFocusable, false);
#endif

   // if (id == 297) particles_all->set_selected(frame, id, true);
}

//////////////////////////////////////////////////////////////////////////

void ParticleItem::setSelected(bool selected)
{
   qDebug() << "my selected" << id;
   // QGraphicsItem::setSelected(selected);
   particles_all->set_selected(current_frame, id, selected);
}

//////////////////////////////////////////////////////////////////////////

QRectF ParticleItem::boundingRect() const
{
   return QRectF(position.x()-1, position.y()-1, 2, 2);
}

//////////////////////////////////////////////////////////////////////////

void ParticleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   // if (!isSelected())
   if (!particles_all->is_selected(current_frame, id))
   {
      painter->setPen(Qt::NoPen);
      painter->setBrush(Qt::red); 
   }
   else 
   {
      painter->setPen(Qt::NoPen);
      painter->setBrush(Qt::green); 
   }
   painter->drawEllipse(position, 1, 1);
}

//////////////////////////////////////////////////////////////////////////

QVariant ParticleItem::itemChange(GraphicsItemChange change, const QVariant &value)
{

    // JGH: 
    //      The problem here is that if we change the is_selected flag on
    //      a particle as we drag the box over the point, then we have to
    //      change it back if we uncover it during the drag. But we need
    //      to change it back to what it was, not merely to what it isn't.
    //      I'm not sure that we retain the information to do this.
    //
    // So the solution seems to be to do the is_selected change only
    // at the button release, not during the drag.

    return QGraphicsItem::itemChange(change, value);

////////////////////////

    switch(change) 
    {
      case ItemSelectedChange:
         qDebug() << "ItemSelectedChange " << id << " " << isSelected() << " " << value << " " << value.toInt(); 
         if (is_positive_selection_on)
         {
            particles_all->set_selected(current_frame, id, true);
         }
         else
         {
            particles_all->set_selected(current_frame, id, false);           
         }
         // }
         update();
         break;
      default:
         break;
    };

    return QGraphicsItem::itemChange(change, value);
}

//////////////////////////////////////////////////////////////////////////

void ParticleItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

#if 0
   // qDebug() << views().size();
   // QGraphicsView *view = views().at(0);
   // if (views().size() > 0)
   // {
   // }

   // if (view->dragMode() == QGraphicsView::RubberBandDrag)
   {
      if (mouseEvent->button() == Qt::LeftButton)
      {
         qDebug() << "mouse left button press event: " << mouseEvent->scenePos().x() << " " << mouseEvent->scenePos().y() << " " << id;
         start_point = mouseEvent->scenePos();
         setSelected(true);
      }      
   }

//   QGraphicsItem::mousePressEvent(mouseEvent);
#endif

}

//////////////////////////////////////////////////////////////////////////

void ParticleItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

#if 0
   // QGraphicsView *view = views().at(0);
   // if (view->dragMode() == QGraphicsView::RubberBandDrag)
   {
      qDebug() << "rect: " << start_point.x() << " " << start_point.y() << " " << mouseEvent->scenePos().x() << " " << mouseEvent->scenePos().y();
      end_point = mouseEvent->scenePos();
      // GraphicsParticleItem *gpi = new GraphicsParticleItem(start_point, end_point);
      // addItem(gpi);
   }

//   QGraphicsItem::mouseReleaseEvent(mouseEvent);
#endif
}

//////////////////////////////////////////////////////////////////////////

void ParticleItem::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

#if 0
printf ("item mouse event\n");
   QGraphicsItem::mouseMoveEvent(mouseEvent);
#endif
}
