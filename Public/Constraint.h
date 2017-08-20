#ifndef OC_CONSTRAINT_H
#define OC_CONSTRAINT_H

#include <float.h>
#include "Vector3.h"
#include "Transform.h"
#include "Logger.h"

struct Constraint
{
    //for now just a simple distance constraint. This is super hacky
    RigidBody* body1;
    RigidBody* body2;
    Transform frame1;
    Transform frame2;

    float invMassScale1;
    float invMassScale2;
    float invInertiaScale1;
    float invInertiaScale2;
    float linearProjection;
    float distance;

    //prepared data
    Vector3 normal;
    Vector3 r1;
    Vector3 r2;
    float geometricError;
    float weight1;
    float weight2;
    float invMass1;
    float invMass2;

    Logger* logger;
    std::string loggerKey;

    Constraint(RigidBody* inBody1, const Transform& localTM1, RigidBody* inBody2, const Transform& localTM2)
        : body1(inBody1)
        , body2(inBody2)
        , invMassScale1(1.f)
        , invMassScale2(1.f)
        , invInertiaScale1(1.f)
        , invInertiaScale2(1.f)
        , linearProjection(FLT_MAX)
        , logger(nullptr)
    {
        frame1 = localTM1;
        frame2 = localTM2;

        distance = ((body1->bodyToWorld * frame1).translation - (body2->bodyToWorld * frame2).translation).length();
    }

    virtual void prepareConstraint()
    {
        invMass1 = body1->invMass * invMassScale1;
        invMass2 = body2->invMass * invMassScale2;

        if(invMass1 > OC_BIG_EPSILON || invMass2 > OC_BIG_EPSILON)
        {
            const float totalMass = 1.f / (invMass1 + invMass2);

            weight1 = totalMass * invMass1;
            weight2 = totalMass * invMass2;

            const Transform& body1TM = body1->bodyToWorld;
            const Transform p1TM = body1TM * frame1;
            const Transform& body2TM = body2->bodyToWorld;
            const Transform p2TM = body2TM * frame2;


            const Vector3 n = p2TM.translation - p1TM.translation;
            normal = n.getSafeNormal();
            const float nLength = n.length();
            geometricError = distance - nLength;

             r1 = p1TM.translation - body1TM.translation;
             r2 = p2TM.translation - body2TM.translation;
        }

    }

    void solveConstraint(float invDeltaTime)
    {
        if (invMass1 > OC_BIG_EPSILON || invMass2 > OC_BIG_EPSILON)
        {
            if(logger)
            {
                logger->log(loggerKey, std::string("linearError"), geometricError);
            }
            //const Vector3 directedDistance = p2ToP1Normal * (linearError);
            /*if(fabs(linearError) > linearProjection)
            {
                //todo: actually sort by kinematics properly
                //body1->bodyToWorld.translation += -p2ToP1Normal * weight1 * linearError;
                //body2->bodyToWorld.translation += p2ToP1Normal * weight2 * linearError;
            }
            else*/
            {
                const Vector3 p1WorldVel = body1->linearVelocity + Vector3::crossProduct(body1->angularVelocity, r1);
                const Vector3 p2WorldVel = body2->linearVelocity + Vector3::crossProduct(body2->angularVelocity, r2);
                const float relVelocity = Vector3::dotProduct(p2WorldVel - p1WorldVel, normal);

                if (logger)
                {
                    logger->log(loggerKey, std::string("relVelocity"), relVelocity);
                }

                const float r2squaredMinusR2Dot = r2.length2()*weight2 - r2.dotProduct(normal*weight2)*r2.dotProduct(normal);
                const float r1squaredMinusR1Dot = r1.length2()*weight1 - r1.dotProduct(normal*weight1)*r1.dotProduct(normal);
                const float lambda = -relVelocity / (1.f + r2squaredMinusR2Dot + r1squaredMinusR1Dot);
                const Vector3 correctionImpulse = (lambda + geometricError * invDeltaTime * 0.05f) * normal;

                if (logger)
                {
                    logger->log(loggerKey, std::string("lambda"), -lambda);
                }

                if(invMass1 > 0)
                {
                    const Vector3 b1LinearImpulse = -correctionImpulse * weight1 * (1.f/ invMass1);  //gross, compute better above
                    body1->linearVelocity += b1LinearImpulse * invMass1;
                    body1->angularVelocity += Vector3::crossProduct(r1, b1LinearImpulse) * body1->invInertia.x;    //terrible assumes completely symmetric
                }
                
                if(invMass2 > 0)
                {
                    const Vector3 b2LinearImpulse = correctionImpulse * weight2 * (1.f/ invMass2);
                    body2->linearVelocity += b2LinearImpulse * invMass2;
                    body2->angularVelocity += Vector3::crossProduct(r2, b2LinearImpulse) * body2->invInertia.x;   //terrible assumes completely symmetric
                }
            }
        }
    }
};

#endif
