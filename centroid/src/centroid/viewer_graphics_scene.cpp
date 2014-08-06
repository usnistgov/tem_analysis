#include "viewer_graphics_scene.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsView>
#include <QDebug>

#include "main_viewer.h"

//////////////////////////////////////////////////////////////////////////

ViewerGraphicsScene::ViewerGraphicsScene(QObject *parent) 
   : QGraphicsScene(parent)
{

}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
   QGraphicsView *view = views().at(0);

   if (view->dragMode() == QGraphicsView::RubberBandDrag)
   {
      if (mouseEvent->button() == Qt::LeftButton)
      {
         start_point = mouseEvent->scenePos();
      }  
   }

   QGraphicsScene::mousePressEvent(mouseEvent);
}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
   // Here's where all the action is for interactive mouse manipulations
   // inside the image/atom viewer.

   QGraphicsView *view = views().at(0);
   if (view->dragMode() == QGraphicsView::RubberBandDrag)
   {
      end_point = mouseEvent->scenePos();

      MainViewerForm *myParent= qobject_cast<MainViewerForm *>(parent()); 
      if (myParent->get_is_selection_global())
      {
         myParent->select_all_particles_global(start_point, end_point, myParent->get_is_positive_selection_on());
         myParent->draw_current_frame();      
      }
      else
      {
         // do the corresponding thing from current frame
         myParent->select_all_particles_current_frame (start_point, 
                        end_point, myParent->get_is_positive_selection_on());
         myParent->draw_current_frame();      

      }
   }

   QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

   QGraphicsScene::mouseMoveEvent(mouseEvent);
}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::keyPressEvent(QKeyEvent *keyEvent)
{
   if (keyEvent->key() == Qt::Key_Delete)
   {
      MainViewerForm *myParent= qobject_cast<MainViewerForm *>(parent()); 
      myParent->remove_all_particles_selected();
      myParent->draw_current_frame();
   }
}

//////////////////////////////////////////////////////////////////////////

void ViewerGraphicsScene::keyReleaseEvent(QKeyEvent *keyEvent)
{
   // qDebug() << "ViewerGraphicsScene::keyReleaseEvent";
}

//////////////////////////////////////////////////////////////////////////


