#include "new_project.h"

#include <QFileDialog>

//////////////////////////////////////////////////////////////////////////

NewProjectDialog::NewProjectDialog(QWidget *parent) 
   : QDialog(parent)
{
   setupUi(this);
   connect(baseDirectoryPushButton, SIGNAL(clicked()), this, SLOT(chooseBaseDirectory()));
    setWindowTitle("New Project");
}

Project* NewProjectDialog::saveProject()
{
    Project* project = new Project;
    project->setShortTag(shortTagLineEdit->text());

    QFileInfo projectFileName(baseDirectoryLineEdit->text(),
                              shortTagLineEdit->text() + ".proj");
    if (!project->save(projectFileName.absoluteFilePath())) {
        qCritical() << "Cannot save project to" << projectFileName.absoluteFilePath();
        return NULL;
    }

    return project;
}

void NewProjectDialog::setDirectory(QDir dir)
{
    directory = dir;
}

QDir NewProjectDialog::getDirectory()
{
    return directory;
}

void NewProjectDialog::chooseBaseDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this,
        "Select project base directory", directory.path(), 0);
    if (dir.length() == 0) return;
    baseDirectoryLineEdit->setText(dir);
}

