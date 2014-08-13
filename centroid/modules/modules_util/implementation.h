#ifndef __H_IMPLEMENTATION__
#define __H_IMPLEMENTATION__

#include "interface.h"

//////////////////////////////////////////////////////////////////////////

class FilterImplementation : public FilterInterface
{
public:
   virtual ~FilterImplementation() {}

   virtual void setProject(Project* proj);
   virtual Project* getProject();

   virtual void setMetaData(MetaData* data);
   virtual MetaData* getMetaData();

private:
   Project *project;
   MetaData *metaData;
};

inline void FilterImplementation::setProject(Project* proj)
{
    project = proj;
}

inline Project* FilterImplementation::getProject()
{
    return project;
}

inline void FilterImplementation::setMetaData(MetaData* data)
{
    metaData = data;
}

inline MetaData* FilterImplementation::getMetaData()
{
    return metaData;
}

//////////////////////////////////////////////////////////////////////////

#endif // __H_IMPLEMENTATION__
