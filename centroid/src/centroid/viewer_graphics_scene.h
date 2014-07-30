#ifndef __H_VIEWER_GRAPHICS_SCENE__
#define __H_VIEWER_GRAPHICS_SCENE__

#include <QGraphicsScene>
//#include <QPointF>

//////////////////////////////////////////////////////////////////////////

class ViewerGraphicsScene : public QGraphicsScene
{
   Q_OBJECT

public:
   ViewerGraphicsScene(QObject *parent = 0);

public slots:

signals:

protected:
   void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
   void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);

   void keyPressEvent(QKeyEvent *keyEvent);
   void keyReleaseEvent(QKeyEvent *keyEvent);

private:
   QPointF start_point;  
   QPointF end_point; 
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_VIEWER_GRAPHICS_SCENE__
