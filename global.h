#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include "MersenneTwister.h"

#define PI 3.14159265

using namespace std;
string _version = "cytoskel2.0_1030";

/**************
  Control Box
 **************/

int _tEqGlobal = 5000; //time to delay before doing analysis
int _tEqLocal = 5000; //time to delay before making measurements

float _dt = 0.00001;  //time step physics

int _tSamp = 5000000;      //time pause rendering
int _nSteps = 300;
int _tMax = _tEqGlobal + (_tSamp * _nSteps);

int _nSys = 3;  //side length is Twice this #
int _nSpectrin = 10; //number of spectrin Springs between each actin
double _lActin = 1.2; //initial length between Actin
float _sRadius = _lActin / 32;
float _sigma = _sRadius * 2;
double _pRadius = 1.1 * _lActin; //particle radius
double _Contour = 2.5 * _lActin;

double _m = 1;
double _k = 20; 
double _gamma = 1;
double _D = 1;
double _epsilon = 1;
float _dz = -0.013;

bool _Particle = true;
bool _ankyrin = true;
bool _printTime = true;
bool _fileTag = false;
bool _sunrise = false;
string _tag = "";


/* Class def */
class Ball {

public:
  Ball (double, double, double); // constructor
  Ball (double, double, int); //polar coordinate constructor (obselete)
  double r[3],v[3],F[3]; //position, velocity, force (all 3 arrays)

  double m,R; //mass, radius
  int idx, pid; //index in vector, particle id as defined in initPID()
  bool isEdge;  //selects for boundary considerations
  int edgeType; //as defined in edgeType()

  double getForce(); //returns total F on Ball at given time
};

class Spring {

public:
  Spring (int, int); //constructor

  int n1,n2; //nodes: index the balls attached to this brings

  double L, k;  //length, spring constant, mass
  double eql; //equilibrium length (F = 0)

  double getLength();
  double getEnergy();
};

class Elem {
 
public:
  Elem(int,int,int);

  int b1,b2,b3;
  int idx;
};


/* this guy is very similar to the spring..  */
class Edge {

public:
  Edge (int, int); //constructor

  int n1,n2; //nodes: index the balls attached to this brings

  int e1,e2; //the unique two adjacent elements

  double L;
  double getLength();
};


/* Global Declaration */
MTRand randi;

int nBalls, nSprings, nTime; 

FILE *f1, *f2, *f3, *f4, *f5, *f6, *f7;

Ball* Particle = 0;
vector<Ball> v_balls;
vector<Spring> v_springs;
vector<Elem> v_elems;
vector<Edge> v_edges;

vector<float> conTime;
vector<float> zStats;
vector<float> zPlus;
vector<float> zMinus;

vector< vector<float> > forceST(3);
vector< pair<float,float> > forceTime; //(z,Fz)_t


bool _samp;
bool localEq(int);

void init();
void meshInit();
void springInit();
void spectrinInit(int);
void elemInit();

void initParticle();
void initPID();
void filesInit();
void filesClose();
void moveParticle();

void physics();
void timeStep();
void doAnalysis();
void ForceSprings();
void updatePosition(Ball &);
void updateBrownian(Ball &);

void SurfaceForce();
double LJforce(double,double);
double zSurface(double, double);

float calcMSD(float**);
float calcMSDx(float*);

float distBall(Ball*, Ball*);
vector<float> normBall(Ball*, Ball*);
bool isEdge(int, int, int);
int edgeType(int, int, int);

pair<float,float> doStats(vector<float> &v);
void writeForceZ(FILE *f);

void writeBalls(FILE*);
void writeSprings(FILE*);
void measureEdge(FILE*);
void measureSpringEnergy();
void measureContour(FILE*);
void sampleForceZ();
void sampleForce3D();
void show();


/* function def */
Ball::Ball (double x, double y, double z) {

  for(int i=0; i<3; i++) {
    r[i]=0;
    v[i]=0;
    F[i]=0;
  }

  r[0] = x;
  r[1] = y;
  r[2] = z;

  idx = v_balls.size();
  pid = 1;
  isEdge = false;

  m = _m;
}


/* returns magnitude of Force */
double Ball::getForce() {

  double f = 0;

  for(int j=0; j<3; j++) {
    f += F[j]*F[j];
  }
  f = sqrt(f);

  return f;
}

Spring::Spring (int b1, int b2) {

  /* connect balls to nodes at construction */
  n1 = b1;
  n2 = b2;

  /* sets equilibrium length 
     as some fraction f of initial length */
  float feq = 1;
  L = getLength();
  eql = feq * L; 
 
  k = _k;
}

/* determines a spring's lengths from its two nodes */
double Spring::getLength(){

  Ball a = v_balls[n1];
  Ball b = v_balls[n2];

  double L = 0;
  double dr[3];

  for (int i=0; i<3; i++) {
    dr[i] = b.r[i] - a.r[i];

    L += dr[i]*dr[i];
  }
  L = sqrt(L);

  return L;
}

double Spring::getEnergy() {

  double E, L;
  L = getLength();
  E = k/2 * (L - eql) * (L - eql);

  return E;
}

Elem::Elem(int j1, int j2, int j3) {

  b1 = j1;
  b2 = j2;
  b3 = j3;

  idx = v_elems.size();

}

Edge::Edge (int b1, int b2) {

  /* connect balls to nodes at construction */
  n1 = b1;
  n2 = b2;

  L = getLength();
}

double Edge::getLength(){
  
  Ball a = v_balls[n1];
  Ball b = v_balls[n2];

  double dr[3];
  double L = 0;
  for (int i=0; i<3; i++) {
    dr[i] = b.r[i] - a.r[i];

    L += dr[i]*dr[i];
  }
  L = sqrt(L);

  return L;
}

bool isEdge(int i, int j, int N) {

  //returns true if corner
  bool foo = false;
  int beg = 0;
  int end = 2*N-1;

  if (i==end || j==beg) foo = true;
  if (i==beg || j==end) foo = true;

  if (i==beg || j==beg) foo = true;
  if (i==end || j==end) foo = true;

  return foo;
}

int edgeType(int i, int j, int N) {

  int beg = 0;
  int end = 2*N-1;

  if (i==end && j==beg) return 0;
  if (i==beg && j==end) return 0;
  if (i==beg && j==beg) return 0;
  if (i==end && j==end) return 0;

  //isn't corner
  if (j==beg || j==end) return 1; //horizontal
  if (i==beg || i==end) return 2; //vertical
  
  cout << "not edge?" << endl;
  getc(stdin);
  return -1;
}

double _pow(double x, int n) {

  double ans = 1;
  for(int i=0; i<n; i++) {

    ans = ans*x;
  }
  return ans;
}
