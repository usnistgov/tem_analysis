#include "main_window.h"


int main(int argc, char *argv[])  
{
   QApplication app(argc, argv);
   app.setOrganizationName("NIST and CSIRO");
   app.setApplicationName("Centroid");
   
   MainWindow mainWin;
   mainWin.show();

   return app.exec();
}
