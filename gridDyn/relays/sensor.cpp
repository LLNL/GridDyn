/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
/*
  * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include "sensor.h"
#include "timeSeries.h"
#include "gridEvent.h"
#include "submodels/gridControlBlocks.h"
#include "gridGrabbers.h"
#include "stateGrabber.h"
#include "gridCondition.h"
#include "linkModels/gridLink.h"  //some special features for links
#include "comms/gridCommunicator.h"
#include "comms/controlMessage.h"
#include "gridCoreTemplates.h"
#include "matrixDataSparse.h"
#include "core/gridDynExceptions.h"
#include "stringConversion.h"

#include <boost/format.hpp>

using namespace gridUnits;

sensor::sensor (const std::string&objName) : gridRelay (objName)
{
  opFlags.set (continuous_flag);
  opFlags.set (late_b_initialize);
  opFlags.reset (no_dyn_states);
}

gridCoreObject *sensor::clone (gridCoreObject *obj) const
{
  sensor *nobj = cloneBase<sensor, gridRelay> (this, obj);
  if (!(nobj))
    {
      return obj;
    }

  nobj->m_terminal = m_terminal;  //!< line terminal
  nobj->outputSize = outputSize;
  nobj->inputStrings = inputStrings;
  nobj->outputNames = outputNames;
  //clone the filter blocks
  for (index_t kk = 0; kk < filterBlocks.size(); ++kk)
  {
      if (nobj->filterBlocks.size() > kk)
      {
          nobj->add(filterBlocks[kk]->clone());
      }
      else if (!nobj->filterBlocks[kk])
      {
          auto fbclone = filterBlocks[kk]->clone();
          fbclone->locIndex = kk;
          nobj->add(fbclone);
      }
      else
      {
          filterBlocks[kk]->clone(nobj->filterBlocks[kk].get());
      }
  }
 //clone the dataSources
  
  for (index_t kk = 0; kk < dataSources.size(); ++kk)
  {
      if (dataSources[kk])
      {
          if (nobj->dataSources.size() > kk)
          {
              nobj->dataSources[kk] = dataSources[kk]->clone(nobj->dataSources[kk]);
          }
          else
          {
              nobj->dataSources.push_back(dataSources[kk]->clone());
          }
      }
      else
      {
          if (nobj->dataSources.size() <= kk)
          {
              nobj->dataSources.push_back(nullptr);
          }
      }
     
  }
  for (index_t kk = 0; kk < dataSourcesSt.size(); ++kk)
  {
      if (dataSourcesSt[kk])
      {
          if (nobj->dataSourcesSt.size() > kk)
          {
              nobj->dataSourcesSt[kk] = dataSourcesSt[kk]->clone(nobj->dataSourcesSt[kk]);
          }
          else
          {
              nobj->dataSourcesSt.push_back(dataSourcesSt[kk]->clone());
          }
      }
      else
      {
          if (nobj->dataSourcesSt.size() <= kk)
          {
              nobj->dataSourcesSt.push_back(nullptr);
          }
      }

  }

  for (index_t kk = 0; kk < outGrabber.size(); ++kk)
  {
      if (outGrabber[kk])
      {
          if (nobj->outGrabber.size() > kk)
          {
              nobj->outGrabber[kk] = outGrabber[kk]->clone(nobj->outGrabber[kk]);
              nobj->outGrabber[kk]->updateObject(nobj);
          }
          else
          {
              nobj->outGrabber.push_back(outGrabber[kk]->clone());
              nobj->outGrabber[kk]->updateObject(nobj);
          }
      }
      else
      {
          if (nobj->outGrabber.size() <= kk)
          {
              nobj->outGrabber.push_back(nullptr);
          }
      }

  }
  for (index_t kk = 0; kk < outGrabberSt.size(); ++kk)
  {
      if (outGrabberSt[kk])
      {
          if (nobj->outGrabberSt.size() > kk)
          {
              nobj->outGrabberSt[kk] = outGrabberSt[kk]->clone(nobj->outGrabberSt[kk]);
              nobj->outGrabberSt[kk]->updateObject(nobj);
          }
          else
          {
              nobj->outGrabberSt.push_back(outGrabberSt[kk]->clone());
              nobj->outGrabberSt[kk]->updateObject(nobj);
          }
      }
      else
      {
          if (nobj->outGrabberSt.size() <= kk)
          {
              nobj->outGrabberSt.push_back(nullptr);
          }
      }

  }
 
  //now clone the output Grabbers
  return nobj;
}

void sensor::add (gridCoreObject *obj)
{
  if (dynamic_cast<basicBlock *> (obj))
    {
        add (static_cast<basicBlock *> (obj));
    }
  else
    {
        gridRelay::add (obj);
    }

}

void sensor::add (basicBlock *blk)
{
  blk->setParent (this);
  if (blk->locIndex != kNullLocation)
    {
      if (blk->locIndex >= static_cast<index_t> (filterBlocks.size ()))
        {
          filterBlocks.resize (blk->locIndex + 1);
        }
      filterBlocks[blk->locIndex] = (std::shared_ptr<basicBlock> (blk));
    }
  else
    {
      filterBlocks.push_back (std::shared_ptr<basicBlock> (blk));
    }
}

void sensor::add (std::shared_ptr<basicBlock> blk)
{
  blk->setParent (this);
  if (blk->locIndex != kNullLocation)
    {
      if (blk->locIndex >= static_cast<index_t> (filterBlocks.size ()))
        {
          filterBlocks.resize (blk->locIndex + 1);
        }
      filterBlocks[blk->locIndex] = blk;
    }
  else
    {
      filterBlocks.push_back (blk);
    }
}

void sensor::add (std::shared_ptr<gridGrabber> dGr, std::shared_ptr<stateGrabber> dGrst)
{
  auto cnum = inputStrings.size ();
  dataSources.resize (cnum + 1);
  dataSourcesSt.resize (cnum + 1);
  inputStrings.push_back ("#");
  dataSources[cnum] = dGr;
  dataSourcesSt[cnum] = dGrst;
  if (!dGrst)
    {
      opFlags.set (continuous_flag, false);
    }
}

void sensor::addsp (std::shared_ptr<gridCoreObject> obj)
{
  if (std::dynamic_pointer_cast<basicBlock> (obj))
    {
      add (std::dynamic_pointer_cast<basicBlock> (obj));
    }
  else
    {
	  return throw(invalidObjectException(this));
    }
}

void sensor::setFlag (const std::string &flag, bool val)
{

  if ((flag == "direct_io")||(flag == "direct"))
    {
      opFlags.set (direct_IO, val);
    }
  else
    {
      gridRelay::setFlag (flag, val);
    }

}

void sensor::set (const std::string &param,  const std::string &val)
{
  std::string iparam;
  int num = trailingStringInt (param,iparam);
  if (iparam == "input")
    {
      auto sp = splitlineTrim (val);
      if (num >= 0)
        {
          if (num + sp.size () - 1 >= inputStrings.size ())
            {
              inputStrings.resize (num + sp.size ());
            }
          for (auto &istr : sp)
            {
              inputStrings[num] = istr + '_';
              ++num;
            }

        }
      else
        {

          inputStrings.reserve (inputStrings.size () + sp.size ());
          for (auto &istr : sp)
            {
              inputStrings.push_back (istr);
            }

        }
      if (opFlags[direct_IO])
        {
          if (num >= 0)
            {
              if (num + sp.size () - 1 >= outputNames.size ())
                {
                  outputNames.resize (num + sp.size ());
                }
              for (auto &istr : sp)
                {
                  outputNames[num] = getTailString (istr,':');
                  ++num;
                }

            }
          else
            {

              outputNames.reserve (outputNames.size () + sp.size ());
              for (auto &istr : sp)
                {
                  outputNames.push_back (getTailString (istr,':'));
                }

            }
        }
    }
  else if (param == "condition")
    {
      add (make_condition (val, this));
    }
  else if (iparam == "filter")
    {
      auto blk = make_block (val);
      if (blk)
        {
          if (num >= 0)
            {
              blk->locIndex = num;
            }
          add (blk);
        }
      else
        {
		  throw(invalidParameterValue());
        }

    }
  else if ((iparam == "outputname")||(param == "outputnames"))
    {
      if (num >= 0)
        {
          if (num >= static_cast<int> (outputs.size ()))
            {
              outputNames.resize (num + 1);
            }
          outputNames[num] = val;
        }
      else
        {
          auto sep = splitlineTrim (val);
          if (outputNames.empty ())
            {
              outputNames = sep;
            }
          else
            {
              for (auto &nm : sep)
                {
                  outputNames.push_back (nm);
                }
            }
        }
    }
  else if ((iparam == "output") || (param == "outputs"))
    {
      auto el = val.find_first_of ('=');
      std::string v2;
      std::string sval;
      if (el != std::string::npos)
        {
          sval = val.substr (0, el);
          v2 = val.substr (el + 1);
        }
      else
        {
          sval.clear ();
          v2 = val;
        }
      if (num >= 0)
        {
          if (num >= static_cast<int> (outputs.size ()))
            {
              outputs.resize (num + 1, -1);
              outputNames.resize (num + 1);

              outputMode.resize (num + 1,outputMode_t::block);
              outGrabber.resize (num + 1);
              outGrabberSt.resize (num + 1);
            }
          if (!sval.empty ())
            {
              outputNames[num] = sval;
            }
          auto outputValue = intReadComplete (v2,-1);
          if (outputValue >= 0)
            {
              outputs[num] = outputValue;
              outputMode[num] = outputMode_t::block;
            }
          else
            {
              int knum = trailingStringInt (v2, sval, 0);
              if (sval == "input")
                {
                  outputs[num] = knum;
                  outputMode[num] = outputMode_t::direct;
                }
              else if (sval == "block")
                {
                  outputs[num] = knum;
                  outputMode[num] = outputMode_t::block;
                }
              else
                {
                  auto gv = makeGrabbers (v2, this);
                  outGrabber[num] = gv[0];
                  if (opFlags[continuous_flag])
                    {
                      auto gvs = makeStateGrabbers (v2, this);
                      outGrabberSt[num] = gvs[0];
                    }
                  else
                    {
                      outGrabberSt[num] = nullptr;
                    }
                  outputMode[num] = outputMode_t::processed;
                }

            }

        }
      else
        {
          auto sep = splitlineTrim (val);
          auto ns = sep.size ();
          outputs.resize (ns, -1);
          outputMode.resize (ns, outputMode_t::block);
          outGrabber.resize (ns);
          outGrabberSt.resize (ns);
          int pp = 0;
          for (auto &v:sep)
            {
              auto outNum = intRead (v,-1);
              if (outNum >= 0)
                {
                  outputs[pp] = outNum;
                  outputMode[pp] = outputMode_t::block;
                }
              else
                {
                  int knum = trailingStringInt (v, sval, 0);
                  if (sval == "input")
                    {
                      outputs[pp] = knum;
                      outputMode[pp] = outputMode_t::direct;
                    }
                  else if (sval == "block")
                    {
                      outputs[pp] = knum;
                      outputMode[pp] = outputMode_t::block;
                    }
                  else
                    {
                      auto gv = makeGrabbers (v, this);
                      outGrabber[pp] = gv[0];
                      if (opFlags[continuous_flag])
                        {
                          auto gvs = makeStateGrabbers (v, this);
                          outGrabberSt[pp] = gvs[0];
                        }
                      else
                        {
                          outGrabberSt[pp] = nullptr;
                        }

                      outputMode[pp] = outputMode_t::processed;
                    }
                }
              ++pp;
            }
        }
    }
  else if (iparam == "process")
    {
      auto seq = str2vector<int>(val,-1,",: ");
      if (num >= 0)
        {
          if (num >= static_cast<int> (processSequence.size ()))
            {
              processSequence.resize (num + 1);
            }
          processSequence[num] = seq;
        }
      else
        {
          processSequence.push_back (seq);
        }
    }
  else
    {
      gridRelay::set (param, val);
    }

}

void sensor::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  std::string iparam;
  int num = trailingStringInt (param, iparam,-1);
  if (param == "terminal")
    {
      m_terminal = static_cast<int> (val);
    }
  else if (param == "direct")
    {
      opFlags.set (direct_IO, (val > 0));
    }
  else if (iparam == "output")
    {
      if (static_cast<int> (val) < 0)
        {
		  throw(invalidParameterValue());
        }
      else
        {
          if (num < 0)
            {
              outputs.push_back (static_cast<int> (val));
            }
          else
            {
              if (num < static_cast<int> (outputs.size ()))
                {
                  outputs[num] = static_cast<int> (val);
                }
            }
        }
    }
  else
    {
      gridRelay::set (param, val, unitType);
    }
 
}


double sensor::get (const std::string & param, gridUnits::units_t unitType) const
{
  std::string iparam;
  int num = trailingStringInt (param, iparam, -1) - 1;
  double ret = kNullVal;
  if (iparam == "output")
    {
      if (num < 0)
        {
          num = 0;
        }
      if (num < static_cast<int> (outputs.size ()))
        {
          switch (outputMode[num])
            {
            case outputMode_t::block:
              if (outputs[num] >= 0)
                {
                  ret = filterBlocks[outputs[num]]->getOutput ();
                }
              break;
            case outputMode_t::processed:
              if (outGrabber[num])
                {
                  ret = outGrabber[num]->grabData ();
                }
              break;
            case outputMode_t::direct:
              if (outputs[num])
                {
                  ret = dataSources[outputs[num]]->grabData ();
                }
              break;
            default:
                ret = kNullVal;
                break;
            }
        }

      return ret;
    }
  else
    {
      for (size_t kk = 0; kk < outputNames.size (); ++kk)
        {
          if (param == outputNames[kk])
            {
              if (kk < outputMode.size ())
                {
                  switch (outputMode[kk])
                    {
                    case outputMode_t::block:
                      if (outputs[kk] >= 0)
                        {
                          ret = filterBlocks[outputs[kk]]->getOutput ();
                        }
                      break;
                    case outputMode_t::processed:
                      if (outGrabber[kk])
                        {
                          ret = outGrabber[kk]->grabData ();
                        }
                      break;
                    case outputMode_t::direct:
                      if (outputs[kk] >= 0)
                        {
                          ret = dataSources[outputs[kk]]->grabData ();
                        }
                      break;
                    default:
                        ret = kNullVal;
                        break;
                    }
                }
              return ret;
            }
        }
      if (param == "terminal")
        {
          ret = static_cast<double> (m_terminal);
        }
      else
        {
          ret = gridRelay::get (param, unitType);
        }
    }

  return ret;
}

void sensor::updateObject(gridCoreObject *obj, object_update_mode mode)
{
    for (auto &ds : dataSources)
    {
        if (ds)
        {
            ds->updateObject(obj, mode);
        }
        
    }
    for (auto &dsst : dataSourcesSt)
    {
        if (dsst)
        {
            dsst->updateObject(obj, mode);
        }
    }
    gridRelay::updateObject(obj, mode);
}

void sensor::getObjects(std::vector<gridCoreObject *> &objects) const
{
    for (auto &ds : dataSources)
    {
        if (ds)
        {
            ds->getObjects(objects);
        }
        
    }
    for (auto &dsst : dataSourcesSt)
    {
        if (dsst)
        {
            dsst->getObjects(objects);
        }

    }
    gridRelay::getObjects(objects);
}


void sensor::generateInputGrabbers ()
{
    auto iSize = inputStrings.size();
  dataSourcesSt.resize (iSize);
  dataSources.resize (iSize);
  int dct = 0;
  for (size_t ii = 0; ii < iSize; ++ii)
    {
      if (inputStrings[ii][0] == '#')            //escape hatch for previously loaded grabbers
        {
          dct = static_cast<int> (ii + 1);
          continue;
        }
      auto istr = inputStrings[ii];
      int num;
      if (istr.back () == '_')
        {
          istr.pop_back ();
          num = static_cast<int> (ii);
        }
      else
        {
          num = -1;
        }
      auto cloc = istr.find_first_of (':');
      gridCoreObject *target_obj = (m_sourceObject) ? m_sourceObject : parent;
      if (cloc == std::string::npos)
        {  //if there is a colon assume the input is fully specified
          if ((opFlags[link_type_source]) && (!isdigit (istr.back ())))
            {
              if (m_terminal > 0)
                {
                  istr = istr + std::to_string (m_terminal);
                }
            }

        }
      std::vector<std::shared_ptr<stateGrabber>> msg;
      if (opFlags[continuous_flag])
        {
          msg = makeStateGrabbers (istr, target_obj);
        }
      auto mg = makeGrabbers (istr, target_obj);
      if (mg.empty ())
        {
          LOG_WARNING (istr + " is an incorrectly specified input ");
          continue;
        }
      if (num >= 0)
        {
          dct = num;
        }

      if (dct >= static_cast<int> (dataSources.size ()))
        {
          dataSourcesSt.resize (dct + mg.size ());
          dataSources.resize (dct + mg.size ());
        }
      if (num >= 0)
        {
          if (!(msg.empty ()))
            {
              dataSourcesSt[dct] = msg[0];
              dataSources[dct] = mg[0];

            }
          else
            {
              dataSourcesSt[dct] = nullptr;
              dataSources[dct] = mg[0];
              opFlags.set (continuous_flag, false);
            }
          ++dct;
        }
      else
        {
          if (msg.size () != mg.size ())
            {
              for (size_t kk = 0; kk < mg.size (); ++kk)
                {
                  dataSourcesSt[dct] = nullptr;
                  dataSources[dct] = mg[kk];
                  opFlags.set (continuous_flag, false);
                  ++dct;
                }

            }
          else
            {
              for (size_t kk = 0; kk < mg.size (); ++kk)
                {
                  dataSourcesSt[dct] = msg[kk];
                  dataSources[dct] = mg[kk];
                  ++dct;
                }
            }
        }
    }
}


void sensor::receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> msg)
{
  std::shared_ptr<controlMessage> m = std::dynamic_pointer_cast<controlMessage> (msg);

  std::shared_ptr<controlMessage> reply;
  double val;

  switch (m->getMessageType ())
    {
    case controlMessage::SET:
      //only local set
		try
		{
			set(convertToLowerCase(m->m_field), m->m_value, gridUnits::getUnits(m->m_units));
			if (!opFlags[no_message_reply])           //unless told not to respond return with the
			{
				auto gres = std::make_shared<controlMessage>(controlMessage::SET_SUCCESS);
				gres->m_actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
				commLink->transmit(sourceID, gres);
			}
		}
		catch (const gridDynException &)
		{
			if (!opFlags[no_message_reply])           //unless told not to respond return with the
			{
				auto gres = std::make_shared<controlMessage>(controlMessage::SET_FAIL);
				gres->m_actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
				commLink->transmit(sourceID, gres);
			}
		}
      
		break;
    case controlMessage::GET:
      val = get (convertToLowerCase (m->m_field), gridUnits::getUnits (m->m_units));
      reply = std::make_shared<controlMessage> (controlMessage::GET_RESULT);
      reply->m_field = m->m_field;
      reply->m_value = val;
      reply->m_time = prevTime;
      commLink->transmit (sourceID, reply);

      break;
    case controlMessage::SET_SUCCESS:
    case controlMessage::SET_FAIL:
    case controlMessage::GET_RESULT:
    case controlMessage::SET_SCHEDULED:
    case controlMessage::GET_SCHEDULED:
    case controlMessage::CANCEL_FAIL:
    case controlMessage::CANCEL_SUCCESS:
    case controlMessage::GET_RESULT_MULTIPLE:
      break;
    case controlMessage::CANCEL:

      break;
    case controlMessage::GET_MULTIPLE:
      reply = std::make_shared<controlMessage> (controlMessage::GET_RESULT_MULTIPLE);
      reply->multiValues.resize (0);
      reply->multiFields = m->multiFields;
      for (auto fname : m->multiFields)
        {
          val = get (convertToLowerCase (fname), gridUnits::getUnits (m->m_units));
          m->multiValues.push_back (val);
        }
      reply->m_time = prevTime;
      commLink->transmit (sourceID, reply);
      break;
    case controlMessage::GET_PERIODIC:
      break;
    default:
        break;
    }

}


const static IOdata kNullVec;

double sensor::getBlockOutput (const stateData *sD, const solverMode &sMode, index_t block) const
{
  double ret = kNullVal;
  if (isLocal (sMode))
    {
      if (block < filterBlocks.size ())
        {
          ret = filterBlocks[block]->getOutput ();
        }

    }
  else
    {
      if (block < filterBlocks.size ())
        {
          ret = filterBlocks[block]->getOutput (kNullVec,sD,sMode);
        }
    }
  return ret;
}

double sensor::getInput (const stateData *sD, const solverMode &sMode, index_t inputNumber) const
{
  double ret = kNullVal;
  if (isLocal (sMode))
    {
      if (inputNumber < dataSources.size ())
        {
          ret = dataSources[inputNumber]->grabData ();
        }

    }
  else
    {
      if (inputNumber < dataSources.size ())
        {
          ret = dataSourcesSt[inputNumber]->grabData (sD,sMode);
        }
    }
  return ret;
}

void sensor::dynObjectInitializeA (gridDyn_time time0, unsigned long flags)
{


  for (auto fb:filterBlocks)
    {
      if (fb)
        {
          fb->initializeA (time0,flags);
        }
    }
  if (dynamic_cast<gridLink *> (m_sourceObject))
    {
      opFlags.set (link_type_source);
    }

  if (dynamic_cast<gridLink *> (m_sinkObject))
    {
      opFlags.set (link_type_sink);
    }
  generateInputGrabbers ();
  //check if we need to go to sampled mode
  if (opFlags[continuous_flag])
    {
      for (auto &dgs : dataSourcesSt)
        {
          if ((!(dgs)) && (dgs->loaded == false))
            {
              opFlags.set (continuous_flag, false);
              LOG_WARNING ("not all data sources support continuous operation , reverting to sampled mode");
              break;
            }
        }
    }


  return gridRelay::dynObjectInitializeA (time0, flags);
}


void sensor::dynObjectInitializeB (IOdata &outputSet)
{

  if (processSequence.empty ())
    {
      if (filterBlocks.empty ())           //no process, no filter blocks, use direct output
        {
          opFlags.set (direct_IO);
        }
      else
        {
          processSequence.resize (1);
          processSequence[0].resize (1);
          processSequence[0][0] = 0;               //the first one is the source
          //then add the available blocks in order
          for (size_t kk = 0; kk < filterBlocks.size (); ++kk)
            {
              if (filterBlocks[kk])
                {
                  processSequence[0].push_back (static_cast<int> (kk));
                }
            }
          processStatus.resize (1, sequenceMode_t::normal);
        }
    }
  if (outputs.empty ())
    {
      if (opFlags[direct_IO])
        {
          for (size_t kk = 0; kk < dataSources.size (); ++kk)
            {
              outputs.push_back (static_cast<int> (kk));
              outputMode.push_back (outputMode_t::direct);
              outGrabber.push_back (nullptr);
              outGrabberSt.push_back (nullptr);
            }
        }
      else
        {
          outputs.push_back (processSequence[0].back ());
          outputMode.push_back (outputMode_t::block);
          outGrabber.push_back (nullptr);
          outGrabberSt.push_back (nullptr);
        }

    }

  for (auto &ps : processSequence)
    {
      double cv = dataSources[ps[0]]->grabData ();
      //make sure we the process can be handled in states

      IOdata out (1);
      IOdata in {
        cv
      };
      IOdata outset;
      for (size_t psb = 1; psb < ps.size (); ++psb)
        {
          for (auto fb : filterBlocks)
            {
              filterBlocks[ps[psb]]->initializeB (in, outset, out);
              in[0] = out[0];
            }
        }
    }

  return gridRelay::dynObjectInitializeB (outputSet);
}


void sensor::loadSizes (const solverMode &sMode, bool dynOnly)
{
  gridRelay::loadSizes (sMode,dynOnly);
  auto so = offsets.getOffsets (sMode);
  if ((isDynamic (sMode))&&(opFlags[continuous_flag]))
    {
      for (auto &fb : filterBlocks)
        {
          if (!(fb->isLoaded (sMode, dynOnly)))
            {
              fb->loadSizes (sMode, dynOnly);
            }
          if (dynOnly)
            {
              so->addRootAndJacobianSizes (fb->getOffsets (sMode));
            }
          else
            {
              so->addSizes (fb->getOffsets (sMode));
            }

        }
    }
  so->stateLoaded = true;
  so->rjLoaded = true;
}

void sensor::updateFlags (bool /*dynOnly*/)
{
  opFlags &= (~flagMask); //clear the flags
  if (offsets.local->local.algRoots > 0)
    {
      opFlags[has_alg_roots] = true;
      opFlags[has_roots] = true;
    }
  for (auto &fb:filterBlocks)
    {
      if (fb)
        {
          opFlags |= fb->cascadingFlags ();
        }
    }
}

void sensor::setOffsets (const solverOffsets &newOffsets, const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      offsets.setOffsets (newOffsets, sMode);
      solverOffsets no (newOffsets);
      no.localIncrement (offsets.getOffsets (sMode));
      for (auto &so : filterBlocks)
        {
          so->setOffsets (no, sMode);
          no.increment (so->getOffsets (sMode));
        }
    }
}

void sensor::setOffset (index_t offset, const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      for (auto &so : filterBlocks)
        {
          so->setOffset (offset, sMode);
          offset += so->stateSize (sMode);
        }

      offsets.setOffset (offset, sMode);
    }
}


void sensor::setRootOffset (index_t Roffset, const solverMode &sMode)
{
  offsets.setRootOffset (Roffset, sMode);
  if (rootSize (sMode) > 0)
    {

      auto so = offsets.getOffsets (sMode);
      auto nR = so->local.algRoots + so->local.diffRoots;
      for (auto &fb : filterBlocks)
        {
          if (fb->checkFlag (has_roots))
            {
              fb->setRootOffset (Roffset + nR, sMode);
              nR += fb->rootSize (sMode);
            }
        }
    }
}

void sensor::updateA (gridDyn_time time)
{
  if (time + kSmallTime >= m_nextSampleTime)
    {
      for (auto &ps : processSequence)
        {
          double inputFB = dataSources[ps[0]]->grabData ();
          //make sure the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              inputFB = filterBlocks[ps[psb]]->step (time,inputFB);
            }
        }

    }
  gridRelay::updateA (time);
}



void sensor::timestep (gridDyn_time ttime, const solverMode &sMode)
{
  for (auto &ps : processSequence)
    {
      double inputFB = dataSources[ps[0]]->grabData ();
      //make sure we the process can be handled in states

      for (size_t psb = 1; psb < ps.size (); ++psb)
        {
          inputFB = filterBlocks[ps[psb]]->step (ttime, inputFB);
        }
    }
  gridRelay::timestep (ttime,sMode);
}

void sensor::jacobianElements (const stateData *sD, matrixData<double> &ad, const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      matrixDataSparse<double> d2,dp;
      index_t currentLoc = 0;
      for (auto &ps : processSequence)
        {
          d2.clear ();
          dp.clear ();
          double out = dataSourcesSt[ps[0]]->grabData (sD, sMode);
          if (dataSourcesSt[ps[0]]->jacCapable)
            {
              dataSourcesSt[ps[0]]->outputPartialDerivatives (sD, d2, sMode);
            }
          //make sure the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              filterBlocks[ps[psb]]->jacElements (out, 0,sD, dp, currentLoc, sMode);
              out = filterBlocks[ps[psb]]->getBlockOutput ( sD, sMode);
            }
          if (d2.size () == 0)
            {
              dp.translateCol (0, kNullLocation);
              dp.filter ();
            }
          else
            {
              dp.cascade (d2, 0);
            }
          ad.merge (dp);
        }
    }
}

void sensor::setState (gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      for (auto &fb:filterBlocks)
        {
          if (fb)
            {
              fb->setState (ttime,state,dstate_dt,sMode);
            }
        }
      prevTime = ttime;     //only update the time if the model is continuous
    }

}

void sensor::residual (const stateData *sD, double resid[], const solverMode &sMode)
{

  if (stateSize (sMode) > 0)
    {
      for (auto &ps : processSequence)
        {
          double out = dataSourcesSt[ps[0]]->grabData (sD, sMode);
          //make sure we the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              filterBlocks[ps[psb]]->residElements (out,0,sD, resid, sMode);
              out = filterBlocks[ps[psb]]->getBlockOutput (sD, sMode);
            }
        }

    }
}

void sensor::algebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double /*alpha*/)
{
  if (algSize (sMode) > 0)
    {
      for (auto &ps : processSequence)
        {
          double out = dataSourcesSt[ps[0]]->grabData (sD, sMode);
          //make sure we the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              filterBlocks[ps[psb]]->algElements (out, sD, update, sMode);
              out = filterBlocks[ps[psb]]->getBlockOutput ( sD, sMode);
            }
        }

    }
}

void sensor::derivative (const stateData *sD, double deriv[], const solverMode &sMode)
{
  if (diffSize (sMode) > 0)
    {
      for (auto &ps : processSequence)
        {
          double out = dataSourcesSt[ps[0]]->grabData (sD, sMode);
          //make sure we the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              filterBlocks[ps[psb]]->derivElements (out,0, sD, deriv, sMode);
              out = filterBlocks[ps[psb]]->getBlockOutput (sD, sMode);
            }
        }

    }
}

void sensor::guess (gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (stateSize (sMode) > 0)
    {
      for (auto &fb : filterBlocks)
        {
          if (fb)
            {
              fb->guess (ttime, state, dstate_dt, sMode);
            }
        }
    }
}

void sensor::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  if (stateSize (sMode) > 0)
    {
      for (auto &fb : filterBlocks)
        {
          if (fb)
            {
              fb->getStateName (stNames, sMode,prefix + name + "::");
            }
        }
    }
}

index_t sensor::lookupOutput (const std::string &outName)
{
  for (index_t kk = 0; kk < outputNames.size (); ++kk)
    {
      if (outName == outputNames[kk])
        {
          return kk;
        }
    }
  return kNullLocation;
}

IOdata sensor::getOutputs ( const stateData *sD, const solverMode &sMode)
{
  IOdata out (outputs.size ());
  for (size_t pp = 0; pp < outputs.size (); ++pp)
    {
      switch (outputMode[pp])
        {
        case outputMode_t::block:
          out[pp] = filterBlocks[outputs[pp]]->getOutput (kNullVec,sD,sMode);
          break;
        case outputMode_t::processed:
          if (sD)
            {
              out[pp] = outGrabberSt[pp]->grabData (sD, sMode);
            }
          else
            {
              out[pp] = outGrabber[pp]->grabData ();
            }
          break;
        case outputMode_t::direct:
          if (sD)
            {
              out[pp] = dataSources[outputs[pp]]->grabData ();
            }
          else
            {
              out[pp] = dataSourcesSt[outputs[pp]]->grabData (sD,sMode);
            }
          break;
        default:
            out[pp] = kNullVal;
            break;
        }
    }
  return out;
}

double sensor::getOutput (const stateData *sD, const solverMode &sMode, index_t num) const
{
  double out = kNullVal;
  if (num >= outputMode.size ())
    {
      return out;
    }
  switch (outputMode[num])
    {
    case outputMode_t::block:
      out = filterBlocks[outputs[num]]->getOutput (kNullVec, sD, sMode);
      break;
    case outputMode_t::processed:
      if (sD)
        {
          out = outGrabberSt[num]->grabData (sD, sMode);
        }
      else
        {
          out = outGrabber[num]->grabData ();
        }
      break;
    case outputMode_t::direct:
      if (sD)
        {
          out = dataSourcesSt[outputs[num]]->grabData (sD,sMode);
        }
      else
        {
          out = dataSources[outputs[num]]->grabData ();
        }
      break;
    default:
        break;
    }
  return out;
}

index_t sensor::getOutputLoc (const solverMode &sMode, index_t num) const
{

  if (num >= outputMode.size ())
    {
      return kNullLocation;
    }
  switch (outputMode[num])
    {
    case outputMode_t::block:
      return filterBlocks[outputs[num]]->getOutputLoc (sMode);
    case outputMode_t::processed:
    case outputMode_t::direct:
    default:
        return kNullLocation;
    
    }
}

//TODO:: PT This is a complicated function still need to work on it
void sensor::outputPartialDerivatives (const stateData * /*sD*/, matrixData<double> & /*ad*/, const solverMode & /*sMode*/)
{

}

void sensor::rootTest (const stateData *sD, double roots[], const solverMode &sMode)
{
  gridRelay::rootTest (sD,roots,sMode);
  if (stateSize (sMode) > 0)
    {
      IOdata args (1);
      for (auto &ps : processSequence)
        {
          args[0] = dataSourcesSt[ps[0]]->grabData (sD,sMode);
          //make sure we the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              filterBlocks[ps[psb]]->rootTest (args, sD, roots, sMode);
              args[0] = filterBlocks[ps[psb]]->getOutput (args, sD, sMode);
            }
        }
    }
}

void sensor::rootTrigger (gridDyn_time ttime, const std::vector<int> &rootMask, const solverMode &sMode)
{
  gridRelay::rootTrigger (ttime,rootMask,sMode);
  if (stateSize (sMode) > 0)
    {
      IOdata args (1);
      for (auto &ps : processSequence)
        {
          args[0] = dataSources[ps[0]]->grabData ();
          //make sure we the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              filterBlocks[ps[psb]]->rootTrigger (ttime,args, rootMask, sMode);
              args[0] = filterBlocks[ps[psb]]->getOutput ();
            }
        }
    }

}

change_code sensor::rootCheck ( const stateData *sD, const solverMode &sMode, check_level_t level)
{
  change_code ret = gridRelay::rootCheck (sD,sMode,level);
  if (stateSize (sMode) > 0)
    {
      IOdata args (1);
      for (auto &ps : processSequence)
        {
          args[0] = dataSourcesSt[ps[0]]->grabData (sD, sMode);
          //make sure we the process can be handled in states

          for (size_t psb = 1; psb < ps.size (); ++psb)
            {
              auto iret = filterBlocks[ps[psb]]->rootCheck (args, sD, sMode,level);
              ret = std::max (ret,iret);
              args[0] = filterBlocks[ps[psb]]->getOutput (args, sD, sMode);
            }
        }
    }
  return ret;
}
