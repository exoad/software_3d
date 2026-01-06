#ifndef PUBLIC_H
#define PUBLIC_H

#include <math.h>
#include <stdlib.h>
#include "shared.h"
#include "gfx.h"
#include "renderer.h"
#include "scene.h"
#include "shaderpass.h"
#include "rendercontext.h"

Bool bmpWrite(CharSeq filename, const GfxBuffer* buffer);

#endif