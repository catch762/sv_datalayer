#pragma once
#include "sv_qtcommon.h"


struct CameraData
{
    glm::vec3 pos       = {};
    glm::vec3 dir       = glm::vec3(0, 0, -1);
    glm::vec3 dirUp     = glm::vec3(0, 1, 0);
    glm::vec3 dirRight  = glm::vec3(1, 0, 0);
    glm::vec2 YfovRoll  = { glm::radians(45.0f), 0.0f }; //packing, cause every var takes vec4 space
};
SV_REGTYPENAME(CameraData);

inline CameraData getFromCamera(const BasicFPSCamera& camera)
{
    CameraData data;
    data.pos        = camera.getPos();
    data.dir        = camera.getDir();
    data.dirUp      = camera.getDirUp();
    data.dirRight   = camera.getDirRight();
    data.YfovRoll   = {camera.getYFov(), camera.getRoll()};

    return data;
}