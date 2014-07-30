#include "main_window.h"


int main(int argc, char *argv[])  
{
   QApplication app(argc, argv);
   app.setOrganizationName("NIST");
   app.setOrganizationDomain("nist.gov");
   app.setApplicationName("TEM Analysis");
   
   MainWindow mainWin;
   mainWin.show();

   return app.exec();
}
