// FHN.ino - 29/05/2014
// Tangible Networks
// Antos
//
// Fitzhugh-Nagumo (FHN) model of neuronal spiking.
// The FHN model is a two-dimensional simplification of the Hodgkin-Huxley model.
// 
// #TODO: Write some other description bits.
//
// See for example 
// - Keener/Sneyd "Mathematical Physiology" 2ed; Springer
// - http://www.scholarpedia.org/article/FitzHugh-Nagumo_model
// - http://en.wikipedia.org/wiki/FitzHugh%E2%80%93Nagumo_model

// ---------- INCLUDES
#include<math.h>
#include<TN.h>

// ---------- TN LIBRARY
// (note for A):
// Constructor is as TN(minVal, maxVal) where minVal and maxVal specify
// the range of analogRead() and analogWrite(): values outside of this
// range will be clipped! So these will need #TODO to be adjusted once we are
// coupling multiple nodes together.
TN Tn = TN(-1.0, 1.0);

// ---------- MODEL PARAMETERS
double I = 0.5;           // Input current
double tau = 12.5;        // 
double tauR = 1.0/tau;
double alpha = 0.7;       // 
double beta = 0.8;        // 

// For ICs, try {V = 1.0; W = 0.1}
double v[] = {1.0, 0.1};
double vOld[] = {1.0, 0.1};

double vRange[] = {-2.2, 2.2};

double vForCol = 0.0;

// timestep for integrator
double dt = 0.1;

void setup () {
  // Initialise the serial output port for logging etc.
  Serial.begin(115200);
}

void loop () {
  // update the previous V values
  for (int i = 0; i < 2; i++) {
    vOld[i] = v[i];
  }
  
  // Set the current (param 'I') using the potentiometer input
  // Tn.pot() returns a double between 0.0 (full CCW) and 1.0 (full CW).
  I = Tn.pot()*1.5;
  
  v[0] = vOld[0] + dt*(vOld[0] - (pow(vOld[0], 3.0))/3.0 - vOld[1] + I);
  v[1] = vOld[1] + dt*(tauR*(vOld[0] + alpha - beta*vOld[1]));
  
  Tn.colour(pow((v[0] - vRange[0])/(vRange[1] - vRange[0]), 2),
    	      0.0,
    	      0.0);
  
   //Serial.println((v[0] + 2.5)/(5));
   delay(5);
}
