#include "module_button.h"


ModuleButton::ModuleButton(const QString& text, QTreeWidget* parent, QTreeWidgetItem* item)
 : QPushButton(text, parent), 
   m_pItem(item)
{
   connect(this, SIGNAL(pressed()), this, SLOT(ButtonPressed()));
}
 

void ModuleButton::ButtonPressed()
{
   m_pItem->setExpanded(!m_pItem->isExpanded());
}
