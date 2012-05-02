/*=========================================================================

  Program:   ParaView
  Module:    vtkPVRenderView.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVRenderView
// .SECTION Description
// vtkRenderViewBase equivalent that is specialized for ParaView. Eventually
// vtkRenderViewBase should have a abstract base-class that this will derive from
// instead of vtkRenderViewBase since we do not use the labelling/icon stuff from
// vtkRenderViewBase.

// FIXME: Following is temporary -- until I decide if that's necessary at all.
// vtkPVRenderView has two types of public methods:
// 1. @CallOnAllProcessess -- must be called on all processes with exactly the
//                            same values.
// 2. @CallOnClientOnly    -- can be called only on the "client" process. These
//                            typically encapsulate client-side logic such as
//                            deciding if we are doing remote rendering or local
//                            rendering etc.

// Utkarsh: Try to use methods that will be called on all processes for most
// decision making similar to what ResetCamera() does. This will avoid the need
// to have special code in vtkSMRenderViewProxy and will simplify life when
// creating new views. "Move logic to VTK" -- that's the Mantra.
#ifndef __vtkPVRenderView_h
#define __vtkPVRenderView_h

#include "vtkPVView.h"
#include "vtkBoundingBox.h" // needed for iVar

class vtkAlgorithmOutput;
class vtkCamera;
class vtkCameraManipulator;
class vtkInformationDoubleKey;
class vtkInformationDoubleVectorKey;
class vtkInformationIntegerKey;
class vtkInformationObjectBaseKey;
class vtkInformationRequestKey;
class vtkInteractorStyleRubberBand3D;
class vtkInteractorStyleRubberBandZoom;
class vtkLight;
class vtkLightKit;
class vtkMatrix4x4;
class vtkProp;
class vtkPVAxesWidget;
class vtkPVCenterAxesActor;
class vtkPVDataRepresentation;
class vtkPVGenericRenderWindowInteractor;
class vtkPVHardwareSelector;
class vtkPVInteractorStyle;
class vtkPVSynchronizedRenderer;
class vtkPVSynchronizedRenderWindows;
class vtkRenderer;
class vtkRenderViewBase;
class vtkRenderWindow;
class vtkRepresentedDataStorage;
class vtkTexture;

class VTK_EXPORT vtkPVRenderView : public vtkPVView
{
  //*****************************************************************
public:
  static vtkPVRenderView* New();
  vtkTypeMacro(vtkPVRenderView, vtkPVView);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This allow the user to overcome the system capability and simply prevent
  // remote rendering.
  static bool IsRemoteRenderingAllowed();
  static void AllowRemoteRendering(bool enable);

  enum InteractionModes
    {
    INTERACTION_MODE_3D=0,
    INTERACTION_MODE_2D, // not implemented yet.
    INTERACTION_MODE_SELECTION,
    INTERACTION_MODE_ZOOM
    };

  // Description:
  // Get/Set the interaction mode. Default is INTERACTION_MODE_3D. If
  // INTERACTION_MODE_SELECTION is
  // selected, then whenever the user drags and creates a selection region, this
  // class will fire a vtkCommand::SelectionChangedEvent event with the
  // selection region as the argument.
  // @CallOnAllProcessess - this must be called on all processes, however it will
  // have any effect only the driver processes i.e. the process with the
  // interactor.
  virtual void SetInteractionMode(int mode);
  vtkGetMacro(InteractionMode, int);

  // Description:
  // Initialize the view with an identifier. Unless noted otherwise, this method
  // must be called before calling any other methods on this class.
  // @CallOnAllProcessess
  virtual void Initialize(unsigned int id);

  // Description:
  // Gets the non-composited renderer for this view. This is typically used for
  // labels, 2D annotations etc.
  // @CallOnAllProcessess
  vtkGetObjectMacro(NonCompositedRenderer, vtkRenderer);
  vtkRenderer* GetRenderer();

  // Description:
  // Get/Set the active camera. The active camera is set on both the composited
  // and non-composited renderer.
  vtkCamera* GetActiveCamera();
  virtual void SetActiveCamera(vtkCamera*);

  // Description:
  // Returns the render window.
  vtkRenderWindow* GetRenderWindow();

  // Description:
  // It's possible to directly add vtkProps to a view. This API provides access
  // to add props to the 3D/composited renderer. Note that if you add props in
  // this way, they will not be included in the computations for geometry-size
  // which is used to make decisions whether to use LOD or remote rendering etc.
  // Nor can such props participate in data-redistribution when volume rendering
  // or translucent rendering. As a rule of thumb only add props not directly
  // connected to input data using this API such as scalar bars, cube axes etc.
  void AddPropToRenderer(vtkProp*);
  void RemovePropFromRenderer(vtkProp*);

  // Description:
  // It's possible to directly add vtkProps to a view. This API provides access
  // to add props to the non-composited renderer. Note that if you add props in
  // this way, they will not be included in the computations for geometry-size
  // which is used to make decisions whether to use LOD or remote rendering etc.
  // Nor can such props participate in data-redistribution when volume rendering
  // or translucent rendering. As a rule of thumb only add props not directly
  // connected to input data using this API such as scalar bars, cube axes etc.
  void AddPropToNonCompositedRenderer(vtkProp*);
  void RemovePropFromNonCompositedRenderer(vtkProp*);

  // Description:
  // Returns the interactor. .
  vtkGetObjectMacro(Interactor, vtkPVGenericRenderWindowInteractor);

  // Description:
  // Returns the interactor style.
  vtkGetObjectMacro(InteractorStyle, vtkPVInteractorStyle);

  // Description:
  // Resets the active camera using collective prop-bounds.
  // @CallOnAllProcessess
  void ResetCamera();
  void ResetCamera(double bounds[6]);

  // Description:
  // Triggers a high-resolution render.
  // @CallOnAllProcessess
  virtual void StillRender();

  // Description:
  // Triggers a interactive render. Based on the settings on the view, this may
  // result in a low-resolution rendering or a simplified geometry rendering.
  // @CallOnAllProcessess
  virtual void InteractiveRender();

  void StreamingUpdate(int, int);

  // Description:
  // Get/Set the reduction-factor to use when for StillRender(). This is
  // typically set to 1, but in some cases with terrible connectivity or really
  // large displays, one may want to use a sub-sampled image even for
  // StillRender(). This is set it number of pixels to be sub-sampled by.
  // Note that image reduction factors have no effect when in built-in mode.
  // @CallOnAllProcessess
  vtkSetClampMacro(StillRenderImageReductionFactor, int, 1, 20);
  vtkGetMacro(StillRenderImageReductionFactor, int);

  // Description:
  // Get/Set the reduction-factor to use when for InteractiveRender().
  // This is set it number of pixels to be sub-sampled by.
  // Note that image reduction factors have no effect when in built-in mode.
  // @CallOnAllProcessess
  vtkSetClampMacro(InteractiveRenderImageReductionFactor, int, 1, 20);
  vtkGetMacro(InteractiveRenderImageReductionFactor, int);

  // Description:
  // Get/Set the data-size in megabytes above which remote-rendering should be
  // used, if possible.
  // @CallOnAllProcessess
  vtkSetMacro(RemoteRenderingThreshold, double);
  vtkGetMacro(RemoteRenderingThreshold, double);

  // Description:
  // Get/Set the data-size in megabytes above which LOD rendering should be
  // used, if possible.
  // @CallOnAllProcessess
  vtkSetMacro(LODRenderingThreshold, double);
  vtkGetMacro(LODRenderingThreshold, double);

  // Description:
  // Get/Set the LOD resolution. This affects the size of the grid used for
  // quadric clustering, for example. 1.0 implies maximum resolution while 0
  // implies minimum resolution.
  // @CallOnAllProcessess
  vtkSetClampMacro(LODResolution, double, 0.0, 1.0);
  vtkGetMacro(LODResolution, double);

  // Description:
  // This threshold is only applicable when in client-server mode. It is the size
  // of geometry in megabytes beyond which the view should not deliver geometry
  // to the client, but only outlines.
  // @CallOnAllProcessess
  vtkSetMacro(ClientOutlineThreshold, double);
  vtkGetMacro(ClientOutlineThreshold, double);

  // Description:
  // Passes the compressor configuration to the client-server synchronizer, if
  // any. This affects the image compression used to relay images back to the
  // client.
  // See vtkPVClientServerSynchronizedRenderers::ConfigureCompressor() for
  // details.
  // @CallOnAllProcessess
  void ConfigureCompressor(const char* configuration);

  // Description:
  // Resets the clipping range. One does not need to call this directly ever. It
  // is called periodically by the vtkRenderer to reset the camera range.
  virtual void ResetCameraClippingRange();

  // Description:
  // Enable/Disable light kit.
  // @CallOnAllProcessess
  void SetUseLightKit(bool enable);
  vtkGetMacro(UseLightKit, bool);
  vtkBooleanMacro(UseLightKit, bool);

  // Description:
  // Key used to pass the internal vtkRepresentedDataStorage instance to the
  // representations to get/set datasets to be delivered to rendering
  // processes.
  static vtkInformationObjectBaseKey* REPRESENTED_DATA_STORE();

  // Description:
  // USE_LOD indicates if LOD is being used for the current render/update.
  static vtkInformationIntegerKey* USE_LOD();

  // Description:
  // Indicates the LOD resolution when USE_LOD() is set.
  static vtkInformationDoubleKey* LOD_RESOLUTION();

  // Description:
  // Representation can publish this key in their REQUEST_INFORMATION() pass to
  // indicate that the representation needs ordered compositing.
  static vtkInformationIntegerKey* NEED_ORDERED_COMPOSITING();

  // Description:
  // Make a selection. This will result in setting up of this->LastSelection
  // which can be accessed using GetLastSelection().
  // @CallOnAllProcessess
  void SelectCells(int region[4]);
  void SelectCells(int region0, int region1, int region2, int region3)
    {
    int r[4] = {region0, region1, region2, region3};
    this->SelectCells(r);
    }
  void SelectPoints(int region[4]);
  void SelectPoints(int region0, int region1, int region2, int region3)
    {
    int r[4] = {region0, region1, region2, region3};
    this->SelectPoints(r);
    }
  void Select(int field_association, int region[4]);

  // Description:
  // Provides access to the last selection.
  vtkGetObjectMacro(LastSelection, vtkSelection);

  // Description:
  // Set or get whether capture should be done as
  // StillRender or InteractiveRender when capturing screenshots.
  vtkSetMacro(UseInteractiveRenderingForSceenshots, bool);
  vtkBooleanMacro(UseInteractiveRenderingForSceenshots, bool);
  vtkGetMacro(UseInteractiveRenderingForSceenshots, bool);

  // Description:
  // Set or get whether offscreen rendering should be used during
  // CaptureWindow calls. On Apple machines, this flag has no effect.
  vtkSetMacro(UseOffscreenRenderingForScreenshots, bool);
  vtkBooleanMacro(UseOffscreenRenderingForScreenshots, bool);
  vtkGetMacro(UseOffscreenRenderingForScreenshots, bool);

  // Description:
  // Get/Set whether to use offscreen rendering for all rendering. This is
  // merely a suggestion. If --use-offscreen-rendering command line option is
  // specified, then setting this flag to 0 on that process has no effect.
  // Setting it to true, however, will ensure that even is
  // --use-offscreen-rendering is not specified, it will use offscreen
  // rendering.
  virtual void SetUseOffscreenRendering(bool);
  vtkBooleanMacro(UseOffscreenRendering, bool);
  vtkGetMacro(UseOffscreenRendering, bool);

  // Description:
  // Returns if remote-rendering is possible on the current group of processes.
  vtkGetMacro(RemoteRenderingAvailable, bool);
  void RemoteRenderingAvailableOff()
    { this->RemoteRenderingAvailable = false; }

  // Description:
  // Returns true if the most recent render used LOD.
  vtkGetMacro(UsedLODForLastRender, bool);

  // Description:
  // Invalidates cached selection. Called explicitly when view proxy thinks the
  // cache may have become obsolete.
  // @CallOnAllProcessess
  void InvalidateCachedSelection();

  // Description:
  // Returns the z-buffer value at the given location.
  // @CallOnClientOnly
  double GetZbufferDataAtPoint(int x, int y);

  // Description:
  // Convenience methods used by representations to pass represented data.
  static void SetPiece(vtkInformation* info,
    vtkPVDataRepresentation* repr, vtkDataObject* data);
  static vtkAlgorithmOutput* GetPieceProducer(vtkInformation* info,
    vtkPVDataRepresentation* repr);
  static void SetPieceLOD(vtkInformation* info,
    vtkPVDataRepresentation* repr, vtkDataObject* data);
  static vtkAlgorithmOutput* GetPieceProducerLOD(vtkInformation* info,
    vtkPVDataRepresentation* repr);
  static void SetDeliverToAllProcesses(
    vtkInformation* info, vtkPVDataRepresentation* repr, bool clone);
  static void SetDeliverLODToAllProcesses(
    vtkInformation* info, vtkPVDataRepresentation* repr, bool clone);
  static void MarkAsRedistributable(
    vtkInformation* info, vtkPVDataRepresentation* repr);
  static void SetGeometryBounds(vtkInformation* info,
    double bounds[6], vtkMatrix4x4* transform = NULL);

public:
  //*****************************************************************
  // Methods merely exposing methods for internal objects.

  // Description:
  // Turn on/off the default light in the 3D renderer.
  void SetLightSwitch(bool enable);
  bool GetLightSwitch();
  vtkBooleanMacro(LightSwitch, bool);

  //*****************************************************************
  // Forwarded to orientation axes widget.
  virtual void SetOrientationAxesInteractivity(bool);
  virtual void SetOrientationAxesVisibility(bool);
  void SetOrientationAxesLabelColor(double r, double g, double b);
  void SetOrientationAxesOutlineColor(double r, double g, double b);

  //*****************************************************************
  // Forwarded to center axes.
  virtual void SetCenterAxesVisibility(bool);

  //*****************************************************************
  // Forward to vtkPVGenericRenderWindowInteractor.
  void SetCenterOfRotation(double x, double y, double z);
  void SetNonInteractiveRenderDelay(unsigned int seconds);

  //*****************************************************************
  // Forward to vtkLightKit.
  void SetKeyLightWarmth(double val);
  void SetKeyLightIntensity(double val);
  void SetKeyLightElevation(double val);
  void SetKeyLightAzimuth(double val);
  void SetFillLightWarmth(double val);
  void SetKeyToFillRatio(double val);
  void SetFillLightElevation(double val);
  void SetFillLightAzimuth(double val);
  void SetBackLightWarmth(double val);
  void SetKeyToBackRatio(double val);
  void SetBackLightElevation(double val);
  void SetBackLightAzimuth(double val);
  void SetHeadLightWarmth(double val);
  void SetKeyToHeadRatio(double val);
  void SetMaintainLuminance(int val);

  //*****************************************************************
  // Forward to 3D renderer.
  void SetUseDepthPeeling(int val);
  void SetMaximumNumberOfPeels(int val);
  void SetBackground(double r, double g, double b);
  void SetBackground2(double r, double g, double b);
  void SetBackgroundTexture(vtkTexture* val);
  void SetGradientBackground(int val);
  void SetTexturedBackground(int val);

  //*****************************************************************
  // Forward to vtkLight.
  void SetAmbientColor(double r, double g, double b);
  void SetSpecularColor(double r, double g, double b);
  void SetDiffuseColor(double r, double g, double b);
  void SetIntensity(double val);
  void SetLightType(int val);

  //*****************************************************************
  // Forward to vtkRenderWindow.
  void SetStereoCapableWindow(int val);
  void SetStereoRender(int val);
  void SetStereoType(int val);
  void SetMultiSamples(int val);
  void SetAlphaBitPlanes(int val);
  void SetStencilCapable(int val);

  //*****************************************************************
  // Forwarded to vtkPVInteractorStyle if present on local processes.
  void Add2DManipulator(vtkCameraManipulator* val);
  void RemoveAll2DManipulators();
  void Add3DManipulator(vtkCameraManipulator* val);
  void RemoveAll3DManipulators();

  // Description:
  // Overridden to synchronize information among processes whenever data
  // changes. The vtkSMViewProxy ensures that this method is called only when
  // something has changed on the view-proxy or one of its representations or
  // their inputs. Hence it's okay to do some extra inter-process communication
  // here.
  virtual void Update();

  // Description:
  // Asks representations to update their LOD geometries.
  virtual void UpdateLOD();

  // Description:
  // Returns whether the view will use LOD rendering for the next
  // InteractiveRender() call based on the geometry sizes determined by the most
  // recent call to Update().
  vtkGetMacro(UseLODForInteractiveRender, bool);

  // Description:
  // Returns whether the view will use outline boxes for the next
  // InteractiveRender() call based on the geometry sizes determined by the most
  // recent call to Update().
  vtkGetMacro(UseOutlineForInteractiveRender, bool);

  // Description:
  // Returns whether the view will use distributed rendering for the next
  // StillRender() call based on the geometry sizes determined by the most
  // recent call to Update().
  vtkGetMacro(UseDistributedRenderingForStillRender, bool);

  // Description:
  // Returns whether the view will use distributed rendering for the next
  // InteractiveRender() call based on the geometry sizes determined by the most
  // recent calls to Update() and UpdateLOD().
  vtkGetMacro(UseDistributedRenderingForInteractiveRender, bool);

  // Description:
  // Returns the processes (vtkPVSession::ServerFlags) that are to be involved
  // in the next StillRender() call based on the decisions made during the most
  // recent Update().
  vtkGetMacro(StillRenderProcesses, vtkTypeUInt32);

  // Description:
  // Returns the processes (vtkPVSession::ServerFlags) that are to be involved
  // in the next InteractiveRender() call based on the decisions made during the most
  // recent Update() and UpdateLOD().
  vtkGetMacro(InteractiveRenderProcesses, vtkTypeUInt32);

  // Description:
  // Returns the data distribution mode to use.
  int GetDataDistributionMode(bool use_remote_rendering);

  // Description:
  // Provides access to the geometry storage for this view.
  vtkRepresentedDataStorage* GetGeometryStore();

  // Description:
  // Returns true when ordered compositing is needed on the current group of
  // processes. Note that unlike most other functions, this may return different
  // values on different processes e.g.
  // \li always false on client and dataserver
  // \li true on pvserver or renderserver if opacity < 1 or volume present, else
  //     false
  bool GetUseOrderedCompositing();

  // Description:
  // Synchronizes core ivars for multi-client setups.
  bool SynchronizeForCollaboration();

//BTX
protected:
  vtkPVRenderView();
  ~vtkPVRenderView();

  // Description:
  // Overridden to assign IDs to each representation. This assumes that
  // representations will be added/removed in a consistent fashion across
  // processes even in multi-client modes. The only exception is
  // vtk3DWidgetRepresentation. However, since vtk3DWidgetRepresentation never
  // does any data-delivery, we don't assign IDs for these, nor affect the ID
  // uniquifier when a vtk3DWidgetRepresentation is added.
  virtual void AddRepresentationInternal(vtkDataRepresentation* rep);
  virtual void RemoveRepresentationInternal(vtkDataRepresentation* rep);

  // Description:
  // These methods are used to setup the view for capturing screen shots.
  // In batch mode, since the server-side has just 1 render window, we need to
  // make sure that the right interactor is activated, otherwise, we end up
  // capturing images from the wrong view.
  virtual void PrepareForScreenshot();

  // Description:
  // Actual render method.
  virtual void Render(bool interactive, bool skip_rendering);

  // Description:
  // Returns true if distributed rendering should be used based on the geometry
  // size.
  bool ShouldUseDistributedRendering(double geometry_size);

  // Description:
  // Returns true if LOD rendering should be used based on the geometry size.
  bool ShouldUseLODRendering(double geometry);

  // Description:
  // Synchronizes bounds information on all nodes.
  // @CallOnAllProcessess
  void SynchronizeGeometryBounds();

  // Description:
  // Set the last selection object.
  void SetLastSelection(vtkSelection*);

  // Description:
  // UpdateCenterAxes().
  // Updates CenterAxes's scale and position.
  void UpdateCenterAxes();

  // Description
  // Returns true if the local process is doing to do actual render or
  // displaying an image in a viewport.
  bool GetLocalProcessDoesRendering(bool using_distributed_rendering);

  vtkLight* Light;
  vtkLightKit* LightKit;
  vtkRenderViewBase* RenderView;
  vtkRenderer* NonCompositedRenderer;
  vtkPVSynchronizedRenderer* SynchronizedRenderers;
  vtkPVGenericRenderWindowInteractor* Interactor;
  vtkInteractorStyleRubberBand3D* RubberBandStyle;
  vtkInteractorStyleRubberBandZoom* RubberBandZoom;
  vtkPVCenterAxesActor* CenterAxes;
  vtkPVAxesWidget* OrientationWidget;
  vtkPVHardwareSelector* Selector;
  vtkSelection* LastSelection;

  int StillRenderImageReductionFactor;
  int InteractiveRenderImageReductionFactor;
  int InteractionMode;

  // 2D and 3D interactor style
  vtkPVInteractorStyle* TwoDInteractorStyle;
  vtkPVInteractorStyle* ThreeDInteractorStyle;

  // Active interactor style either [TwoDInteractorStyle, ThreeDInteractorStyle]
  vtkPVInteractorStyle* InteractorStyle;

  // Used in collaboration mode to ensure that views are in the same state
  // (as far as representations added/removed goes) before rendering.
  int SynchronizationCounter;

  // In mega-bytes.
  double RemoteRenderingThreshold;
  double LODRenderingThreshold;
  double ClientOutlineThreshold;
  vtkBoundingBox GeometryBounds;

  bool UseOffscreenRendering;
  bool UseOffscreenRenderingForScreenshots;
  bool UseInteractiveRenderingForSceenshots;
  bool NeedsOrderedCompositing;

  double LODResolution;
  bool UseLightKit;

  bool UsedLODForLastRender;
  bool UseLODForInteractiveRender;
  bool UseOutlineForInteractiveRender;
  bool UseDistributedRenderingForStillRender;
  bool UseDistributedRenderingForInteractiveRender;

  static bool RemoteRenderingAllowed;

  vtkTypeUInt32 StillRenderProcesses;
  vtkTypeUInt32 InteractiveRenderProcesses;

private:
  vtkPVRenderView(const vtkPVRenderView&); // Not implemented
  void operator=(const vtkPVRenderView&); // Not implemented

  bool MakingSelection;
  void OnSelectionChangedEvent();
  void FinishSelection(vtkSelection*);

  // This flag is set to false when not all processes cannot render e.g. cannot
  // open the DISPLAY etc.
  bool RemoteRenderingAvailable;

  int PreviousParallelProjectionStatus;

  class vtkInternals;
  vtkInternals* Internals;
//ETX
};

#endif
