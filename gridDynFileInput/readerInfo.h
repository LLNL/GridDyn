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

#ifndef READERINFO_H_
#define READERINFO_H_

//forward declarations
class readerElement;

class gridGrabberInfo;

#include "gridParameter.h"

#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <memory>
#include <tuple>

class collector;
class gridEvent;
class coreObject;

/** @brief class containing some basic information for reading power system files*/
class basicReaderInfo
{
public:
  double base = 100.0;       //!< base power in MW
  double basefreq = 60.0;       //!< base frequency in Hz
  uint32_t flags = 0;        //!< any reader flags
  int version = 0;        //!< file version info
  std::string prefix;        //!< prefix to add to the names of imported objects
  /** @brief default constructor*/
  basicReaderInfo ();

  /** @brief construct a basicReaderInfo object from another one
  @param[in] a pointer to another basicReaderInfo object*/
  explicit basicReaderInfo (const basicReaderInfo *bri);

  /** @brief virtual destructor*/
  virtual ~basicReaderInfo ();
};

const basicReaderInfo defInfo = basicReaderInfo ();

/** @brief a class defining information to help manage the inputs files
*/
class readerInfo : public basicReaderInfo
{
public:
  std::vector < std::shared_ptr < collector >> collectors;         //!<stores the active recorders
  std::list < std::shared_ptr < gridEvent >> events;          //!< store the captured events
  bool keepdefines = false;
  typedef std::uint64_t scopeID;
private:
  std::unordered_map<std::string, std::string> defines;              //!<storages for string definitions
  std::vector<std::string > directories;              //!<stores a list of folders for finding files
  std::unordered_map<std::string, std::string> objectTranslations;           //!<storage for object Translations
  std::unordered_map<std::string, std::string> objectTranslationsType;          //!<storage for the type associated with an object translation
  std::map < std::string, std::pair < coreObject *, std::vector<gridParameter >>> library;          //!< library objects
  std::unordered_map<std::string, std::string> lockDefines;       //!< locked definitions
  std::map<std::string, std::pair<std::shared_ptr<readerElement>,int>> customElements;       //!< custom objects
  scopeID currentScope = 0;
  std::vector<std::tuple<scopeID, std::string,bool,std::string>> scopedDefinitions;
  std::vector<scopeID> directoryScope;
  std::unordered_set<std::string> parameterIgnoreStrings;
public:
  /** @brief default constructor*/
  readerInfo ();
  /** @brief construct a reader info object from a basic readerInfo object
  @param[in] bri  a basic reader info object
  */
  explicit readerInfo (const basicReaderInfo *bri);
  /**@brief destructor*/
  ~readerInfo ();
  /** @brief add an object to the library
  @param[in] obj  the object to add
  @param[in] pobjs a set of parameter objects to apply to newly created objects
  @return true if object was successfully added
  */
  bool addLibraryObject (coreObject *obj, std::vector<gridParameter> &pobjs);
  /** @brief  create an object based on a reference object in the library
  @param[in] ename  the name of the object in the library to find
  @param[in] mobj set to nullptr to create a new object, if set the library object is cloned onto the given object
  @return a pointer to the new object created/or cloned from the library object
  */
  coreObject * makeLibraryObject (const std::string &ename, coreObject *mobj);
  /** @brief get a pointer to a library object
  @param[in] ename  the name of the object in the library to locate
  @return nullptr if no object found or a pointer to the library object
  */
  coreObject * findLibraryObject (const std::string &ename) const;

  /** @brief  find a collector stored in the readerInfo either by name or by sink location
  *@param[in] name  the name of the collector to find
  @param[in] fname  the sink name of the collector to find by file name
  @return a shared pointer to the collector or nullptr if not found
  */
  std::shared_ptr<collector> findCollector (const std::string &name, const std::string &fname);

  /** @brief change the scope for definitions and translations
  @return the new scopeID for closing the scope later */
  scopeID newScope ();

  /** @brief change the scope for definitions and translations
  @param[in] scopeToClose the scopeID for closing*/
  void closeScope (scopeID scopeToClose = 0);

  /** @brief  add a definition to the reader info object
    the definition will only be added or updated if there is no locked definition of the same name
  @param[in] def the string that has a translation
  @param[in] replacement  the string to replace def with
  */
  void addDefinition (std::string def, std::string replacement);

  /** @brief  add a definition to the reader info object
    the definition will only be added or updated if there is no locked definition of the same name
  @param[in] def the string that has a translation
  @param[in] replacement  the string to replace def with
  */
  void addLockedDefinition (std::string def, std::string replacement);

  /** @brief  add or replace a locked definition
    this function will replace any existing locked definition with a new one
  @param[in] def the string that has a translation
  @param[in] replacement  the string to replace def with
  */
  void replaceLockedDefinition (std::string def, std::string replacement);

  /** @brief  add a name translation
   name translations are one to one translations of object names for declaring new names for convenience
  @param[in] def the object name needing translation
  @param[in] component the underlying component name usually one with a defined reader
  */
  void addTranslate (std::string def, std::string component);

  /** @brief  add a name translation with a specific type
   name translations are one to one translations of object names for declaring new names for convenience and can include a specific type of the component along with it
  @param[in] def the object name needing translation
  @param[in] component the underlying component name usually one with a defined reader
  @param[in] type the type of the object to go along with the component
  */
  void addTranslate (std::string def, std::string component, std::string type);

  /** @brief  add a type specification associated with a component
   name translations are one to one translations of object names for declaring new names for convenience
  @param[in] def the object name needing translation
  @param[in] type the type associated with a given definition
  */
  void addTranslateType (std::string def, std::string type);

  /** @brief  add a custom object to the reader Info
   unlike library objects a pointer to the actual XML code is stored and reread to create an object allowing for creation of complex
  XML structures as a library
  @param[in] name the name of the custom object
  @param[in] element  a pointer to the xml element with the object creation information
	@param[in] nargs the number of arguments the custom element has
  */
  void addCustomElement (std::string name, const std::shared_ptr<readerElement> &element, int nargs);

  /** @brief  check if a string represents a custom object
  @param[in] name the name of the supposed custom object
  @return true if the string names a custom object
  */
  bool isCustomElement (const std::string &name) const;

  /** @brief  retrieve a custom object
  @param[in] name the name of the supposed custom object
  @return a shared pointer to the custom object
  */
  const std::pair<std::shared_ptr<readerElement>, int > getCustomElement (const std::string &name) const;

  /** @brief check and translate a string for definitions
   does a direct translation for any strings also interprets and translates to a string anything contained within pairs of $
  @param[in] input  the input string to translate
  @return  the translated string
  */
  std::string checkDefines (const std::string input);

  /** @brief check and translate an object name
   does a direct translation of any previously defined names to translate
  @param[in] input  the input string to translate
  @return  the translated object name
  */
  std::string objectNameTranslate (const std::string input);

  /** @brief add directory*/
  void addDirectory (const std::string &directory);

  /** @brief check that a file parameter is valid
  @param[in,out] strVal  the file to check and output an updated file location
  @param[in] extra_find if the file cannot be found on the first check, ignore the path information and check again for just the filename in known locations
  @return true if the file was changed false otherwise
  */
  bool checkFileParam (std::string &strVal, bool extra_find = false);

  /** @brief check if a directory is valid
  @param[in,out] strVal  the directory to check and output an updated location
  @return true if the file was changed false otherwise
  */
  bool checkDirectoryParam (std::string &strVal);
  /** @brief get a container of all the strings which should be interpreted as blocks instead of parameters
  */
  const std::unordered_set<std::string> &getIgnoreList () const;
private:
  /** @brief load some default defines such as %time, %date, and %datetime */
  void loadDefaultDefines ();
};


#endif