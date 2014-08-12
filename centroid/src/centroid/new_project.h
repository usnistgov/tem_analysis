#ifndef __H_NEW_PROJECT__
#define __H_NEW_PROJECT__

#include "ui_new_project.h"

#include "project.h"

class NewProjectDialog : public QDialog, public Ui::NewProjectDialog
{
   Q_OBJECT

public:
   NewProjectDialog(QWidget *parent = 0);

   Project* saveProject();

   void setDirectory(QDir dir);
   QDir getDirectory();

private slots:
    void chooseBaseDirectory();

private:
    QDir directory;
};

//////////////////////////////////////////////////////////////////////////

#endif // __H_NEW_PROJECT__
