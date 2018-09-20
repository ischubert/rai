#include "bounds.h"

void skeleton2Bound(KOMO& komo, BoundType boundType, const Skeleton& S, const rai::KinematicWorld& startKinematics, const rai::KinematicWorld& parentKinematics, bool collisions){
  double maxPhase=0;
  for(const SkeletonEntry& s:S) if(s.phase1>maxPhase) maxPhase=s.phase1;
  //-- prepare the komo problem
  switch(boundType) {
    case 1: {
      //-- grep only the latest entries in the skeleton
      Skeleton finalS;
      for(const SkeletonEntry& s:S) if(s.phase1>=maxPhase){
        finalS.append(s);
        finalS.last().phase0 -= maxPhase-1.;
        finalS.last().phase1 -= maxPhase-1.;
      }

      komo.setModel(parentKinematics, collisions);
      komo.setTiming(1., 1, 5., 1);

      komo.setHoming(0., -1., 1e-2);
      komo.setSquaredQVelocities(.5, -1., 1.); //IMPORTANT: do not penalize transitions of from prefix to x_{0} -> x_{0} is 'loose'
      //komo.setFixEffectiveJoints(0., -1., 1e2); //IMPORTANT: assume ALL eff to be articulated; problem: no constraints (touch)
//      komo.setFixSwitchedObjects(0., -1., 1e2);
      komo.setSquaredQuaternionNorms();

      komo.setSkeleton(finalS);

      if(collisions) komo.add_collision(false);

      komo.reset();
//      komo.setPairedTimes();
//      cout <<komo.getPath_times() <<endl;
    } break;
    case 2: {
      komo.setModel(startKinematics, collisions);
      maxPhase += 1. ;
      if(maxPhase>1e-2) komo.setTiming(maxPhase, 1, 5., 1);
      else  komo.setTiming(1., 1, 5., 1);

      komo.setHoming(0., -1., 1e-2);
      komo.setSquaredQVelocities(0., -1., 1e-2);
//      komo.setFixEffectiveJoints(0., -1., 1e2);
//      komo.setFixSwitchedObjects(0., -1., 1e2);
      komo.setSquaredQuaternionNorms();

      komo.setSkeleton(S);

      if(collisions) komo.add_collision(true, 0., 1e1);

      komo.reset();
//      komo.setPairedTimes();
      //      cout <<komo.getPath_times() <<endl;
    } break;
    case 3: {
      komo.setModel(startKinematics, collisions);
      uint stepsPerPhase = rai::getParameter<uint>("LGP/stepsPerPhase", 10);
      uint pathOrder = rai::getParameter<uint>("LGP/pathOrder", 2);
      komo.setTiming(maxPhase+.5, stepsPerPhase, 5., pathOrder);

      komo.setHoming(0., -1., 1e-2);
      if(pathOrder==1) komo.setSquaredQVelocities();
      else komo.setSquaredQAccelerations();
//      komo.setFixEffectiveJoints(0., -1., 1e2);
//      komo.setFixSwitchedObjects(0., -1., 1e2);
      komo.setSquaredQuaternionNorms();

      komo.setSkeleton(S);

      if(collisions) komo.add_collision(true, 0, 1e1);

      komo.reset();
      //      cout <<komo.getPath_times() <<endl;
    } break;
    default: NIY;
  }
}