#pragma once
#include "sv_qtcommon.h"
#include "SerializationLogic/SerializerInterface.h"
#include "Interpolation/InterpolationInterface.h"
#include "WidgetLogic/WidgetDefs.h"
#include "CameraScaleConstraint.h"

//Every var will be stored in vec4, so im packing multiple vars in vec4's to save space.
//What actually defines camera is 'PitchYawRoll' angles. Dir vectors is data DERIVED from angles.
struct CameraData
{
    glm::vec3 pos               = {};
    glm::vec3 dir               = glm::vec3(0, 0, -1);
    glm::vec3 dirUp             = glm::vec3(0, 1, 0);
    glm::vec3 dirRight          = glm::vec3(1, 0, 0);
    glm::vec4 PitchYawRollYfov  = { 0.0f,  0.0f,  0.0f, glm::radians(45.0f) };

    //Its 2 values
    // 1) x: Mode: 0 or 1: whether phase scaling mode is enabled  
    // 2) y: Phase: if Mode is 1, its unconstrained phase01u number
    glm::vec4 ModePhase         = {}; 

    bool operator==(const CameraData& other) const
    {
        return  glmVecEquals(pos,               other.pos)      &&
                glmVecEquals(dir,               other.dir)      &&
                glmVecEquals(dirUp,             other.dirUp)    &&
                glmVecEquals(dirRight,          other.dirRight) &&
                glmVecEquals(PitchYawRollYfov,  other.PitchYawRollYfov) &&
                glmVecEquals(ModePhase,         other.ModePhase);
    }
};
SV_REGTYPENAME(CameraData);
SV_DECL_OPT(CameraData);

inline CameraData getFromCamera(const BasicFPSCamera& camera, Vec4Opt ModePhaseOpt = std::nullopt)
{
    CameraData data;
    data.pos                = camera.getPos();
    data.dir                = camera.getDir();
    data.dirUp              = camera.getDirUp();
    data.dirRight           = camera.getDirRight();
    data.PitchYawRollYfov   = { camera.getPitch(), camera.getYaw(), camera.getRoll(), camera.getYFov()};
    data.ModePhase          = ModePhaseOpt.value_or(glm::vec4{});

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
        QJsonObject obj;
        obj[TypeFieldKey] = typeName<CameraData>();

        QJsonArray data;
        data    << glmVecToJson(cd.pos)
                << glmVecToJson(cd.dir)
                << glmVecToJson(cd.dirUp)
                << glmVecToJson(cd.dirRight)
                << glmVecToJson(cd.PitchYawRollYfov)
                << glmVecToJson(cd.ModePhase);

        obj["data"] = data;

        return obj;
    }

    CameraDataOpt fromJson(const QJsonValue& json)
    {
        CameraDataOpt result = fromJsonBase(json);
        if (!result)
        {
            SV_ERROR("Couldnt deserialize CameraData from json, gonna return default-initialized CameraData{}");
        }

        return result.value_or(CameraData{});
    }

    CameraDataOpt fromJsonBase(const QJsonValue& json)
    {
        QJsonObjectOpt obj = convertJson<QJsonObject>(json);
        if (!obj) return std::nullopt;

        QJsonArrayOpt arr = getFromJson<QJsonArray>(obj, "data");
        if (!arr) return std::nullopt;

        if (arr->size() != 6) return std::nullopt;

        Vec3Opt pos                 = glmVecFromJson<float, 3>(arr->at(0));
        Vec3Opt dir                 = glmVecFromJson<float, 3>(arr->at(1));
        Vec3Opt dirUp               = glmVecFromJson<float, 3>(arr->at(2));
        Vec3Opt dirRight            = glmVecFromJson<float, 3>(arr->at(3));
        Vec4Opt PitchYawRollYfov    = glmVecFromJson<float, 4>(arr->at(4));
        Vec4Opt phase               = glmVecFromJson<float, 4>(arr->at(5));

        if (!pos || !dir || !dirUp || !dirRight || !PitchYawRollYfov || !phase) return std::nullopt;

        return CameraData{
            *pos, *dir, *dirUp, *dirRight, *PitchYawRollYfov, *phase
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

        //its tricky, but lets start with initializing with mixed:
        glm::vec4 ModePhase = glm::mix(A.ModePhase, B.ModePhase, ratioAToB01);
        {
            bool A_phaseModeOn = A.ModePhase.x >= 0.5;
            bool B_phaseModeOn = A.ModePhase.x >= 0.5;

            if (A_phaseModeOn && B_phaseModeOn)
            {
                ModePhase.x = 1.0;

                //see, if we are interpolating phases, then its phase which decides actual pos:

                temp.pos = CameraScaleConstraint::applyPhase(temp.pos, ModePhase.y, ModePhase.z, ModePhase.w);
            }
            else
            {
                ModePhase.x = 0.0;
            }
        }

        BasicFPSCamera camera;

        //Now this camera contains proper direction vectors
        setOnCamera(camera, temp);
        Result = getFromCamera(camera, ModePhase);
    }
};