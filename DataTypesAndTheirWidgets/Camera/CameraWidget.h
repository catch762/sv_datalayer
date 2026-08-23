#pragma once
#include "sv_qtcommon.h"

struct CameraData
{
	std::array<float, 4> pos	= {};
	std::array<float, 4> lookat = {};
};

SV_REGTYPENAME(CameraData);