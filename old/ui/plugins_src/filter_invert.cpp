#include <iostream>
#include "filter_invert.h"
#include <QDebug>

// 
// When we define metadata for a plugin, we know upfront the name of
// variables we use. 
// 
// In this particular plugin, we have filter_invert.json file:
//
// {
//    "name" : "filter.invert",
//    "version" : "0.0.1",
//    "type" : "imaging",
//    "parameters" : 
//    [ 
//       {
//          "name" : "input",
//          "type" : "file",
//          "form" : "Input image",
//          "value" : ""
//       },
//       {
//          "name" : "output",
//          "type" : "file",
//          "form" : "Output image",
//          "value" : ""
//       },
//       {
//          "name" : "threshold",
//          "type" : "float",
//          "form" : "Threshold",
//          "value" : "0.5"
//       }
//    ],
//    "dependancies" : []
// }
//
// Description of the filter_invert.json:
//    "name": defines unique name of the module,
//    "version": defines version of the module (can be used for checking
//          dependancies on other modules if we use cross modules),
//    "type": defines type of module (imaging, particles, textual, etc.)
//    "parameters": define all out customised parameters we could later
//          refer to when writing execution code, etc. 
//    "dependancies": describe names and version of the modules that this
//          module depends on.
//
// There could be many parameters use by a module. The structure will have
// though unique fields:
//    "name": defines name of the parameter,
//    "type": defines type of the parameter,
//    "form": defines string that will be displayed when UI is constructed
//            from filter_invert.json metadata,
//    "value": contains predefined value. We can assume safely to keep
//             all values always as QString 
//


void FilterInvert::execute(const QVector<QMap<QString, QVariant> >& parameters) 
{
   std::cout << "\n>>> BLOCK " << getName().toStdString() << std::endl;

   // in metadata we specified three parameters: input, output, threshold
   //
   QStringList metadata_parameters;
   metadata_parameters << "input" << "output" << "threshold" << "entropy" << "pika";

   // let's get them, and store in local variables
   //
   QString input;   
   QString output;
   float threshold;
   float entropy;
   bool pika;
   
   // get the parameters values
   //
   for (int i=0; i<parameters.size(); ++i)
   {
      // qDebug() << parameters[i]["name"].toString();

      switch (metadata_parameters.indexOf(parameters[i]["name"].toString()))
      {
         case 0:
            // get input
            //
            input = parameters[i]["value"].toString();
            break;
         case 1:
            // get output
            //
            output = parameters[i]["value"].toString();
            break;
         case 2:
            // get threshold
            //
            threshold = parameters[i]["value"].toFloat();
            break;
         case 3:
            // get entropy
            //
            entropy = parameters[i]["value"].toFloat();
            break;
         case 4:
            // get pika
            //
            if (parameters[i]["value"].toString() == "1")
            {
               pika = 1;
            } else pika = 0;
            break;
      }
   }

   qDebug() << "EXECUTE PARAMS >>>";  
   qDebug() << input;
   qDebug() << output;
   qDebug() << threshold;
   qDebug() << entropy;
   qDebug() << pika;
   qDebug() << "<<< EXECUTE PARAMS\n";  

   // processing code below
   //








}


