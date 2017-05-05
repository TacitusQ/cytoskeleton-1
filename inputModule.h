/* This file includes
   setDouble()
   setFloat()
   setInt()
   setBool()

   process()
   checktime()
   readInput()
   we suggest the reader start from the bottom

   printLog()
 */

#include <fstream>
#include <iostream>
#include <ctime>

using namespace std;

void setDouble(double &param, ifstream &data, char* name){

    double x; data >> x;
    cout << " changing: " 
	 << name << " " 
	 << param << " -> " 
	 << x << endl;
    param = x;

}

void setInt(int &param, ifstream &data, char* name){

    int x; data >> x;
    cout << " changing: " 
	 << name << " " 
	 << param << " -> " 
	 << x << endl;
    param = x;

}

void setFloat(float &param, ifstream &data, char* name){

    float x; data >> x;
    cout << " changing: " 
	 << name << " " 
	 << param << " -> " 
	 << x << endl;
    param = x;

}

void setBool(bool &param, ifstream &data, char* name){

    bool x; data >> x;
    cout << " set: " 
	 << name << " " 
	 << param << " -> " 
	 << x << endl;
    param = x;

}

void process(char* in, std::ifstream &fin) {

  if (strcmp(in, "epsilon") == 0) setDouble(_epsilon, fin, in);
  if (strcmp(in, "gamma") == 0)   setDouble(_gamma, fin, in);
  if (strcmp(in, "D") == 0)       setDouble(_D, fin, in);
  if (strcmp(in, "k") == 0)       setDouble(_k, fin, in);

  if (strcmp(in, "nSpectrin") == 0) setInt(_nSpectrin,fin,in);
  if (strcmp(in, "tEqGlobal") == 0) setInt(_tEqGlobal,fin,in);
  if (strcmp(in, "tEqLocal") == 0)  setInt(_tEqLocal,fin,in);
  if (strcmp(in, "SysSize") == 0)   setInt(_nSys,fin,in);
  if (strcmp(in, "nSteps") == 0)    setInt(_nSteps,fin,in);
  if (strcmp(in, "tSamp") == 0)     setInt(_tSamp,fin,in);

  if (strcmp(in, "dt") == 0)  setFloat(_dt,fin,in);
  if (strcmp(in, "dz") == 0)  setFloat(_dz,fin,in);

  if (strcmp(in, "Print") == 0) setBool(_printTime,fin,in);
  if (strcmp(in, "Sunrise") == 0) setBool(_sunrise,fin,in);
  if (strcmp(in, "Ankyrin") == 0) setBool(_ankyrin,fin,in);
  if (strcmp(in, "Particle") == 0)  setBool(_Particle,fin,in);


  if (strcmp(in, "Tag") == 0) {

    cout << "reading: " << in << endl;
   
    _fileTag = true;
    fin >> _tag;
    cout << "   " << _tag << endl;
  }


  if (strcmp(in, "Radius") == 0) {

    cout << "reading: " << in << endl;
    float r; fin >> r; //gets data

    cout << "   " << _pRadius << endl;
    _pRadius = r * _lActin;
    cout << "   " << _pRadius << endl;
  }


  if (strcmp(in, "SpectrinContour") == 0) {

    cout << "reading: " << in << endl;
    float c; fin >> c; //gets data

    cout << "   " << _Contour << endl;
    _Contour = c * _lActin;
    cout << "   " << _Contour << endl;
  }

  if (strcmp(in, "Zara") == 0) {
    cout << "true love" << endl;
  }

}


/* Timing Considerations and Recalculation */
void checkTime() {

  if (_tSamp <= _tEqLocal) {

    cout << "\nFatal Error: Equilibrium Time (local) exceeds sample Time" << endl;
    cout << "   local Equlibrium delay: " << _tEqLocal << endl;
    cout << "   allowed sample time: " << _tSamp << endl;
    cout << "** measurements will never be made!!" << endl;
    getc(stdin);
  }

  //recalc max time
  _tMax = _tEqGlobal + (_tSamp * _nSteps);
}

/* this is the main() file */

void readInput() {

  cout << "Reading from file" << endl;   

  char data[100];
  ifstream infile; 
  infile.open("command.txt");

  while (infile >> data) {
    //    cout << "loop" << endl;
    process(data, infile);
  }
  infile.close();

  checkTime();
}


/* append */
void printLog() {

  cout << "Preparing .log file" << endl;
  char fname[64] = "";

  if (_fileTag) strcat(fname, _tag.c_str() );
  else strcat(fname, "temp");

  strcat(fname, ".log");
  cout << " writing to " << fname << endl;

  std::ofstream log (fname, std::ofstream::out);
  log << endl;

  /* time */
  log << "Time Parameters" << endl;
  log << " nSteps: " << _nSteps << endl;
  log << " tSamp: " << _tSamp << endl;
  log << " tMax: " << _tMax << endl;
  log << " dt: " << _dt << endl;
  log << " tEqLocal: " << _tEqLocal << endl;
  log << " tEqGlobal: " << _tEqGlobal << endl;


  /* system */
  log << "System Parameters" << endl;
  log << " nSys: " << _nSys << endl;
  log << " lActin: " << _lActin << endl;
  log << " Contour: " << _Contour << endl;
  log << " nSpectrin: " << _nSpectrin << endl;
  log << " particleRadius: " << _pRadius << endl;
  log << " spectrinRadius: " << _sRadius << endl;
  log << " sunrise: " << _sunrise << endl;

  /* physics */
  log << "Physics Parameters" << endl;
  log << " D: " << _D << endl;
  log << " gamma: " << _gamma << endl;
  log << " epsilon: " << _epsilon << endl;
  log << " k: " << _k << endl;
  log << " m: " << _m << endl;
  log << " dz: " << _dz << endl;

  /* finish */
  time_t now = time(0);
  char* cstamp = ctime(&now);
  log << "Launch time (moscow): " << cstamp << endl;
  log << " w version: " << _version << endl;
  log.close();

}
