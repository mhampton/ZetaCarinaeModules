#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelBrownianBridge);
	p->addModel(modelOrnsteinUhlenbeck);
	p->addModel(modelIOU);
	p->addModel(modelWarbler);
	p->addModel(modelRosenchance);
	p->addModel(modelGuildensTurn);
	p->addModel(modelRosslerRustler);
	p->addModel(modelFirefly);
	

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
