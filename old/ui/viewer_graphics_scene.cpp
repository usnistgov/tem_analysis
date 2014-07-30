#include "viewer_graphics_scene.h"
#include "graphics_items.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QDebug>


//////////////////////////////////////////////////////////////////////////

ViewerGraphicsScene::ViewerGraphicsScene(QObject *parent) 
   : QGraphicsScene(parent)
{

}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
   // qDebug() << views().size();
   QGraphicsView *view = views().at(0);
   // if (views().size() > 0)
   // {
   // }

   if (view->dragMode() == QGraphicsView::RubberBandDrag)
   {
      if (mouseEvent->button() == Qt::LeftButton)
      {
         // qDebug() << "mouse left button press event: " << mouseEvent->scenePos().x() << " " << mouseEvent->scenePos().y();
         start_point = mouseEvent->scenePos();
      }  

      
   }


   QGraphicsScene::mousePressEvent(mouseEvent);
}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
   QGraphicsView *view = views().at(0);
   if (view->dragMode() == QGraphicsView::RubberBandDrag)
   {
      qDebug() << "rect: " << start_point.x() << " " << start_point.y() << " " << mouseEvent->scenePos().x() << " " << mouseEvent->scenePos().y();

      end_point = mouseEvent->scenePos();

      GraphicsParticleItem *gpi = new GraphicsParticleItem(start_point, end_point);
      addItem(gpi);


   }

   QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

   QGraphicsScene::mouseMoveEvent(mouseEvent);
}

//////////////////////////////////////////////////////////////////////////
