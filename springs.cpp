#include <iostream>
#include <vector>
#include <math.h>

#define PI 3.14159265

using namespace std;

/* Class def */
class Ball {

private:

public:
  Ball (double, double); // ctor
  Ball (double, double, int); //polar
  double dist2ball(Ball &);

  double x,y,z;
  double vx,vy;

  double T,U;

  double F;
  double Fx,Fy;
};

class Spring {

public:

  int n1,n2; //nodes index the balls that the spring is attached to

  Spring (int, int); //constructor
  double eql; //equilibrium length (F = 0)
  double Energy(vector<Ball> &);
  double Length();
};


/* adjustable parameters, globally defined */
double k = 2; 
double L = 1; 
double m = 1;
double dt = 0.05;
float tmax=100;

vector<Ball> ball_v;
vector<Spring> spring_v;



/* function def */
Ball::Ball (double x0, double y0) {

  x = x0;
  y = y0;
  z = 0;

  vx = 0;
  vy = 0;  

  F = 0; //to be retired
  Fx = 0;
  Fy = 0;
}

Ball::Ball (double r, double t, int hex) {

  if (!hex) {
    //cout << " but I should be ctor 1.. " << endl;
    Ball(r, t);

  } else {

    x = r * cos(t);
    y = r * sin(t);

    z = 0;
    vx = 0;
    vy = 0;  
    F = 0;
    Fx=0;
    Fy=0;
  }

}


double Ball::dist2ball(Ball &b) {

  double x,y,z;
  double dx,dy,dz;
  
  dx = b.x - x;
  dy = b.y - y;
  dz = b.z - z;

  double L;
  L = sqrt( dx*dx + dy*dy + dz*dz );

  return L;
}
//b1->dist2ball(b2);


Spring::Spring (int b1, int b2) {

  //eql = L;
  /* connect balls to nodes at construction */
  n1 = b1;
  n2 = b2;

}

double Spring::Length(){

  /* determines a spring's lengths from its two nodes */

  double dx, dy, L;
  
  Ball a = ball_v[n1];
  Ball b = ball_v[n2];

  dx = b.x - a.x;
  dy = b.y - a.y;

  L = sqrt( dx*dx + dy*dy );

  return L;
}

double Spring::Energy(vector<Ball> &v) {

  /* U = 1/2 k x*x */
  double dx = v[n1].x - v[n2].x;
  double dy = v[n1].y - v[n2].y;

  double mag = sqrt(dx*dx + dy*dy);

  double E = k/2 * (mag - eql) * (mag - eql);

  return E;
}
//maybe it would be better to store this as a force


void hexInit() {

  Ball a0(0.001,0,0);
  ball_v.push_back(a0);

  double sd = PI/3; //'Sixty Degrees'

  /* Can I automate this in a 2line for loop? */
  Ball a1(1, 0*sd, 1);
  Ball a2(1, 1*sd, 1);
  Ball a3(1, 2*sd, 1);
  Ball a4(1, 3*sd, 1);
  Ball a5(1, 4*sd, 1);
  Ball a6(1, 5*sd, 1);

  ball_v.push_back(a1);
  ball_v.push_back(a2);
  ball_v.push_back(a3);
  ball_v.push_back(a4);
  ball_v.push_back(a5);
  ball_v.push_back(a6);

  /* now for springs */
  Spring s01 = Spring(0,1);
  Spring s02 = Spring(0,2);
  Spring s03 = Spring(0,3);
  Spring s04 = Spring(0,4);
  Spring s05 = Spring(0,5);
  Spring s06 = Spring(0,6);

  Spring s12 = Spring(1,2);
  Spring s23 = Spring(2,3);
  Spring s34 = Spring(3,4);
  Spring s45 = Spring(4,5);
  Spring s56 = Spring(5,6);
  Spring s61 = Spring(6,1);

  spring_v.push_back(s01);
  spring_v.push_back(s02);
  spring_v.push_back(s03);
  spring_v.push_back(s04);
  spring_v.push_back(s05);
  spring_v.push_back(s06);
  spring_v.push_back(s12);
  spring_v.push_back(s23);
  spring_v.push_back(s34);
  spring_v.push_back(s45);
  spring_v.push_back(s56);
  spring_v.push_back(s61);

  for (int i=0; i<12; i++) {
    spring_v[i].eql = 0.95;
  } 

}


void initString() {

  /* Sets Initial Conditions
     we start w 1D spring for 1D forces*/

  Ball a1(0,0);
  Ball b0(1,0);
  Ball a2(3,0); 

  ball_v.push_back(a1);
  ball_v.push_back(b0);
  ball_v.push_back(a2);

  /* need to build springs */

  Spring s1 = Spring(0,1);
  Spring s2 = Spring(1,2);

  s1.eql = (a2.x - a1.x) / 2;
  s2.eql = (a2.x - a1.x) / 2;

  spring_v.push_back(s1);
  spring_v.push_back(s2);

}


/* this test script only moves one spring, 
   so I have something to practice rendering with*/
void updatePosition(Ball &b) {

  //compute a
  double ax = b.Fx / m;
  double ay = b.Fy / m;
  
  //update v
  b.vx += ax * dt;
  b.vy += ay * dt;

  //update x
  b.x += b.vx * dt;
  b.y += b.vy * dt;

  if (0) {
    cout << b.x << ", "
	 << b.vx << ", "
	 << ax << endl;

    cout << b.y << ", "
	 << b.vy << ", "
	 << ay << endl;
  }
}

/* loop all springs to compute force on all balls */
void ForceSprings() {

  Spring *spr;
  Ball *b1, *b2;

  int j1,j2; //vector index for the balls
  double L, X, F;
  double x1, x2, y1, y2;

  size_t N = spring_v.size();
  //cout << "N springs: " << N << endl;
  for (size_t j=0; j<N; j++) {

    spr = &spring_v[j];

    L = spr->Length();
    X = spr->eql;
    F = -k * (L - X); // magnitude 

    if (0) {
      cout << "Spring " << j << endl;
      cout << "(L, X, F): "
	   << L << ", "
	   << X << ", "
	   << F 
	   << endl;
    }

    /* 
       either both are attracted
       or both repelled
       No matter what there will be opposite sign
    */

    j1 = spr->n1;
    j2 = spr->n2;
    b1 = &ball_v[j1];
    b2 = &ball_v[j2];

    x1 = b1->x;
    x2 = b2->x;

    b1->Fx += F * (x1-x2) / L; 
    b2->Fx -= F * (x1-x2) / L;

    y1 = b1->y;
    y2 = b2->y;

    b1->Fy += F * (y1-y2) / L; 
    b2->Fy -= F * (y1-y2) / L;

    if (0) {
    cout << j1 << ", " << j2 << "\n"
	 << x1 << ", " << x2 << endl;
    printf("\n");    
    }
  }

}


/* Everything here should be called once per time step*/
void physics() {

  size_t N = ball_v.size(); 
  //cout << "N balls: " << N << endl;

  /* Zeros Forces */
  for (size_t j=0; j<N; j++) {
    ball_v[j].F  = 0;
    ball_v[j].Fx = 0;
    ball_v[j].Fy = 0;
  }

  /* add springs */
  ForceSprings();

  /* add brownian motion */

  /* loop particles */
  //cout << "(x, v, a)" << endl;
  for (size_t j=0; j<N; j++) {

    //cout << "Ball " << j << endl;
    updatePosition(ball_v[j]); 
  }

}

void writePositions(FILE* f) {

  int n = ball_v.size();
  for(int j=0; j<n; j++) {
    fprintf(f, "%f" , ball_v[j].x);
    fprintf(f, " %f", ball_v[j].y);
    fprintf(f, " %f", ball_v[j].z);
    fprintf(f, "\n");
  }

  fprintf(f, "\n"); //new timestep
}

void writeSprings(FILE* f) {

  Spring *spr;
  Ball *b1, *b2;
  int n = spring_v.size();
  float dx,dy,dz;
  float L;
  float nx,ny,nz;

  /* fetch information about position, direction, L */
  for(int j=0; j<n; j++) {

    spr = &spring_v.at(j);
    b1 = &ball_v.at(spr->n1);
    b2 = &ball_v.at(spr->n2);

    dx = b2->x - b1->x;
    dy = b2->y - b1->y;
    dz = b2->z - b1->z;

    L = sqrt( dx*dx + dy*dy + dz*dz );    

    nx = dx/L;
    ny = dy/L;
    nz = dz/L;
    
    fprintf(f, "%f" , b1->x);
    fprintf(f, " %f", b1->y);
    fprintf(f, " %f", b1->z);

    fprintf(f, " %f", dx);
    fprintf(f, " %f", dy);
    fprintf(f, " %f", dz);

    fprintf(f, " %f\n", L);
  }
  fprintf(f, "\n");
  /*
    float a[3], b[3], d[3], n[3];

    d = {b2->x - b1->x,
	 b2->y - b1->y,
	 b2->z - b1->z};

    for(int i=0; i<3; i++) {
      n[i] = d[i]/L;
    }

    //float d[3] = {x/L, y/L, z/L};
  */
}

/* what is the cost of not passing by ref? */
float distBall(Ball a, Ball b) {

  float x,y,z;
  x = b.x - a.x;
  y = b.y - a.y;
  z = b.z - a.z;

  float L;
  L = sqrt( x*x + y*y + z*z );

  return L;


  float d[3] = {x/L, y/L, z/L};
  // how to return this?
}


void show() {
  cout << "showing..." << endl;

  int n = ball_v.size();
  cout << n << endl;

  Ball* foo;
  for (int i=0; i<n; i++) {
    foo = &ball_v.at(i);

    cout << foo->x << " "
	 << foo->y << endl;
  }
  getc(stdin);
}


int main() {

  cout << "hello world" << endl;
  hexInit();
  //show();

  FILE *f1, *f2;
  f1 = fopen("newBalls.txt", "w");
  f2 = fopen("newSprings.txt", "w");

  int T, B, S; 
  T = (int)tmax/dt;
  B = ball_v.size();
  S = spring_v.size();

  fprintf(f1, "%i\n" , T);
  fprintf(f1, "%i\n\n" , B);
  fprintf(f2, "%i\n\n" , S);

  float t=0; int ts=0; 
  while (t<tmax) {

    physics();
    //getc(stdin);

    if (ts % 10 == 0) {
      writePositions(f1);
      writeSprings(f2);
    }
    t += dt;
    ts++;
  }

  fclose(f1);
  fclose(f2);

  return 0;
}
