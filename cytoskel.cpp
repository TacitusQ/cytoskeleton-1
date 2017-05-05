#include "global.h"
#include "init.h"
#include "physics.h"
#include "analysis.h"

#include "inputModule.h"

int main() {

  cout << "hello world!" << endl;

  init();
  physics();
  filesClose();

  return 0;
}

void init() {

  /* Read input, Write Log, Set Output, Seed Rand */
  readInput();
  printLog();
  randInit();

  /* Build Cytoskeleton System */
  cout << " initializing system.." << endl;
  meshInit();
  spectrinInit(_nSpectrin);
  initParticle();
  fileInit();

}

void physics() {

  /* Defined in Header:
     tmax - (float) absolute stop time
     _dt - (float) time step physics
     _tSamp - (int) time step output */

  cout << " beginning physics.." << endl;

  float T=0; //elapsed time
  int t=0, t_count=0;

  while (t < _tMax) {

    _samp = localEq(t);

    timeStep();
    t++;
    T += _dt;

    //init equilibriation
    if (t < _tEqGlobal) continue;

    if (t % _tSamp == 0) { 

      t_count++;
      doAnalysis();

      if (_printTime) {
	cerr << t_count << "\r";
      }
    }
  }

  cout << "   rendered time steps: " << t_count << endl;
  cout << "   final system time : " << T << endl;
  
  integrateWrappingEnergy();

  cout << " finished physics.." << endl;
}


/* Everything here called once per Physics time step */
void timeStep() {

  size_t N = nBalls;

  /* Zeros Forces */
  for(int i=0; i<3; i++) {
    for (size_t j=0; j<N; j++) {
      v_balls[j].F[i] = 0;
    }
    if(_Particle) Particle->F[i] = 0;
  }

  /* Tally Forces */
  ForceSprings();
  SurfaceForce();
  ParticleInteraction();


  /* Update Particles */
  for (size_t j=0; j<N; j++) {

    if (v_balls[j].isEdge) continue; 
    //updatePosition(v_balls[j]); 
    updateBrownian(v_balls[j]); 
  }


}

/* this function is called once every (_tSamp) */
void doAnalysis() {
 
  /* write for rendering */
  writeBalls(f1);
  writeSprings(f2);

  if (!_Particle) return;

  /* make measurements */
  writeForceZ(f6); //mean Fz discovered in here!
  //  writeForce3D();

  /* move Particle */
  Particle->r[2] += _dz; 
  
}
 
