#pragma once
#include "sv_qtcommon.h"

class CameraScaleConstraint
{
public:

    static std::pair<double /*newphase01u*/, glm::vec3 /*scaledpoint*/> constrainPos(   const glm::vec3 pos,
                                                                                        const double    oldphase01u,
                                                                                        const double    baseRadius,
                                                                                        const double    scale)
    {
        SV_ASSERT(scale >= 1.0);

        //********************************************************************
        // 
        // The following defines three zones:
        // 
        //  [zone +1]: [radiusPrev, radiusMin]
        //  [zone  0]: [radiusMin,  radiusMax]
        //  [zone -1]: [radiusMax,  radiusNext]
        //
        // We have to:
        //      - calculate new phase01u (which is zone idx integer + ratio to next SMALLEST zone from 0 to 1)
        //      - return camera to zone 0.
        // 
        //********************************************************************
        const double radiusPrev = baseRadius / scale;
        const double radiusMin  = baseRadius;
        const double radiusMax  = baseRadius * scale;
        const double radiusNext = baseRadius * scale * scale;

        //yes, we only calculate "horizontal plane radius" for now
        const double radiusCur  = glm::length(glm::vec2(pos.x, pos.z));

        //Lets limit it by "zone -1" to "zone +1"
        const double radiusCurClamped = std::max(std::min(radiusCur, radiusNext), radiusPrev);

        if (radiusCurClamped >= radiusMin && radiusCurClamped <= radiusMax)
        {
            //All in range.

            const double newPhase01u = std::floor(oldphase01u) + getValue01Clamped(radiusCurClamped, radiusMax, radiusMin);

            return { newPhase01u, pos };
        }

        //For smoother movement, we scale the distance travelled past the border?
        //It kinda makes sense but im not sure tbh:

        if (radiusCurClamped < radiusMin)
        {
            const double scaledDiff         = (radiusCurClamped - radiusMin) / scale;
            const double radiusCurCorrected = radiusMin + scaledDiff;

            const double phaseInZoneMinusOne = getValue01Clamped(radiusCurCorrected, radiusMin, radiusPrev);

            const double newPhase01u = std::floor(oldphase01u) + 1.0 + phaseInZoneMinusOne;

            return { newPhase01u, pos * float(scale) };
        }
        else // radiusCurClamped > radiusMax
        {
            const double scaledDiff         = (radiusCurClamped - radiusMax) * scale;
            const double radiusCurCorrected = radiusMax + scaledDiff;

            const double phaseInZonePlusOne = getValue01Clamped(radiusCurCorrected, radiusNext, radiusMax);

            const double newPhase01u = std::floor(oldphase01u) - 1.0 + phaseInZonePlusOne;

            return { newPhase01u, pos / float(scale) };
        }
    }

    static glm::vec3 applyPhase(glm::vec3 pos,
                                double    phase01u,
                                double    baseRadius,
                                double    scale)
    {
        //how this handles negative values is controversial
        const double phase01 = phase01u - floor(phase01u);

        const double radiusMax      = baseRadius * scale;    //at phase01 = 0
        const double radiusMin      = baseRadius;            //at phase01 = 1
        const double radiusNeeded   = mix(radiusMax, radiusMin, phase01);

        //now its guaranteed it will be in [radiusMin, radiusMax]
        std::tie(std::ignore, pos) = constrainPos(pos, phase01u, baseRadius, scale);

        const double radiusActual   = glm::length(pos);

        pos *= radiusNeeded / radiusActual;

        return pos;
    }
};