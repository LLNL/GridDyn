/*
* LLNS Copyright Start
* Copyright (c) 2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#include "VirtualMatrix.h"

std::ostream& operator<<(std::ostream& output, const VirtualMatrix& mat)
{
  mat.dump(output);
  return output;
}
void VirtualMatrix::dump(std::string filename) const
{
  std::ofstream file;
  file.open(filename.c_str());
  file << std::setprecision(20);
  this->dump(file);
  file.close();
}

