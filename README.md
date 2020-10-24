# ZetaCarinaeModules

This project is for my VCV rack modules.  

The first module, the Brownian Bridge (https://en.wikipedia.org/wiki/Brownian_bridge), interpolates between two given values over a given time with a stochastic process - a directed Brownian motion.

![Brownian Bridge in VCV](https://github.com/mhampton/ZetaCarinaeModules/blob/master/BBshot.png?raw=true "BB in action")

The second module implements an [Ornstein-Uhlenbeck process](https://en.wikipedia.org/wiki/Ornstein%E2%80%93Uhlenbeck_process) (also known as the Vasicek model in finance).  It is a Brownian motion that has an additional mean-reverting tendency, which can be useful compared to a Brownian motion that is reflected within a bounded interval.


<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/Ornstein.png?raw=true " alt="Ornstein-Uhlenbeck in VCV" width="200"/>

The third module, "IOU", provides a few more options from a Ornstein-Uhlenbeck derived process.  IOU stands for "integrated Ornstein-Uhlenbeck", since the IOU output provides a solution x to the process

dx/dt = y

dy = (-c y - k(m - x))dt + s dB

where c is a linear damping parameter, k is a spring-like restoring parameter for the mean value m of x, and s is the amplitude of a Brownian motion process.  The module provides outputs for the Brownian increments (white noise) and the values of x and y.  An external input can be mixed together with all of the outputs.

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/IOU_inaction.png?raw=true " alt="IOU module in action" width="400"/>

The fourth module, the Weeble Warbler, is quite different.  It is my first attempt at an oscillator.  For each polyphonic channel it creates 8 dynamical systems which can be randomly perturbed by Brownian motions.  The eight channels can be altered in octaves to some extent by a harmonic control, and detuned.  The basic oscillation of each subsystem is of the form

x' = - k y + x (1 - x^2 - y^2) + external + noise


y' = k x + y (1 - x^2 - y^2)

The cubic terms herd each suboscillator towards a stable limit cycle with frequency k.  Left alone, these create sine waves, but adding an external signal will perturb the system in interesting ways.  

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/Warbler.png?raw=true " alt="Warbler in action" width="400"/>

The fifth module is called Rosenchance.  It is a 2-state, 2-emission Hidden Markov Model (HMM).  The modulatable parameters P_A,A and P_B,B control the self-transition probabilities of the two states (A and B):

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/RosenchanceStates.png?raw=true " alt="Rosenchance state transition diagram" width="300px"/>

Each time a trigger is received, a state transition occurs and an emission value is generated.  For each state there is a modulatable probability P_E1 of emitting the E1 value for that state; the other emission value E2 has probability 1-P_E1.  The outputs also provide the current state (encoded as A=1V, B=2V), and triggers for the entry into each state (similar to the Bernoulli gate).  

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/Rosenchance.png?raw=true " alt="Rosenchance" width="125px"/>

The sixth module is called Guilden's Turn; it is a 4-state Markovian router. Each of the four states (A,B,C,D) can transition to its neighbors; so for example A can transition to B or D or to itself.  Modulatable controls are given for 2 of the three transition probabilities for each state, and the self-transition probability is derived from the relation that P_i,j + P_i,k + P_i,i = 1 for each state i.  If the forward and backward transition probabilities add up to more than 1, the self-transition probability is zero.  There are four signal inputs corresponding to each state, and the one corresponding to the active state is routed to the Out output.  

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/GuildenDiagrams.png?raw=true " alt="GuildensTurn Diagram" width="400px"/>


Thanks to Xenakios, Squinky.Labs, baconpaul, k-chaffin, and augment for suggestions on improvements to these modules in the VCV community forum.  Also thanks to cschol on github for all the very speedy reviews of this code.  Finally thank you Andrew Belt for creating and maintaining VCV Rack.