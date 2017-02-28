/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include "gridObjectsHelperClasses.h"
#include "gridObjects.h"
#include <cstring>

static const solverOffsets nullOffsets;

solverMode::solverMode (index_t index)
{
  offsetIndex = index;
  if (index == 0)  //predefined local
    {
      local = true;
      dynamic = true;
      differential = true;
      algebraic = true;
    }
  else if ((index == 1)||(index == 3)) //predefined localb and dae
    {
      dynamic = true;
      differential = true;
      algebraic = true;
    }
  else if (index == 2)  //predefined pflow
    {
      algebraic = true;
      differential = false;
      dynamic = false;
    }
  else if (index == 4) //predefined dynAlg
    {
      algebraic = true;
      differential = false;
      dynamic = true;
    }
  else if (index == 5) //predefined dynDiff
    {
      algebraic = false;
      differential = true;
      dynamic = true;
    }
}

void stateSizes::reset ()
{
  std::memset (this, 0, sizeof(stateSizes));
}

void stateSizes::stateReset ()
{
  vSize = aSize = algSize = diffSize = 0;
}

void stateSizes::rootAndJacobianReset ()
{
  algRoots = diffRoots = jacSize = 0;
}

void stateSizes::add (const stateSizes &arg)
{
  vSize += arg.vSize;
  aSize += arg.aSize;
  algSize += arg.algSize;
  diffSize += arg.diffSize;
  algRoots += arg.algRoots;
  diffRoots += arg.diffRoots;
  jacSize += arg.jacSize;
}

void stateSizes::addStateSizes (const stateSizes &arg)
{
  vSize += arg.vSize;
  aSize += arg.aSize;
  algSize += arg.algSize;
  diffSize += arg.diffSize;
}

void stateSizes::addRootAndJacobianSizes (const stateSizes &arg)
{
  algRoots += arg.algRoots;
  diffRoots += arg.diffRoots;
  jacSize += arg.jacSize;
}


count_t stateSizes::totalSize() const
{
	return vSize + aSize + algSize + diffSize;

}
solverOffsets::solverOffsets (const solverOffsets *no)
{
  std::memcpy (this, no, sizeof(solverOffsets));
}
solverOffsets::solverOffsets (const solverOffsets &nOffsets)
{
  std::memcpy (this, &nOffsets, sizeof(solverOffsets));
}

void solverOffsets::reset ()
{

  diffOffset = aOffset = vOffset = algOffset = rootOffset = kNullLocation;
  local.reset ();
  total.reset ();

  rjLoaded = stateLoaded = offetLoaded = false;
}

void solverOffsets::stateReset ()
{

  local.stateReset ();
  total.stateReset ();
  diffOffset = aOffset = vOffset = algOffset = kNullLocation;
  stateLoaded = false;
}

void solverOffsets::rootAndJacobianCountReset ()
{
  rootOffset = kNullLocation;
  local.rootAndJacobianReset ();
  total.rootAndJacobianReset ();

  rjLoaded = false;
}




void solverOffsets::increment ()
{
  count_t algExtra = 0;
  if (aOffset != kNullLocation)
    {
      aOffset += total.aSize;
    }
  else
    {
      algExtra = total.aSize;
    }
  if (vOffset != kNullLocation)
    {
      vOffset += total.vSize;
    }
  else
    {
      algExtra += total.vSize;
    }


  algOffset += total.algSize + algExtra;

  if (diffOffset != kNullLocation)
    {
      diffOffset += total.diffSize;
    }
  else
    {
      algOffset += total.diffSize;
    }
  if (rootOffset != kNullLocation)
    {
      rootOffset += total.algRoots + total.diffRoots;
    }

}

void solverOffsets::increment (const solverOffsets *offsets)
{
  count_t algExtra = 0;
  if (aOffset != kNullLocation)
    {
      aOffset += offsets->total.aSize;
    }
  else
    {
      algExtra = offsets->total.aSize;
    }
  if (vOffset != kNullLocation)
    {
      vOffset += offsets->total.vSize;
    }
  else
    {
      algExtra += offsets->total.vSize;
    }


  algOffset += offsets->total.algSize + algExtra;

  if (diffOffset != kNullLocation)
    {
      diffOffset += offsets->total.diffSize;
    }
  else
    {
      algOffset += offsets->total.diffSize;
    }
  if (rootOffset != kNullLocation)
    {
      rootOffset += offsets->total.algRoots + offsets->total.diffRoots;
    }

}

void solverOffsets::localIncrement (const solverOffsets *offsets)
{
  count_t algExtra = 0;
  if (aOffset != kNullLocation)
    {
      aOffset += offsets->local.aSize;
    }
  else
    {
      algExtra = offsets->local.aSize;
    }
  if (vOffset != kNullLocation)
    {
      vOffset += offsets->local.vSize;
    }
  else
    {
      algExtra += offsets->local.vSize;
    }


  algOffset += offsets->local.algSize + algExtra;

  if (diffOffset != kNullLocation)
    {
      diffOffset += offsets->local.diffSize;
    }
  else
    {
      algOffset += offsets->local.diffSize;
    }
  if (rootOffset != kNullLocation)
    {
      rootOffset += offsets->local.algRoots + local.diffRoots;
    }

}

void  solverOffsets::addSizes (const solverOffsets *offsets)
{
  total.add (offsets->total);
}

void  solverOffsets::addStateSizes (const solverOffsets *offsets)
{
  total.addStateSizes (offsets->total);
}

void  solverOffsets::addRootAndJacobianSizes (const solverOffsets *offsets)
{
  total.addRootAndJacobianSizes (offsets->total);
}

void  solverOffsets::localLoad (bool finishedLoading)
{
  total = local;
  stateLoaded = finishedLoading;
  rjLoaded = finishedLoading;
}

void solverOffsets::setOffsets (const solverOffsets &newOffsets)
{

  algOffset = newOffsets.algOffset;
  diffOffset = newOffsets.diffOffset;

  if (total.aSize > 0)
    {
      if (newOffsets.aOffset != kNullLocation)
        {
          aOffset = newOffsets.aOffset;
        }
      else
        {
          aOffset = algOffset;
          algOffset += total.aSize;
        }
    }
  else
    {
      aOffset = kNullLocation;
    }

  if (total.vSize > 0)
    {
      if (newOffsets.vOffset != kNullLocation)
        {
          vOffset = newOffsets.vOffset;
        }
      else
        {
          vOffset = algOffset;
          algOffset += total.vSize;
        }
    }
  else
    {
      vOffset = kNullLocation;
    }


  if (diffOffset == kNullLocation)
    {
      diffOffset = algOffset + total.algSize;
    }
}

void solverOffsets::setOffset (index_t newOffset)
{
  aOffset = newOffset;
  vOffset = aOffset + total.aSize;
  algOffset = vOffset + total.vSize;
  diffOffset = algOffset + total.algSize;
  if (total.aSize == 0)
    {
      aOffset = kNullLocation;
    }
  if (total.vSize == 0)
    {
      vOffset = kNullLocation;
    }
}




offsetTable::offsetTable () : offsetContainer (6),cSize (6)
{
  //most simulations use the first 2 and powerflow(3) and likely dynamic DAE(4)  and often 5 and 6 for dynamic partitioned
  offsetContainer[0].sMode = cLocalSolverMode;
  offsetContainer[1].sMode = cLocalbSolverMode;
}

offsetTable::offsetTable (const offsetTable &oTable) : offsetContainer (oTable.offsetContainer), cSize (oTable.cSize)
{

}

offsetTable &offsetTable::operator= (const offsetTable &oTable)
{
  offsetContainer = oTable.offsetContainer;
  cSize = oTable.cSize;
  return *this;
}

bool offsetTable::isLoaded (const solverMode &sMode) const
{
  return ((sMode.offsetIndex >= cSize) ? false : ((offsetContainer[sMode.offsetIndex].stateLoaded)&&(offsetContainer[sMode.offsetIndex].rjLoaded)));

}

bool offsetTable::isStateLoaded (const solverMode &sMode) const
{
  return ((sMode.offsetIndex >= cSize) ? false : offsetContainer[sMode.offsetIndex].stateLoaded);

}

bool offsetTable::isrjLoaded (const solverMode &sMode) const
{
  return ((sMode.offsetIndex >= cSize) ? false : offsetContainer[sMode.offsetIndex].rjLoaded);

}

solverOffsets * offsetTable::getOffsets (const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);
      offsetContainer[sMode.offsetIndex].sMode = sMode;
      cSize = sMode.offsetIndex + 1;
    }
  return &(offsetContainer[sMode.offsetIndex]);
}

const solverOffsets * offsetTable::getOffsets (const solverMode &sMode) const
{
  if (sMode.offsetIndex >= cSize)
    {
      return &nullOffsets;
    }
  return &(offsetContainer[sMode.offsetIndex]);
}

void offsetTable::setOffsets (const solverOffsets &newOffsets, const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);
      cSize = sMode.offsetIndex + 1;
    }
  offsetContainer[sMode.offsetIndex].sMode = sMode;
  offsetContainer[sMode.offsetIndex].setOffsets (newOffsets);
}


void offsetTable::setOffset (index_t newOffset, const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);
      cSize = sMode.offsetIndex + 1;
    }
  offsetContainer[sMode.offsetIndex].sMode = sMode;
  offsetContainer[sMode.offsetIndex].setOffset (newOffset);
}


void offsetTable::setAlgOffset (index_t newOffset, const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);
      cSize = sMode.offsetIndex + 1;
    }
  offsetContainer[sMode.offsetIndex].sMode = sMode;
  offsetContainer[sMode.offsetIndex].algOffset = newOffset;
}

void offsetTable::setDiffOffset (index_t newOffset, const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);

      cSize = sMode.offsetIndex + 1;
    }
  offsetContainer[sMode.offsetIndex].sMode = sMode;
  offsetContainer[sMode.offsetIndex].diffOffset = newOffset;
}

void offsetTable::setVOffset (index_t newOffset, const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);

      cSize = sMode.offsetIndex + 1;
    }
  offsetContainer[sMode.offsetIndex].sMode = sMode;
  offsetContainer[sMode.offsetIndex].vOffset = newOffset;
}

void offsetTable::setAOffset (index_t newOffset, const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);
      cSize = sMode.offsetIndex + 1;
    }
  offsetContainer[sMode.offsetIndex].sMode = sMode;
  offsetContainer[sMode.offsetIndex].aOffset = newOffset;
}

void offsetTable::setRootOffset (index_t newOffset, const solverMode &sMode)
{
  if (sMode.offsetIndex >= cSize)
    {
      offsetContainer.resize (sMode.offsetIndex + 1);

      cSize = sMode.offsetIndex + 1;
    }
  offsetContainer[sMode.offsetIndex].sMode = sMode;
  offsetContainer[sMode.offsetIndex].rootOffset = newOffset;

}

index_t offsetTable::maxIndex (const solverMode &sMode) const
{
  if (sMode.offsetIndex >= cSize)
    {
      return 0;
    }
  auto so = offsetContainer[sMode.offsetIndex];
  index_t mx;
  if (isDynamic (sMode))
    {
      mx = so.diffOffset + so.total.diffSize;
      if (so.algOffset + so.total.algSize > mx)
        {
          mx = so.algOffset + so.total.algSize;
        }
    }
  else
    {
      mx = so.algOffset + so.total.algSize;
    }
  if ((so.vOffset != kNullLocation)&&(so.vOffset + so.total.vSize > mx))
    {
      mx = so.vOffset + so.total.vSize;
    }
  if ((so.aOffset != kNullLocation) && (so.aOffset + so.total.aSize > mx))
    {
      mx = so.aOffset + so.total.aSize;
    }
  return mx;
}

void offsetTable::getLocations (const solverMode &sMode, Lp *Loc) const
{

  Loc->algOffset = offsetContainer[sMode.offsetIndex].algOffset;
  Loc->diffOffset = offsetContainer[sMode.offsetIndex].diffOffset;
}

void offsetTable::unload (bool dynamic_only)
{
  if (dynamic_only)
    {
      for (auto &so : offsetContainer)
        {
          if (isDynamic (so.sMode))
            {
              so.stateLoaded = false;
              so.rjLoaded = false;
              so.diffOffset = kNullLocation;
              so.algOffset = kNullLocation;
            }
        }
    }
  else
    {
      for (auto &so:offsetContainer)
        {
          so.stateLoaded = false;
          so.rjLoaded = false;
          so.diffOffset = kNullLocation;
          so.algOffset = kNullLocation;
        }
    }
}

void offsetTable::stateUnload (bool dynamic_only)
{
  if (dynamic_only)
    {
      for (auto &so : offsetContainer)
        {
          if (isDynamic (so.sMode))
            {
              so.stateLoaded = false;
              so.diffOffset = kNullLocation;
              so.algOffset = kNullLocation;
            }
        }
    }
  else
    {
      for (auto &so : offsetContainer)
        {
          so.stateLoaded = false;
          so.diffOffset = kNullLocation;
          so.algOffset = kNullLocation;
        }
    }
}

void offsetTable::rjUnload (bool dynamic_only)
{
  if (dynamic_only)
    {
      for (auto &so : offsetContainer)
        {
          if (isDynamic (so.sMode))
            {
              so.rjLoaded = false;
            }
        }
    }
  else
    {
      for (auto &so : offsetContainer)
        {
          so.rjLoaded = false;
        }
    }
}


void offsetTable::localUpdateAll (bool dynOnly)
{
  if (dynOnly)
    {
      for (auto &so : offsetContainer)
        {
          if (isDynamic (so.sMode))
            {
			  auto &lc = local();
              so.total.algRoots = so.local.algRoots = lc.local.algRoots;
              so.total.diffRoots = so.local.diffRoots = lc.local.diffRoots;
              so.total.jacSize = so.local.jacSize = lc.local.jacSize;
              so.rjLoaded = true;
            }
        }
    }
  else
    {
      for (auto &so : offsetContainer)
        {
          so.local = local().local;
          so.localLoad (true);
        }
    }
}
const solverMode &offsetTable::getSolverMode (index_t index) const
{
  if (index < cSize)
    {
      return offsetContainer[index].sMode;
    }
  else
    {
      return cEmptySolverMode;
    }
}

const solverMode &offsetTable::find (const solverMode &tMode) const
{
  for (auto &so:offsetContainer)
    {

      if (so.sMode.dynamic != tMode.dynamic)
        {
          continue;
        }
      if (so.sMode.local != tMode.local)
        {
          continue;
        }
      if (so.sMode.algebraic != tMode.algebraic)
        {
          continue;
        }
      if (so.sMode.differential != tMode.differential)
        {
          continue;
        }

      if (so.sMode.extended_state != tMode.extended_state)
        {
          continue;
        }
      if (so.sMode.approx != tMode.approx)
        {
          continue;
        }
      return so.sMode;
    }
  return cEmptySolverMode;
}
