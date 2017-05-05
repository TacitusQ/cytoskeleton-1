/*
  This file contains the following Physics
    distBall(Ball*, Ball*)
    normBall(Ball*, Ball*)
    magForce(Ball*)

    ForceSprings()
    SurfaceForce()
    updateBrownian(Ball &)
    updatePosition(all &)
    LJforce(r, sig)
    zSurface(x, y)

    ParticleInteraction()
 */

float distBall(Ball *b1, Ball *b2) {

  float L = 0;
  float r1[3],r2[3],dr[3];
  for (int i=0; i<3; i++) {

    r1[i] = b1->r[i];
    r2[i] = b2->r[i];

    dr[i] = r2[i] - r1[i];

    L += dr[i]*dr[i];
  }
  L = sqrt(L);
  
  return L;
}

//returns norm from origin of a to origin of b
vector<float> normBall(Ball *a, Ball *b) {

  /* get distance */
  float u[3],v[3],n[3];
  for (int i=0;i<3;i++) {
    u[i] = a->r[i];
    v[i] = b->r[i];

    n[i] = v[i] - u[i];
  }

  /* get magnitude */
  float m=0;
  for(int j=0; j<3; j++) {
    m += n[j]*n[j];
  }
  m = sqrt(m);

  /* create norm */
  vector<float> norm = vector<float>(3);
  for(int j=0; j<3; j++) {
    norm[j] = n[j]/m;
  }

  return norm;
}

/* have fx fy fz, wish to calc magnitude */
double magForce(Ball *b) {

  double F=0, f=0;
  for (int i=0; i<3; i++) {
    f = b->F[i];
    F += f*f;
  }
  return sqrt(F);
}


/* loop all springs to compute force on all balls */
void ForceSprings() {

  Spring *spr;
  Ball *b1, *b2;

  int j1,j2; //vector index for the balls
  double L, X, K, F;
  double u[3], v[3], dr[3];

  size_t N = v_springs.size();
  for (size_t j=0; j<N; j++) {

    /* computes force */
    spr = &v_springs[j];

    L = spr->getLength();
    X = spr->eql;
    K = spr->k;
    F = - K * (L - X);

    /* assigns force to balls */
    j1 = spr->n1;
    j2 = spr->n2;
    b1 = &v_balls[j1];
    b2 = &v_balls[j2];

    for(int i=0; i<3; i++) {
      u[i] = b1->r[i];
      v[i] = b2->r[i];

      dr[i] = u[i]-v[i];
      b1->F[i] += F * dr[i] / L;
      b2->F[i] -= F * dr[i] / L;
    }

  }
}

/* repels spectrin from surface, tethers actin to surface*/
void SurfaceForce() {
  int N = nBalls;
  Ball *b;

  bool inMembrane = false;
  for (size_t j=0; j<N; j++) {
    b = & v_balls[j];

    if (b->pid == 1) inMembrane = true;
    if (b->pid == 3) inMembrane = true;

    if (inMembrane) {
      b->r[2] = zSurface(b->r[0], b->r[1]); 
      continue;
    } 

    //else: is spectrin
    double x,y,z,z0;  
    double r,F;
    x = b->r[0];
    y = b->r[1];
    z = b->r[2]; //should be negative
    z0= zSurface(x,y);    

    if (z > z0) {
      cout << "Warning: z > z0!" << endl;
    }

    r = abs(z - z0);
    F = LJforce(r,_sigma); 
    b->F[2] += -F; //sign?
  }
}


void updateBrownian(Ball &b) {

  //double D = 0.01;
  double D = _D;
  double g = _gamma;

  for (int i=0; i<3; i++) {

    if (i==2) {
      if (b.pid==1 || b.pid==3) continue;
    }

    b.r[i] += b.F[i]/g * _dt;
    b.r[i] += sqrt(2*D*_dt) * randi.randNorm(0,1); 
  }
}

void updatePosition(Ball &b) {

  double a[3];
  double m = b.m;

  for (int i=0; i<3; i++) {
    a[i] = b.F[i]/m;

    b.v[i] += a[i] * _dt;
    b.r[i] += b.v[i] * _dt;
  }
}


/* computes MAGNITUDE of LJ force
   for origin separation r, and particle thickness (sum of radii) d*/
double LJforce(double r, double d) {

  /* r_min : F = 0, truncate potential
     sigma : U = 0, is const for all, 
     but it should be the sum of radii d = (a+b) */

  double F, e, m, p;
  e = _epsilon;
  m = 1.1225 * _sigma; //r_min
  r = r - d + _sigma;

  if (r > m) return 0; //yes, U(r') = 0

  /* F = 12 e/r [p12 - p6]  
      where p = r_min/r */
  p = m/r;
  if (1/p < 0.05) 
    cout << "Warning: LJ Singularity from small r" << endl;

  p = _pow(p,6);
  F = 12.*(e/r)*p*(p - 1);

  return F;
}


/* returns Membrane z(x,y) */
double zSurface(double x, double y) {

  return 0.1;   //temp flat
}



/* played at each Physics step */
void ParticleInteraction() {

  if(!_Particle) {
    return;
  }

  double radius = Particle->R;
  double r,d,F;
  vector<float> nm; //normal

  float zp = 0; 
  float zm = 0;

  Ball *b;

  int count = 0;
  int N = nBalls;
  for (int j=0; j<N; j++) {
    
    b = & v_balls.at(j);

    r = distBall(b, Particle);
    nm = normBall(b,Particle); // points b->P

    if ( r < radius ) 
    {

      F = -5; //F- rides the nm from P->b
      for (int i=0; i<3; i++) {
	b->F[i] += F * nm[i];
      }

    } else { 

      if (r > 1.5 * radius) continue;

      count++;
      d = radius + (_sigma/2); // R_particle + R_interactant
      F = LJforce(r,d);

      for (int i=0; i<3; i++) {
	b->F[i]        -= F *  nm[i];
	Particle->F[i] += F *  nm[i];
      }


      // NEW
      float fz = F * nm[2];
      if ( fz > 0 ) zp += fz;
      if ( fz < 0 ) zm += fz;

    }

  } //end of interactant loop  


  if (_samp) {
    float z = Particle->F[2];
    zStats.push_back(z); 

    zPlus.push_back(zp);
    zMinus.push_back(zm);
  }

}
