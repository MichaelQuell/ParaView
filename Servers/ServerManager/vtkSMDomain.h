/*=========================================================================

  Program:   ParaView
  Module:    vtkSMDomain.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMDomain - represents the possible values a property can have
// .SECTION Description
// vtkSMDomain is an abstract class that describes the "domain" of a
// a widget. A domain is a collection of possible values a property
// can have.
// Each domain can depend on one or more properties to compute it's
// values. This are called "required" properties and can be set in
// the XML configuration file.
// .SECTION See Also
// vtkSMProxyGroupDomain

#ifndef __vtkSMDomain_h
#define __vtkSMDomain_h

#include "vtkSMObject.h"

class vtkSMProperty;
class vtkPVXMLElement;
//BTX
struct vtkSMDomainInternals;
//ETX

class VTK_EXPORT vtkSMDomain : public vtkSMObject
{
public:
  vtkTypeRevisionMacro(vtkSMDomain, vtkSMObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Is the (unchecked) value of the property in the domain? Overwritten by
  // sub-classes.
  virtual int IsInDomain(vtkSMProperty* property) = 0;

  // Description:
  // Update self based on the "unchecked" values of all required
  // properties. Overwritten by sub-classes.
  virtual void Update(vtkSMProperty*) {};

  // Description:
  // Set the value of an element of a property from the animation editor.
  virtual void SetAnimationValue(vtkSMProperty*, int, double) {}

protected:
  vtkSMDomain();
  ~vtkSMDomain();

//BTX
  friend class vtkSMProperty;
//ETX

  // Description:
  // Returns a given required property of the given function.
  // Function is a string associated with the require property
  // in the XML file.
  vtkSMProperty* GetRequiredProperty(const char* function);

  // Description:
  // Remove the given property from the required properties list.
  void RemoveRequiredProperty(vtkSMProperty* prop);

  // Description:
  // Set the appropriate ivars from the xml element. Should
  // be overwritten by subclass if adding ivars.
  virtual int ReadXMLAttributes(vtkSMProperty* prop, vtkPVXMLElement* elem);

  // Description:
  // When the IsOptional flag is set, IsInDomain() always returns true.
  // This is used by properties that use domains to provide information
  // (a suggestion to the gui for example) as opposed to restrict their
  // values.
  vtkSetMacro(IsOptional, int);
  vtkGetMacro(IsOptional, int);
  int IsOptional;

  char* XMLName;

  // Description:
  // Assigned by the XML parser. The name assigned in the XML
  // configuration. Can be used to figure out the origin of the
  // domain.
  vtkSetStringMacro(XMLName);

  vtkSMDomainInternals* Internals;


private:
  vtkSMDomain(const vtkSMDomain&); // Not implemented
  void operator=(const vtkSMDomain&); // Not implemented
};

#endif
