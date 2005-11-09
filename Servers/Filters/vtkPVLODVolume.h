/*=========================================================================

  Program:   ParaView
  Module:    vtkPVLODVolume.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVLODVolume - an actor that supports multiple levels of detail
// .SECTION Description
// vtkPVLODVolume is much like vtkPVLODActor except that it works on
// volumes instead of surfaces.  This just has two mappers: full res and
// LOD, and this actor knows which is which.

// .SECTION see also
// vtkActor vtkRenderer vtkLODProp3D vtkLODActor

#ifndef __vtkPVLODVolume_h
#define __vtkPVLODVolume_h

#include "vtkVolume.h"

class vtkLODProp3D;
class vtkMapper;
class vtkPVRenderModuleHelper;

class VTK_EXPORT vtkPVLODVolume : public vtkVolume
{
public:
  vtkTypeRevisionMacro(vtkPVLODVolume,vtkVolume);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkPVLODVolume *New();

  // Description:
  // This method is used internally by the rendering process.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentGeometry(vtkViewport *viewport);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Set the high res input.  Overloads the virtual vtkVolume method.
  virtual void SetMapper(vtkAbstractVolumeMapper *);

  // Description:
  // This sets the low res input.
  virtual void SetLODMapper(vtkAbstractVolumeMapper *);
  virtual void SetLODMapper(vtkMapper *);

  // Description:
  // Sets the volume propery.  Overloads the virtual vtkVolume method.
  virtual void SetProperty(vtkVolumeProperty *property);

  // Description:
  // Shallow copy of an LOD actor. Overloads the virtual vtkProp method.
  virtual void ShallowCopy(vtkProp *prop);

  // Description:
  // Get the bounds of the current mapper.
  virtual double *GetBounds();

  // Description:
  // Overloads the virtual vtkProp method.
  virtual void SetAllocatedRenderTime(double t, vtkViewport *v);

  // Description:
  // Get/Set the vtkPVRenderModuleHelper. This helper is a representation
  // of the render module and must be set before rendering. The helper is used
  // to query the LOD rendering decision made by the render module.
  vtkGetObjectMacro(RenderModuleHelper, vtkPVRenderModuleHelper);
  void SetRenderModuleHelper(vtkPVRenderModuleHelper*);
protected:
  vtkPVLODVolume();
  ~vtkPVLODVolume();

  vtkLODProp3D *LODProp;
  int HighLODId;
  int LowLODId;

  int SelectLOD();

  double MapperBounds[6];
  vtkTimeStamp BoundsMTime;

  vtkPVRenderModuleHelper* RenderModuleHelper;

  virtual void UpdateLODProperty();

private:
  vtkPVLODVolume(const vtkPVLODVolume&); // Not implemented.
  void operator=(const vtkPVLODVolume&); // Not implemented.
};

#endif


