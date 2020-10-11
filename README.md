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

<img src="https://github.com/mhampton/ZetaCarinaeModules/blob/master/IOU_inaction.png?raw=true " alt="IOU module in action" width="200"/>

Thanks to Xenakios, Squinky.Labs, and baconpaul for suggestions on improvements.