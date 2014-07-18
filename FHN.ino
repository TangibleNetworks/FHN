// FHN.ino - 18/07/2014
// Tangible Networks
// Antos
//
// Fitzhugh-Nagumo (FHN) model of neuronal spiking.
// The FHN model is a two-dimensional simplification of the Hodgkin-Huxley model.
// 
// Nodes in isolation (basic features):
// - Push-button will pause the simultion in the current state while it is held down
// - Pot will adjust the input current ('forcing'/activation) for an individual node
// Nodes will not intrinsically spike. However once the input current is above a 
// threshold, they will move into a spiking regime. As the current is further increased,
// the system will eventually saturate and stop spiking, reaching a second stable state.
//
// Each node will by default broadcast to its outputs. Only connected inputs will receive
// a signal from neighbour nodes. Coupling is set to zero by defualt and so effects of
// coupling nodes together will only become visible if the master controller is connected
// somewhere.
//
// Connected nodes, basic behaviour:
// - Master controller changes coupling parameter k from -0.5 to +0.5
// - Positive values of k will induce synchronisation between connected nodes, IF a node is
//   already spiking
// - Negative values of k will induce synchronisation in anti-phase. This can induce spiking
//   even if none of the nodes have any intrinsic per-node current input (local Pot = 0)
// - Coupling between nodes can be uni- or bi-directional, depending on the desired effect.
//
// For further details, see for example 
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
double kRange[] = {-0.5, 0.5};   // Range of the global coupling parameter k 
int nConnections = 0;            // Number of connected nodes
double vInput = 0;               // Total input voltage to this node from its neighbours

// ---------- DYNAMIC VARIABLES
double v[] = {1.0, 0.1};          // Current value of the two dynamic variables
                                  // (in the equations 'V' = v[0], 'W' = v[1])
double vOld[] = {1.0, 0.1};       // Previous value of the two dynamic variables
double vRange[] = {-2.2, 3.0};    // Range of 'V' within which to constrain (for colouring etc)

double dt = 0.05;  // Timestep for Euler integrator

// ---------- TN LIBRARY
// (note for A):
// Constructor is as TN(minVal, maxVal) where minVal and maxVal specify
// the range of analogRead() and analogWrite(): values outside of this
// range will be clipped! So these will need #TODO to be adjusted once we are
// coupling multiple nodes together.
TN Tn = TN(-3.5, 3.5);

// ---------- OUTPUT
double out = 0.0;

// ---------- SOUND FROM PIEZO
byte dipBitMap = 0;
double cMajor[] = {261.626, 293.665, 329.628, 349.228, 391.995, 440, 493.883};

void setup () {
  // Initialise the serial output port for logging etc.
  //Serial.begin(115200);
  
  pinMode(AUX_OUT, OUTPUT);
}


void loop () {
  // Set up the coupling parameter k
  if (Tn.masterConnected()) {
    // Choose coupling strength based on the global potentiometer
    // state and the range of possible k values
    k = kRange[0] + (kRange[1] - kRange[0])*Tn.masterRead();
  } else {
    // This will result in zero coupling strength if the kRange is
    // From [-1.0 to 1.0] rather than from [0.0 to 1.0]...!
    k = 0.0;//kRange[0] + (kRange[1] - kRange[0])*0.5;
  }
  
  // update the previous V values
  for (int i = 0; i < 2; i++) {
    vOld[i] = v[i];
  }
  
  if(!Tn.sw()){
    // Set the current (param 'I') using the potentiometer input
    // Tn.pot() returns a double between 0.0 (full CCW) and 1.0 (full CW).
    I = Tn.pot()*IMax;
  } else {
    I = IMax;
  }
  
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
  
  // Broadcast the voltage variable 'V' = v[0] to all connected
  // neighbouring nodes.
  for (int i = 0; i < 3; i++) {
    Tn.analogWrite(i + 1, v[0]);
  }
  
  // Artistic license... We will normalise and square the voltage for
  // a prettier effect.
  out = pow((v[0] - vRange[0])/(vRange[1] - vRange[0]), 2);
  
  // Using AUX for audio breaks the red LED so now we are green/blue  
  // Glow green for high voltage, dark (slightly blue) for low voltage.
  Tn.colour(0.0,
    	    out,
    	    0.15);
  
  // Just bitwise combine all the dip switches to select our 
  // desired tone :)
  dipBitMap = B001*Tn.dip1() + B010*Tn.dip2() + B100*Tn.dip3();
  
  /*
  So, in terms of dip switches:
  dip1 | dip2 | dip3
    0  |  0   |  0  = OFF
    1  |  0   |  0  = C
    0  |  1   |  0  = D
    1  |  1   |  0  = E
    0  |  0   |  1  = F
    1  |  0   |  1  = G
    0  |  1   |  1  = A
    1  |  1   |  1  = B  
  */
  
  if(out > 0.25) {
    // Play tone
    if(dipBitMap > 0) {
      // At least one tone spec is activated
      tone(AUX_OUT, cMajor[dipBitMap - 1]);
    } else {
      noTone(AUX_OUT);
    }  
  } else {
    // 0 0 0
    noTone(AUX_OUT);
  }
  
  //Serial.println(v[0]);
  //printSOI();
  //delay(1);
}
