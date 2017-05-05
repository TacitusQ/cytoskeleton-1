/*
  This file contains the following Initialization functions
    meshInit()
    spectrinInit()
    initParticle()
    *elemInit()
    *edgeInit()

    fileInit()
    fileClose()
    initPID()
    randInit()

  and the retired functions as well
    hexInit()
    toyInit()
 */

void meshInit() {

  int N=_nSys;
  double L = _lActin;
  double h = sqrt(3)*L/2;

  double a = -(N - 1/2.);
  double x0 = L*a;
  double y0 = h*a;
  double z0 = 0;


  /* Build Balls */
  Ball *b;
  for (int y=0; y<2*N; y++) {
    for (int x=0; x<2*N; x++) {

      if (y%2 == 0) //even
	{ b = new Ball(x0 + x*L, y0 + y*h, z0); }    
      else //odd
	{ b = new Ball(x0+L/2 + x*L, y0 + y*h, z0); }

      if (isEdge(x,y,N)) {
	b->isEdge = true; 
	b->pid=2;

	b->edgeType = edgeType(x,y,N);
      }

      v_balls.push_back(*b);
    } //rows done.
  } //column and rows done

  springInit();

  cout << "   mesh complete" << endl;
}


void springInit() {

  /* Build Springs */
  int j;
  int N = _nSys;
  int n = 2*N;
  for (int y=0; y<n; y++) {
    for (int x=0; x<n; x++) {

      j = n*y + x; //computes ball index in vector

      if (x != n-1) {
	Spring s(j,j+1);
	v_springs.push_back(s);
      }

      if (y == n-1) continue;
      Spring s1(j, j+n);
      v_springs.push_back(s1);

      if (y%2 == 0) { /* y even */
	if (x == 0) continue;
	Spring s0(j, j+n - 1);
	v_springs.push_back(s0);

      } else { /* y odd */
	if (x == n-1) continue;
	Spring s0(j, j+n + 1);
	v_springs.push_back(s0);
      }
      
    }
  }
  cout << "   proto-spring complete" << endl;
}


/* between mesh and spectrin, loop through existing balls 
   and write nodes 

   e1 and e0 stand for elemUP and elemDOWN respectively,
  where an element is the triangular surface element connecting 3 balls
*/
void elemInit() {

  Elem *e0, *e1;

  /* borrowed from Build Springs */
  int j;
  int N = _nSys;
  int n = 2*N;
  for (int y=0; y<n; y++) {
    for (int x=0; x<n; x++) {

      if (x%n == 0) continue;
      j = y*n + x;

      if (y%2 == 0) {
	e0 = new Elem(j,j+1,j+1-n);

	if (y==n) continue;

	e1 = new Elem(j,j+1,j+1+n);

      } else {
	e1 = new Elem(j,j+1,j+n);

	if (y==0) continue;
	e0 = new Elem(j,j+1,j-n);
      }

      if (e0) v_elems.push_back(*e0);
      if (e1) v_elems.push_back(*e1);
    }
    
  }

}


/* Build Edges, same basic algorithm as initSprings */
void edgeInit() {

  int j;
  int N = _nSys;
  int n = 2*N;
  for (int y=0; y<n; y++) {
    for (int x=0; x<n; x++) {

      j = n*y + x;

      if (x != n-1) {
	Edge e(j,j+1);
	v_edges.push_back(e);
      }

      if (y == n-1) continue;
      Edge e1(j, j+n);
      v_edges.push_back(e1);

      if (y%2 == 0) { /* y even */
	if (x == 0) continue;
	Edge e0(j, j+n - 1);
	v_edges.push_back(e0);

      } else { /* y odd */
	if (x == n-1) continue;
	Edge e0(j, j+n + 1);
	v_edges.push_back(e0);
      }
      
    }
  }
  /* still need to identify two adjacent elems!! */
  cout << "edge init complete" << endl;
}

/* Mesh is Done. 
   For each protoSpring: Create and replace with Spectrin 'Dagger' */
void spectrinInit(int n) {

  //makes a copy
  int N = v_springs.size();
  if (N < 1) {
    cout << "Error: spring vector is empty!" << endl;
    getc(stdin);
  }

  Spring *spr;
  Ball *b1, *b2;

  double r1[2],r2[2],dr[2]; //lives in 2D plane
  double x,y,z;

  double A = _lActin;
  double C = _Contour;
  double h = sqrt(C*C + A*A) / 2; //creates dagger strings

  vector<Spring> newSpring_v;
  //for each spring
  int j1,j2;
  for (int j=0; j<N; j++) {
    spr = &v_springs.at(j);

    j1 = spr->n1; 
    j2 = spr->n2; 

    b1 = &v_balls.at(j1);
    b2 = &v_balls.at(j2);

    for (int i=0; i<2; i++) {
      r1[i] = b1->r[i];
      r2[i] = b2->r[i];
      dr[i] = (r2[i] - r1[i]) / n; //slice into chunks
    }

    //create n-1 new balls
    int k = v_balls.size(); //used later
    double a = A/2;
    for (int i=1; i<n; i++) {
      x = r1[0] + i*dr[0];
      y = r1[1] + i*dr[1];
      z = -h + h/a * abs( a - (i*A/n) );

      Ball b(x,y,z);
      b.pid = 0; //denotes spectrin
      if (_ankyrin) {
	if (i==n/2) b.pid = 3; // new ankyrin
      }
      v_balls.push_back(b);
    }

    //connect w N new springs (+2 old balls)
    if (n == 1) return;
    newSpring_v.push_back( Spring(j1,k) ); 
    for (int i=1; i<n-1; i++) 
    {
      newSpring_v.push_back( Spring(k, k+1) );
      k++;
    }
    newSpring_v.push_back( Spring(k, j2) );

  }//Springs done.

  //haben Sie memory leak??
  v_springs = newSpring_v;

  cout << "   spectrin spring complete " << n << endl;
       
}


void initParticle() {

  if(!_Particle) {
    cout << " * skipping particle init!" << endl;
    return;
  }

  /* Let wrapping fraction c (0,2)
     then z = c * R (if z = 0, c=0, at psi=0)
   */
  double z,c,R;
  
  c = -0.2; //wrapping fraction (- to place above membrane)
  R = _pRadius;
  z = R * (1-c);

  /* wish to implement boolean toggle that flips initial z and vz */
  if (_sunrise) {
    cout << "** swtiching particle direction **" << endl;

    z = -2*z;
    _dz = -_dz; //velocity stored as global differential
  }

  Particle = new Ball(0,0,z);
  Particle->R = R;

}


void fileInit() {

  cout << " initializing files.." << endl;

  // is it ineffective to do all of these all the time?

  f1 = fopen("balls.dat", "w");
  f2 = fopen("springs.dat", "w");
  //  f3 = fopen("force.txt", "w");
  //  f4 = fopen("contour.txt", "w");
  //  f5 = fopen("part.txt", "w");

  //  strcat(tag6, _tag.c_str());
  //  strcat(tag7, _tag.c_str());
  if (_fileTag) {
    //    string temp;
    string s6 = string("zForce.") + _tag + string(".txt");
    string s7 = string("WrappingEnergy.") + _tag + string(".txt");

    f6 = fopen(s6.c_str(), "w");
    f7 = fopen(s7.c_str(), "w");

  } else {
    char tag6[32] = "zForce.txt";
    char tag7[32] = "WrappingEnergy.txt";

    f6 = fopen(tag6, "w");
    f7 = fopen(tag7, "w");
  }

  /* Writes nTime, nBalls, and nSprings */
  nBalls = v_balls.size();
  nSprings = v_springs.size();

  cout << "   predicted time steps: " << _nSteps << endl;
  cout << "   physics steps per render step time: " << _tSamp << endl;
  cout << "   TOTAL differential steps: " << _tMax << endl;

  fprintf(f1, "%i\n" , _nSteps);
  fprintf(f1, "%i\n\n" , nBalls);
  fprintf(f2, "%i\n\n" , nSprings);

  initPID();
}

void filesClose() {
  fclose(f1);
  fclose(f2);
  fclose(f6);
  fclose(f7);
  /*
  fclose(f3);
  fclose(f4);
  fclose(f5);
  */

  cout << "files written." << endl;
}

/* writes PID */
void initPID() {

  int N = nBalls;
  for(int j=0; j<N; j++) { 

    //write pid (1-anchor, 0-spectrin, 2-boarder, 3-ankyrin)
    fprintf(f1, " %i", v_balls[j].pid);
  }
  fprintf(f1, "\n\n");

}

void randInit() {
  cout << " initializing rand.." << endl;
  unsigned int saat = (unsigned int)time(0); 
  randi.seed(saat);
}


/* Equilibriates the system, 
   or add desired displacement */
void sysConfig() {
  
  int N = v_balls.size();
  int j;
  for (j=0; j<N; j++) {
    v_balls[j].pid = 1;
    v_balls[j].isEdge = false;
  }

  int n = v_springs.size();
  float f = 0.4;
  for (j=0; j<n; j++) {
       v_springs[j].eql = f * v_springs[j].L;
  }

}
