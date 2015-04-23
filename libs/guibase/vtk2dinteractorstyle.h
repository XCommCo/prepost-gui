#ifndef VTK_2D_INTERACTOR_STYLE
#define VTK_2D_INTERACTOR_STYLE 1

#include "guibase_global.h"

#include <vtkInteractorStyleTrackballCamera.h>

class GUIBASEDLL_EXPORT vtk2DInteractorStyle :
	public vtkInteractorStyleTrackballCamera
{
public:
	static vtk2DInteractorStyle* New();
	void OnLeftButtonDown();
	virtual ~vtk2DInteractorStyle(void);

private:
	vtk2DInteractorStyle(void);
};

#endif