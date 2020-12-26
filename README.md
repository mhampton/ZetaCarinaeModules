# ZetaCarinaeModules

This project is for my VCV rack modules.  All of these modules should support 16 channels of polyphony.  

## The Brownian Bridge
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/BrownianBridgeLogo.png" width="400">

The first module, the Brownian Bridge (https://en.wikipedia.org/wiki/Brownian_bridge), interpolates between two given values over a given time with a stochastic process - a directed Brownian motion.

![Brownian Bridge in VCV](https://github.com/mhampton/ZetaCarinaeModules/blob/master/BBshot.png?raw=true "BB in action")

## The Ornstein-Uhlenbeck
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/OrnsteinULogo.png" width="400">

The second module implements an [Ornstein-Uhlenbeck process](https://en.wikipedia.org/wiki/Ornstein%E2%80%93Uhlenbeck_process) (also known as the Vasicek model in finance).  It is a Brownian motion that has an additional mean-reverting tendency, which can be useful compared to a Brownian motion that is reflected within a bounded interval.


<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/Ornstein.png?raw=true " alt="Ornstein-Uhlenbeck in VCV" width="200"/>

## IOU
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/IOULogo.png" width="400">


The third module, "IOU", provides a few more options from a Ornstein-Uhlenbeck derived process.  IOU stands for "integrated Ornstein-Uhlenbeck", since the IOU output provides a solution x to the process

dx/dt = y

dy = (-c y - k(m - x))dt + s dB

where c is a linear damping parameter, k is a spring-like restoring parameter for the mean value m of x, and s is the amplitude of a Brownian motion process.  The module provides outputs for the Brownian increments (white noise) and the values of x and y.  An external input can be mixed together with all of the outputs.

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/IOU_inaction.png?raw=true " alt="IOU module in action" width="400"/>

## The Weeble Warbler
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/WeebleWarblerLogo.png" width="400">


The fourth module, the Weeble Warbler, is quite different.  It is my first attempt at an oscillator.  For each polyphonic channel it creates 8 dynamical systems which can be randomly perturbed by Brownian motions.  The eight channels can be altered in octaves to some extent by a harmonic control, and detuned.  The basic oscillation of each subsystem is of the form

x' = - k y + x (1 - x^2 - y^2) + external + noise


y' = k x + y (1 - x^2 - y^2)

The cubic terms herd each suboscillator towards a stable limit cycle with frequency k.  Left alone, these create sine waves, but adding an external signal will perturb the system in interesting ways.  

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/Warbler.png?raw=true " alt="Warbler in action" width="400"/>

## Rosenchance
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/RosenchanceLogo.png" width="400">


The fifth module is called Rosenchance.  It is a 2-state, 2-emission Hidden Markov Model (HMM).  The modulatable parameters P_A,A and P_B,B control the self-transition probabilities of the two states (A and B):

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/RosenchanceStates.png?raw=true " alt="Rosenchance state transition diagram" width="300px"/>

Each time a trigger is received, a state transition occurs and an emission value is generated.  For each state there is a modulatable probability P_E1 of emitting the E1 value for that state; the other emission value E2 has probability 1-P_E1.  The outputs also provide the current state (encoded as A=1V, B=2V), and triggers for the entry into each state (similar to the Bernoulli gate).  

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/Rosenchance.png?raw=true " alt="Rosenchance" width="125px"/>

## Guilden's Turn
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/GuildensTurnLogo.png" width="400">


The sixth module is called Guilden's Turn; it is a 4-state Markovian router. Each of the four states (A,B,C,D) can transition to its neighbors; so for example A can transition to B or D or to itself.  Modulatable controls are given for 2 of the three transition probabilities for each state, and the self-transition probability is derived from the relation that P_i,j + P_i,k + P_i,i = 1 for each state i.  If the forward and backward transition probabilities add up to more than 1, the self-transition probability is zero.  There are four signal inputs corresponding to each state, and the one corresponding to the active state is routed to the Out output.  

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/GuildenDiagrams.png?raw=true " alt="GuildensTurn Diagram" width="400px"/>

## The Rossler Rustler
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/RosslerLogo.png" width="400">


The seventh module, the Rossler Rustler, is somewhat similar to the Weebler Warbler, in that it is a combination oscillator/effect/distortion based on a dynamical system.  The dynamical system is the Rossler Attractor, a 3D chaotic dynamical system with a strange attractor.  

The differential equations are:

x' = k(- y - z)

y' = k(x + A y)

z' = k(B + z (x-C))

The attractor (for the parameters a=0.2,b=0.2,c=5.7) looks like this:

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/rossler.png?raw=true " alt="Rossler Attractor" width="400px"/>

The module provides controls for A,B, and C, as well as the ability to perturb the system with an external signal and modify the internal pitch through the parameter k.  

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/RosslerRustler.png?raw=true " alt="Rossler Attractor" width="400px"/>

## Firefly
<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/FireflyLogo.png" width="400">


The eighth module, still in an experimental development phase (warning: things might change!), is called Firefly.  Its an oscillator based on the Kuramoto model of phase-coupled oscillator systems, which has been extensively studied as a model for synchronization in biological systems including neurons in the brain and firefly flashing.  Firefly has 5 internal oscillators, whose phases evolve according to an ODE similar to the Kuramoto model:

T_i' = w_i + K sum(sin(T_j - T_i))

The output of the module is the sum of wavetable values for each oscillator, multiplied by a gain.  The wavetables can be set separately for each oscillator; wavetable 0 is a simple sine wave.

The Kuramoto model has mostly been studied for positive coupling; just for fun the option to have negative coupling has been included, and the form of the coupling function can be altered to be non-monotonic (through the "Type" control).

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/Firefly2.png?raw=true " alt="Kuramoto Oscillator" width="250px"/>


Thanks to Xenakios, KautenjaDSP, Squinky.Labs, baconpaul, k-chaffin, and augment for suggestions  on improvements and bugfixes to these modules in the VCV community forum and github.  Also thanks to cschol on github for all the very speedy reviews of this code.  Finally thank you Andrew Belt for creating and maintaining VCV Rack.