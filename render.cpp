#include "gr.h"
#include "gr3.h"

#include "stdio.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

/* User Controls */
bool _mov = false;    //writes movie to file
bool _tracer = false; //follows origin

bool _cut = false;    //cuts animation length 
int _tcut = 120;      //  after _tcut frames
/*~~~~~~~~~~~~~~~*/

int main() {

  cout << "begin rendering.." << endl;
  double num;

  /* Read in Balls */

  ifstream dataBalls("balls.dat", ios::in);

  int T,N,X;
  dataBalls >> T;
  dataBalls >> N;

  X = 3*N;
  cout << T << " " << X << " " << N << endl;
  float in[T][X];
  float colors[X];  
  float radii[N];
  float *positions;

  // col & rad
  int pid;
  for(int i=0; i<N; i++) {

    dataBalls >> pid;
    /* Particle ID
       0: spectrin, white
       1: anchor, red
       2: edge, blue
       3: ankyrin, orange
    */
    if (pid == 0) {
      colors[3*i + 0] = 1;
      colors[3*i + 1] = 1;
      colors[3*i + 2] = 1;
      radii[i] = 0.05;
    } else if (pid == 2)  
    {
      colors[3*i + 0] = 0;
      colors[3*i + 1] = 0;
      colors[3*i + 2] = 1;
      radii[i] = 0.18;
    } else if (pid == 3)  
    {
      colors[3*i + 0] = 1;
      colors[3*i + 1] = 0.5;
      colors[3*i + 2] = 0;
      radii[i] = 0.1;
    } else {
      colors[3*i + 0] = 1;
      colors[3*i + 1] = 0;
      colors[3*i + 2] = 0;
      radii[i] = 0.18;
    }

  }

  /* toggle to turn Ball 0 yellow */
  if (_tracer) {
    colors[0] = 1;
    colors[1] = 1;
    colors[2] = 0;
  }

  // position
  for(int t=0; t<T; t++) {
    for(int x=0; x<X; x++) {

      dataBalls >> in[t][x];
    }
  }
  cout << " finished reading balls" << endl;


  /* Read in Springs */
  ifstream dataSprings("springs.dat", ios::in);
  int S,Y,V;
  dataSprings >> S;
  Y = 7*S; // data * n springs
  V = 3*S; // vect * n springs
  float in2[T][Y];
  for(int t=0; t<T; t++) {
    for(int y=0; y<Y; y++) {
      dataSprings >> num;
      in2[t][y] = num;
    }
  }

  float inX[T][V], inD[T][V], inL[T][S];
  for(int t=0; t<T; t++) {
    for(int s=0; s<S; s++) {
   
      inX[t][3*s + 0] = in2[t][7*s + 0];
      inX[t][3*s + 1] = in2[t][7*s + 1];
      inX[t][3*s + 2] = in2[t][7*s + 2];

      inD[t][3*s + 0] = in2[t][7*s + 3];
      inD[t][3*s + 1] = in2[t][7*s + 4];
      inD[t][3*s + 2] = in2[t][7*s + 5];

      inL[t][s] = in2[t][7*s + 6];
    }
  } //maybe there is a more elegant way to do this, or is this very elegant?

  float *cyl_pos;
  float *cyl_dir;
  float *cyl_len;

  float cyl_col[V];
  float cyl_rad[S];
  for(int j=0; j<S; j++) {
    cyl_col[3*j + 0] = 1;
    cyl_col[3*j + 1] = 1;
    cyl_col[3*j + 2] = 1;

    cyl_rad[j] = 0.05;
  }
  cout << " finished reading springs" << endl;


  /* Animate Video */
  if(_mov) {
    cout << "* Writing to .mov!" << endl;
    setenv("GKS_WSTYPE", "mov", 1); 
  }

  int tend = T-1;
  if(_cut) tend = _tcut;
  cout << " projected tend:  " << tend << endl;

  cout << "Begin looping.. " << endl;
  for(int t=0; t<tend; t++) {

    positions = in[t];
    cyl_pos = inX[t];
    cyl_dir = inD[t];
    cyl_len = inL[t];
    
    gr3_clear();
    gr3_drawspheremesh(N, positions, colors, radii);
    gr3_drawcylindermesh(S, cyl_pos, cyl_dir, cyl_col, cyl_rad, cyl_len);

    gr_clearws();
    gr_setwsviewport(0, 0.12, 0, 0.12); // in units m
    gr_setviewport(0, 1, 0, 1);         // relative to Work Station

    gr3_drawimage(0, 1, 0, 1, 800, 800, GR3_DRAWABLE_GKS);
    gr_updatews();    

    cerr << t << "\r";
  }
  cout << "finished loop" << endl;

  if (_mov) gr_emergencyclosegks();
  cout << "rendering complete! press any key <> to contiune" << endl;

  getc(stdin);
  return 0;
}
