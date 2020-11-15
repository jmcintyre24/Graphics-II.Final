#pragma once

#define mouseThreshold 1

// Simple basecode showing how to create a window and attatch a d3d11surface
#define GATEWARE_ENABLE_CORE
#define GATEWARE_ENABLE_SYSTEM
#define GATEWARE_ENABLE_GRAPHICS 
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX12SURFACE 
#define GATEWARE_DISABLE_GRASTERSURFACE
#define GATEWARE_DISABLE_GOPENGLSURFACE
#define GATEWARE_DISABLE_GVULKANSURFACE 

#include "../Gateware/Gateware.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <vector>

using namespace DirectX;