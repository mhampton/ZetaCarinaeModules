#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelBrownianBridge;
extern Model* modelOrnsteinUhlenbeck;
extern Model* modelIOU;
extern Model* modelWarbler;
extern Model* modelRosenchance;
extern Model* modelGuildensTurn;
extern Model* modelRosslerRustler;
extern Model* modelFirefly;
