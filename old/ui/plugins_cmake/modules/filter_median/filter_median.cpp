#include <iostream>
#include <QDebug>
#include "filter_median.h"

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"

// VTK
#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkParticleReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVertexGlyphFilter.h>


void FilterInvert::execute(const QVector<QMap<QString, QVariant> >& parameters) 
{
   std::cout << "\n>>> BLOCK " << getName().toStdString() << std::endl;

   writeLog("BLOCK ");
   writeLog(getName());
   writeLog(" executed.");

   // in metadata we specified three parameters: input, output, threshold
   //
   QStringList metadata_parameters;
   metadata_parameters << "input" << "output" << "threshold";

   // let's get them, and store in local variables
   //
   QString input;   
   QString output;
   float threshold;
   
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
      }
   }

   qDebug() << "EXECUTE PARAMS >>>";  
   qDebug() << input;
   qDebug() << output;
   qDebug() << threshold;
   qDebug() << "<<< EXECUTE PARAMS\n";  

   // processing code below
   //

   typedef itk::RGBPixel< unsigned char >   PixelType;
   typedef itk::Image< PixelType, 2 > ImageType;
    typedef itk::ImageFileReader< ImageType >  ReaderType;
    typedef itk::ImageFileWriter< ImageType >  WriterType;
    ReaderType::Pointer reader = ReaderType::New();
    WriterType::Pointer writer = WriterType::New();




}


