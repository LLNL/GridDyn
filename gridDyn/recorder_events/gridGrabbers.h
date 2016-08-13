/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#ifndef GRID_GRABBER_H_
#define GRID_GRABBER_H_

#include "basicDefs.h"
#include "gridObjects.h"
#include <functional>
#include <memory>

class gridBus;
class gridLoad;
class gridLink;
class gridDynGenerator;
class gridSimulation;
class gridArea;
class gridRelay;

class gridGrabber
{
public:
  std::string field;
  std::string desc;
  bool loaded = false;
  bool vectorGrab = false;
  gridUnits::units_t outputUnits = gridUnits::defUnit;
  gridUnits::units_t inputUnits = gridUnits::defUnit;
  double gain = 1.0;
  double bias = 0.0;
  double m_baseVoltage = 100;
protected:
  gridCoreObject *cobj = nullptr;
  std::function<double ()> fptr;
  std::function<void(std::vector<double> &)> fptrV;
  std::function<void(stringVec &)> fptrN;
public:
  gridGrabber ()
  {
  }
  virtual ~gridGrabber ()
  {
  }
  virtual std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const;
  virtual int setInfo (std::string fld, gridCoreObject* obj);
  virtual double grabData ();
  virtual void grabData (std::vector<double> &data);
  virtual void getDesc (std::vector<std::string > &desc_list) const;
  virtual void updateObject (gridCoreObject *obj);
  virtual gridCoreObject * getObject () const
  {
    return cobj;
  }
protected:
  void makeDescription ();
};

/** custom grabber function class
*/
class customGrabber : public gridGrabber
{
public:
  void setGrabberFunction (std::string fld,std::function<double ()> nfptr);
};

std::shared_ptr<gridGrabber> createGrabber (const std::string &fld, gridCoreObject *obj);
std::shared_ptr<gridGrabber> createGrabber (int noffset, gridCoreObject *obj);

std::vector < std::shared_ptr < gridGrabber >> makeGrabbers (const std::string & command, gridCoreObject * obj);


class gridBusGrabber : public gridGrabber
{
protected:
  gridBus *bus;
public:
  gridBusGrabber (std::string fld, gridBus *gdB);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  int setInfo (std::string fld, gridCoreObject *gdO) override;
  void updateObject (gridCoreObject *obj) override;
};

class gridLoadGrabber : public gridGrabber
{
protected:
  gridLoad *load;
  index_t offset = kInvalidLocation;
public:
  gridLoadGrabber (std::string fld, gridLoad *gdL);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  int setInfo (std::string fld, gridCoreObject *gdO) override;

  void updateObject (gridCoreObject *obj) override;
  using gridGrabber::grabData;
  double grabData () override;
};

class gridAreaGrabber : public gridGrabber
{
protected:
  gridArea *area;
  //double (gridArea::*dgptr)();
  //int(gridArea::*dgptrV)(std::vector<double> &,int);
  //int (gridArea::*nmptr)(stringVec &, int);
  //int (gridSimulation::*dgptrV)(std::vector<double> &);

public:
  gridAreaGrabber (std::string fld, gridArea *gdA);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  int setInfo (std::string fld, gridCoreObject *gdO) override;

  void updateObject (gridCoreObject *obj) override;
};


class gridLinkGrabber : public gridGrabber
{
protected:
  gridLink *link;
  //double (gridLink::*dgptr) const();
public:
  gridLinkGrabber (std::string fld, gridLink *gdL);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  int setInfo (std::string fld, gridCoreObject *gdO) override;

  void updateObject (gridCoreObject *obj) override;
};

class gridDynGenGrabber : public gridGrabber
{
protected:
  gridDynGenerator *gen;
  //double (gridDynGenerator::*dgptr) const();
  index_t offset = kInvalidLocation;
public:
  gridDynGenGrabber (std::string fld, gridDynGenerator *gdG);
  gridDynGenGrabber (index_t nOffset, gridDynGenerator *gdG);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  int setInfo (std::string fld, gridCoreObject *gdO) override;
  int setInfo (index_t nOffset, gridCoreObject *gdO);
  double grabData () override;
  using gridGrabber::grabData;
  void updateObject (gridCoreObject *obj) override;
};

class subModelGrabber : public gridGrabber
{
protected:
  gridSubModel *sub;
  index_t offset = kInvalidLocation;
public:
  subModelGrabber (std::string fld, gridSubModel *gdG);
  subModelGrabber (index_t nOffset, gridSubModel *gdG);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  int setInfo (std::string fld, gridCoreObject *gdO) override;
  int setInfo (index_t nOffset, gridCoreObject *gdO);
  double grabData () override;
  using gridGrabber::grabData;
  void updateObject (gridCoreObject *obj) override;
};


class gridRelayGrabber : public gridGrabber
{
protected:
  gridRelay *rel;
  //double (gridLink::*dgptr) const();
public:
  gridRelayGrabber (std::string fld, gridRelay *gdR);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  int setInfo (std::string fld, gridCoreObject *gdO) override;

  void updateObject (gridCoreObject *obj) override;
};

class functionGrabber : public gridGrabber
{
public:
protected:
  std::shared_ptr<gridGrabber> bgrabber;
  std::string function_name;
  std::function<double(double val)> opptr;
  std::function<double(std::vector<double>)> opptrV;
  std::vector<double> tempArray;

public:
  functionGrabber ();
  functionGrabber (std::shared_ptr<gridGrabber> ggb, std::string func);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  double grabData () override;
  virtual void grabData (std::vector<double> &data) override;
  void updateObject (gridCoreObject *obj) override;
  gridCoreObject * getObject () const override;
  int setInfo (std::string fld, gridCoreObject* obj) override;
  virtual void getDesc (std::vector<std::string > &desc_list) const override;
};

class opGrabber : public gridGrabber
{
protected:
  std::shared_ptr<gridGrabber> bgrabber1;
  std::shared_ptr<gridGrabber> bgrabber2;
  std::string op_name;
  std::function<double(double, double)> opptr;
  std::function<double(std::vector<double>, std::vector<double>)> opptrV;
  std::vector<double> tempArray1,tempArray2;
public:
  opGrabber ();
  opGrabber (std::shared_ptr<gridGrabber> ggb1, std::shared_ptr<gridGrabber> ggb2, std::string op);
  std::shared_ptr<gridGrabber> clone (gridCoreObject *nobj = nullptr, std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  double grabData () override;
  virtual void grabData (std::vector<double> &data) override;
  void updateObject (gridCoreObject *obj) override;
  void updateObject (gridCoreObject *obj, int num);
  gridCoreObject * getObject () const override;
  int setInfo (std::string fld, gridCoreObject* obj) override;
  virtual void getDesc (std::vector<std::string > &desc_list) const override;
};
/*
class expressionGrabber : public gridGrabber
{
protected:
        std::vector<gridGrabber *> glist;
        std::vector<int> index;
        std::string expression;

};

*/
#endif
