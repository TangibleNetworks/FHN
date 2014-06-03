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

// ---------- GENERAL MODEL PARAMETERS
double I = 0.5;           // Input current (forcing term in the voltage equation)
double IMax = 1.5;        // Max input current
double tau = 12.5;        // Time scale separation between fast and slow variables
double tauR = 1.0/tau;    // Reciprocal of the time constant
double alpha = 0.7;       // 
double beta = 0.8;        // 

// ---------- COUPLING
double k = 0.0;                  // Global coupling parameter used for diffusive coupling 
                                 // between neighbouring nodes.
double kRange[] = {0.0, 1.0};   // Range of the global coupling parameter k 
int nConnections = 0;            // Number of connected nodes
double vInput = 0;               // Total input voltage to this node from its neighbours

// ---------- DYNAMIC VARIABLES
double v[] = {1.0, 0.1};          // Current value of the two dynamic variables
                                  // (in the equations 'V' = v[0], 'W' = v[1])
double vOld[] = {1.0, 0.1};       // Previous value of the two dynamic variables
double vRange[] = {-2.2, 2.2};    // Range of 'V' within which to constrain (for colouring etc)

double dt = 0.05;  // Timestep for Euler integrator

// ---------- TN LIBRARY
// (note for A):
// Constructor is as TN(minVal, maxVal) where minVal and maxVal specify
// the range of analogRead() and analogWrite(): values outside of this
// range will be clipped! So these will need #TODO to be adjusted once we are
// coupling multiple nodes together.
TN Tn = TN(-2.5, 2.5);


void setup () {
  // Initialise the serial output port for logging etc.
  Serial.begin(115200);
}

void loop () {
  // If the push-button switch is pressed in then do nothing, 
  // else iterate the simulation
  
  if (!Tn.sw()) {
    // Set up the coupling parameter k
    if (Tn.masterConnected()) {
      // Choose coupling strength based on the global potentiometer
      // state and the range of possible k values
      k = kRange[0] + (kRange[1] - kRange[0])*Tn.masterRead();
    } else {
      // This will result in zero coupling strength if the kRange is
      // From [-1.0 to 1.0] rather than from [0.0 to 1.0]...!
      k = kRange[0] + (kRange[1] - kRange[0])*0.5;
    }
    
    // update the previous V values
    for (int i = 0; i < 2; i++) {
      vOld[i] = v[i];
    }
    
    // Set the current (param 'I') using the potentiometer input
    // Tn.pot() returns a double between 0.0 (full CCW) and 1.0 (full CW).
    I = Tn.pot()*IMax;
    
    // Compute the input voltage from connected neighbours (if any)
    // vInput = (k / N) * \sum_j (v_j - v_i)
    // where k = global coupling strength
    //       N = num connected neighbours,
    //       i = current node
    //       j = neighbour node
    //       v = voltage (v[0])
    nConnections = 0;
    vInput = 0;
    for (int i = 0; i < 3; i++) {
      if (Tn.isConnected(i + 1)) {
        vInput = vInput + (Tn.analogRead(i + 1) - vOld[0]);
        nConnections++;
      }
    }
    if (nConnections > 0) {
      vInput = k*vInput/nConnections;
    }
    
    // Forward Euler
    v[0] = vOld[0] + dt*(vOld[0] - (pow(vOld[0], 3.0))/3.0 - vOld[1] + I + vInput);
    v[1] = vOld[1] + dt*(tauR*(vOld[0] + alpha - beta*vOld[1]));
  }
  
  // Broadcast the voltage variable 'V' = v[0] to all connected
  // neighbouring nodes.
  for (int i = 0; i < 3; i++) {
    Tn.analogWrite(i + 1, v[0]);
  }
  
  // Glow red for high voltage, dark for low voltage.
  // Artistic license... We will normalise and square the voltage for
  // a prettier effect.
  Tn.colour(pow((v[0] - vRange[0])/(vRange[1] - vRange[0]), 2),
    	      0.0,
    	      0.15);
   
   //Serial.println((v[0] + 2.5)/(5));
   //delay(5);
}
