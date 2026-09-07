#pragma once
#include "sv_qtcommon.h"
#include "SerializationLogic/SerializerInterface.h"
#include "Interpolation/InterpolationInterface.h"

//Every var will be stored in vec4, so im packing multiple vars in vec4's to save space
struct CameraData
{
    glm::vec3 pos               = {};
    glm::vec3 dir               = glm::vec3(0, 0, -1);
    glm::vec3 dirUp             = glm::vec3(0, 1, 0);
    glm::vec3 dirRight          = glm::vec3(1, 0, 0);
    glm::vec4 PitchYawRollYfov  = { 0.0f,  0.0f,  0.0f, glm::radians(45.0f) };

    bool operator==(const CameraData& other) const
    {
        return  glmVecEquals(pos,               other.pos)      &&
                glmVecEquals(dir,               other.dir)      &&
                glmVecEquals(dirUp,             other.dirUp)    &&
                glmVecEquals(dirRight,          other.dirRight) &&
                glmVecEquals(PitchYawRollYfov,  other.PitchYawRollYfov);
    }
};
SV_REGTYPENAME(CameraData);
SV_DECL_OPT(CameraData);

inline CameraData getFromCamera(const BasicFPSCamera& camera)
{
    CameraData data;
    data.pos                = camera.getPos();
    data.dir                = camera.getDir();
    data.dirUp              = camera.getDirUp();
    data.dirRight           = camera.getDirRight();
    data.PitchYawRollYfov   = { camera.getPitch(), camera.getYaw(), camera.getRoll(), camera.getYFov()};

    return data;
}

inline void setOnCamera(BasicFPSCamera& camera, const CameraData& data)
{
    camera.setPos   (data.pos);
    camera.setPitch (data.PitchYawRollYfov.x);
    camera.setYaw   (data.PitchYawRollYfov.y);
    camera.setRoll  (data.PitchYawRollYfov.z);
    camera.setYFov  (data.PitchYawRollYfov.w);

    //hopefully since we did set the angles, this will form same direction vectors, as in CameraData.
    //we are not setting them directly.
}

template <>
class Serializer<CameraData>
{
public:
    QJsonValue toJson(const CameraData& cd)
    {
        QJsonArray res;
        res << glmVecToJson(cd.pos)
            << glmVecToJson(cd.dir)
            << glmVecToJson(cd.dirUp)
            << glmVecToJson(cd.dirRight)
            << glmVecToJson(cd.PitchYawRollYfov);
        return res;
    }
    std::optional<CameraData> fromJson(const QJsonValue& json)
    {
        QJsonArrayOpt arr = convertJson<QJsonArray>(json);
        if (!arr) return std::nullopt;

        if (arr->size() != 5) return std::nullopt;

        Vec3Opt pos                 = glmVecFromJson<float, 3>(arr->at(0));
        Vec3Opt dir                 = glmVecFromJson<float, 3>(arr->at(1));
        Vec3Opt dirUp               = glmVecFromJson<float, 3>(arr->at(2));
        Vec3Opt dirRight            = glmVecFromJson<float, 3>(arr->at(3));
        Vec4Opt PitchYawRollYfov    = glmVecFromJson<float, 4>(arr->at(4));

        if (!pos || !dir || !dirUp || !dirRight || !PitchYawRollYfov) return std::nullopt;

        return CameraData{
            *pos, *dir, *dirUp, *dirRight, *PitchYawRollYfov
        };
    }
};

template<>
class Interpolator<CameraData>
{
public:
    static void interpolate(const CameraData&   A, 
                            const CameraData&   B, 
                            CameraData&         Result, 
                            double              ratioAToB01)
    {
        //we cant just interpolate vectors, we have to interpolate all we can
        //(angles) then set it on camera and get final dirs

        CameraData temp{};
        temp.pos                = glm::mix(A.pos,               B.pos,              ratioAToB01);
        temp.PitchYawRollYfov   = glm::mix(A.PitchYawRollYfov,  B.PitchYawRollYfov, ratioAToB01);

        BasicFPSCamera camera;

        //Now this camera contains proper direction vectors
        setOnCamera(camera, temp);
        Result = getFromCamera(camera);
    }
};