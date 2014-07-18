# Tangible Networks
## Fitzhugh-Nagumo Model

The Fitzhugh-Nagumo (FHN) model of neuronal spiking.
The FHN model is a two-dimensional simplification of the Hodgkin-Huxley equations.

## Implementation, node behaviour

### Behaviour of nodes in isolation

Nodes will not intrinsically spike. 
However once the input current is above a threshold, they will move into a spiking regime. 
As the current is further increased, the system will eventually saturate and stop spiking, reaching a second stable state. 

#### Basic inputs
- Push-button will pause the simulation in the current state while it is held down
- Potentiometer will adjust the input current ('forcing'/activation) for an individual node.

### Behaviour of connected nodes

Each node will by default broadcast to its outputs. 
Only connected inputs will receive a signal from neighbour nodes. 
Coupling is set to zero by default and so effects of coupling nodes together will only become visible if the master controller is connected somewhere.

#### Additional inputs, for a network
- Master controller changes coupling parameter k from -0.5 to +0.5
  - Positive values of k will induce synchronisation between connected nodes, IF a node is already spiking
  - Negative values of k will induce synchronisation in anti-phase. 
  This can induce spiking even if none of the nodes have any intrinsic per-node current input (local Pot = 0)
  - Coupling between nodes can be uni- or bi-directional, depending on the desired effect. 

### Outputs
- The 'voltage' variable in the FHN equations is visualised through a TN node's LED.
  LED brightness is proportional to voltage: as the node fires an 'action potential', the LED will swiftly glow more brightly, blinking more rapidly as input current (stimulus) is increased and spike trains are generated.
- If a piezo speaker is built into the node, and receives signal from the auxiliary output, a tone will be emitted as voltage passes a threshold.
  Thus, spiking neurons will elicit periodic beeping tones as they activate.
  The pitch of the tone can be set using all three dip switches:
  ```
  |dip1 | dip2 | dip3      |
  |-----|------|-----------|
  |  0  |  0   |  0  = OFF |
  |  1  |  0   |  0  = C   |
  |  0  |  1   |  0  = D   |
  |  1  |  1   |  0  = E   |
  |  0  |  0   |  1  = F   |
  |  1  |  0   |  1  = G   |
  |  0  |  1   |  1  = A   |
  |  1  |  1   |  1  = B   |
  ```

## Further information
For further details, see for example:
  - Keener/Sneyd "Mathematical Physiology" 2ed; Springer
  - http://www.scholarpedia.org/article/FitzHugh-Nagumo_model
  - http://en.wikipedia.org/wiki/FitzHugh%E2%80%93Nagumo_model
