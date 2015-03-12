#include "../Include/AseModel/ase_helperobject.hpp"
#include "../Include/AseModel/asemodel.hpp"

void Ase_HelperObject::setData(QString string)
{
     QStringList line = string.split(" ");

     if (line.contains(ASE_NODE_NAME))
     {
         this->Name.setString(line[1]);
     }
     if (line.contains(ASE_NODE_PARENT))
     {
         this->NodeParent.setString(line[1]);
     }
     if (line.contains(ASE_HELPER_CLASS))
     {
         this->HelperClass.setString(line[1]);
     }
     if (line.contains(ASE_BOUNDINGBOX_MAX))
     {
         this->BoundingBoxMax.X.setF(line[1]);
         this->BoundingBoxMax.Y.setF(line[2]);
         this->BoundingBoxMax.Z.setF(line[3]);
     }
     if (line.contains(ASE_BOUNDINGBOX_MIN))
     {
         this->BoundingBoxMin.X.setF(line[1]);
         this->BoundingBoxMin.Y.setF(line[2]);
         this->BoundingBoxMin.Z.setF(line[3]);
     }

}
