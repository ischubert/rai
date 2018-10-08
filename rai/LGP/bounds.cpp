#include "bounds.h"
//#include <Kin/switch.h>

void skeleton2Bound(KOMO& komo, BoundType boundType, const Skeleton& S, const rai::KinematicWorld& startKinematics, const rai::KinematicWorld& effKinematics, bool collisions, const arrA& waypoints){
  double maxPhase=0;
  for(const SkeletonEntry& s:S) if(s.phase1>maxPhase) maxPhase=s.phase1;
  //-- prepare the komo problem
  switch(boundType) {
    case BD_pose: {
      //-- make the effective kinematics
//      KOMO tmp;
//      tmp.setModel(startKinematics, false);
//      tmp.setSkeleton(S);
//      for(rai::KinematicSwitch *s : tmp.switches){
//        if(s->timeOfApplication<maxPhase) s->apply(tmp.world);
//      }

      //-- grep only the latest entries in the skeleton
      Skeleton finalS;
      for(const SkeletonEntry& s:S) if(s.phase0>=maxPhase){
        finalS.append(s);
        finalS.last().phase0 -= maxPhase-1.;
        finalS.last().phase1 -= maxPhase-1.;
      }

      komo.setModel(effKinematics, collisions);
      komo.setTiming(1., 1, 10., 1);

      komo.setHoming(0., -1., 1e-2);
      komo.setSquaredQVelocities(1., -1., 1e-1); //IMPORTANT: do not penalize transitions of from prefix to x_{0} -> x_{0} is 'loose'
      komo.setSquaredQuaternionNorms();

      komo.setSkeleton(finalS, false);

      if(collisions) komo.add_collision(false);

      komo.reset();
//      komo.setPairedTimes();
    } break;
    case BD_seq: {
      komo.setModel(startKinematics, collisions);
      komo.setTiming(maxPhase+1., 1, 5., 1);

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
    case BD_path: {
      komo.setModel(startKinematics, collisions);
      uint stepsPerPhase = rai::getParameter<uint>("LGP/stepsPerPhase", 10);
      uint pathOrder = rai::getParameter<uint>("LGP/pathOrder", 2);
      komo.setTiming(maxPhase+.5, stepsPerPhase, 10., pathOrder);

      komo.setHoming(0., -1., 1e-2);
      if(pathOrder==1) komo.setSquaredQVelocities();
      else komo.setSquaredQAccelerations();
      komo.setSquaredQuaternionNorms();

      komo.setSkeleton(S);

      if(collisions) komo.add_collision(true, 0, 1e1);

      komo.reset();
      //      cout <<komo.getPath_times() <<endl;
    } break;
    case BD_seqPath: {
      komo.setModel(startKinematics, collisions);
      uint stepsPerPhase = rai::getParameter<uint>("LGP/stepsPerPhase", 10);
      uint pathOrder = rai::getParameter<uint>("LGP/pathOrder", 2);
      komo.setTiming(maxPhase+.1, stepsPerPhase, 10., pathOrder);

      komo.setHoming(0., -1., 1e-2);
      if(pathOrder==1) komo.setSquaredQVelocities();
      else komo.setSquaredQAccelerations();
      komo.setSquaredQuaternionNorms();

      CHECK_EQ(waypoints.N-1, floor(maxPhase+.5), "");
      for(uint i=0;i<waypoints.N-1;i++){
        komo.addObjective(ARR(double(i+1)), OT_sos, FS_qItself, {}, 1e-1, waypoints(i));
      }

      uint O = komo.objectives.N;
      komo.setSkeleton(S);
      //delete all added objectives! -> only keep switches
//      for(uint i=O; i<komo.objectives.N; i++) delete komo.objectives(i);
//      komo.objectives.resizeCopy(O);

      if(collisions) komo.add_collision(true, 0, 1e1);

      komo.reset();
      komo.initWithWaypoints(waypoints);
      //      cout <<komo.getPath_times() <<endl;
    } break;

    default: NIY;
  }
}
