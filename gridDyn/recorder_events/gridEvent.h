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

#ifndef GRIDDYN_EVENT_H_
#define GRIDDYN_EVENT_H_

// headers
//#include "gridDyn.h"

#include "gridCore.h"
#include "fileReaders.h"

#include "eventInterface.h"
#include "units.h"

class gridEventInfo
{
public:
  std::string  name;
  std::string description;
  std::string field;
  std::string file;
  std::string eString;
  std::vector<double> time;
  std::vector<double> value;

  double period = 0.0;
  index_t column = 0;
  gridUnits::units_t unitType = gridUnits::defUnit;
public:
  gridEventInfo ()
  {
  }

};

class gridEvent : public eventInterface
{
public:
  std::string  name;
  std::string description;
  std::string field;
  double period;
  double value;
  gridUnits::units_t unitType = gridUnits::defUnit;
protected:
  double triggerTime = kBigNum;
  std::shared_ptr<timeSeries> ts = nullptr;
  index_t currIndex = kNullLocation;
  gridCoreObject *m_obj = nullptr;
  std::string eFile;
  int eColumn = 0;
  bool armed = false;
public:
  gridEvent (double time0 = 0.0,double period = 0.0);
  virtual std::shared_ptr<gridEvent> clone ();
  virtual std::shared_ptr<gridEvent> clone (gridCoreObject *newObj);
  virtual ~gridEvent ();
  virtual change_code trigger ();
  virtual change_code trigger (double time);

  double nextTriggerTime () const
  {
    return triggerTime;
  }
  bool isArmed () const
  {
    return armed;
  }
  event_execution_mode executionMode () const
  {
    return event_execution_mode::normal;
  }
  virtual void setTime (double time);
  virtual void setTimeValue (double time, double val);
  void setTimeValue (const std::vector<double> &time, const std::vector<double> &val);
  void EventFile (const std::string &fname, unsigned int column = 0);
  virtual std::string toString ();

  virtual bool setTarget ( gridCoreObject *gdo,const std::string var = "");
  virtual gridCoreObject * getObject () const
  {
    return m_obj;
  }
  //friendly helper functions for sorting
protected:
  virtual void updateTrigger (double time);

  friend std::shared_ptr<gridEvent> make_event (gridEventInfo *gdEI, gridCoreObject *rootObject);
  friend std::shared_ptr<gridEvent> make_event (const std::string &eventString, gridCoreObject *rootObject);
};

/*
class gridEvent2
{
public:
        std::string  name;
        std::string description;
        std::string field;
        double value;
        gridUnits::units_t unitType = gridUnits::defUnit;
protected:
        double triggerTime;
        gridCoreObject *m_obj = nullptr;
        bool armed = false;
public:
        gridEvent2(double time0 = 0.0) const;
        virtual std::shared_ptr<gridEvent> clone();
        virtual std::shared_ptr<gridEvent> clone(gridCoreObject *newObj);
        virtual ~gridEvent2() const;
        virtual int trigger();
        virtual int trigger(double time);

        double nextTriggerTime() const
        {
                return triggerTime;
        }
        bool isArmed() const
        {
                return armed;
        }
        int executionMode() const
        {
                return NORMAL;
        }
        virtual void setTime(double time);
        virtual void setTimeValue(double time, double val);
        virtual std::string toString();

        virtual bool setTarget(gridCoreObject *gdo, const std::string var = "");
        virtual gridCoreObject * getObject() const
        {
                return m_obj;
        }
        //friendly helper functions for sorting
protected:

        friend std::shared_ptr<gridEvent> make_event(gridEventInfo *gdEI, gridCoreObject *rootObject);
        friend std::shared_ptr<gridEvent> make_event(const std::string &eventString, gridCoreObject *rootObject) const;
};

class multiEvent :public gridEvent2
{
protected:
        stringVec fields;
        std::vector<double> values;
        std::vector<gridUnits::units_t> units;
        std::vector<gridCoreObject *> objects;
public:

};


class gridPlayer: public gridEvent
{
protected:
        double period = kBigNum;
        timeSeries ts = nullptr;
        index_t currIndex = kNullLocation;
        gridCoreObject *m_obj = nullptr;
        std::string eFile;
public:
        gridPlayer(double time0 = 0.0, double period = 0.0) const;
        std::shared_ptr<gridEvent> clone();
        std::shared_ptr<gridEvent> clone(gridCoreObject *newObj);
        ~gridPlayer() const;
        int trigger();
        int trigger(double time);


        void setTime(double time);
        void setTimeValue(double time, double val);
        void setTimeValue(const std::vector<double> &time, const std::vector<double> &val);
        void EventFile(const std::string &fname, unsigned int column = 0);
        std::string toString();

        bool setTarget(gridCoreObject *gdo, const std::string var = "");

        //friendly helper functions for sorting
protected:
        void updateTrigger(double time) const;

        friend std::shared_ptr<gridEvent> make_event(gridEventInfo *gdEI, gridCoreObject *rootObject);
        friend std::shared_ptr<gridEvent> make_event(const std::string &eventString, gridCoreObject *rootObject) const;
};
*/

bool compareEvent (const std::shared_ptr<gridEvent> s1, const std::shared_ptr<gridEvent> s2);

std::shared_ptr<gridEvent> make_event (const std::string &eventString, gridCoreObject *rootObject);
std::shared_ptr<gridEvent> make_event (const std::string &field, double val, double eventTime, gridCoreObject *rootObject);
std::shared_ptr<gridEvent> make_event (gridEventInfo *gdEI, gridCoreObject *rootObject);

#endif
