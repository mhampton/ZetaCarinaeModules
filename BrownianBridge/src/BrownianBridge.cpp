/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"

float outsignal = 0.f;
float internaltime = 0.f;
float internalmaxtime = 5.f;

struct BrownianBridge : Module {

	enum ParamIds {
		NOISE_PARAM,
		RANGE_PARAM,
		OFFSET_PARAM,
		TIME_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIG_OUTPUT,
		NUM_OUTPUTS
	};

	BrownianBridge() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(NOISE_PARAM, 0.f, 1.f, 0.f, "Noise level");
		configParam(RANGE_PARAM, 0.f, 10.f, 5.f, "Range");
		configParam(OFFSET_PARAM, -5.f, 5.f, 0.f, "Offset");
		configParam(TIME_PARAM, 0.1f, 5.f, 1.f, "Time");
	}

	void process(const ProcessArgs& args) override {
		float range = params[RANGE_PARAM].getValue();
		float offset = params[OFFSET_PARAM].getValue();
		float noise = params[NOISE_PARAM].getValue();
		float timeParam = params[TIME_PARAM].getValue();
		if(timeParam!=internalmaxtime){
			internaltime = 0;
			outsignal = offset;
			internalmaxtime = timeParam;
		}

		float r = random::normal(); 

		internaltime += args.sampleTime;
		internaltime = clamp(internaltime,0.0f,timeParam*0.99f);
		outsignal += std::sqrt(args.sampleTime)*r*noise;
		outsignal += args.sampleTime*(range+offset-outsignal)/(timeParam - internaltime);
		outsignal = clamp(outsignal, offset, range+offset);
 
        outputs[SIG_OUTPUT].setVoltage(5.f * outsignal);
	}
};



struct BrownianBridgeWidget : ModuleWidget {
	BrownianBridgeWidget(BrownianBridge* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BrownianBridge.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 56.410)), module, BrownianBridge::NOISE_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 94.410)), module, BrownianBridge::TRIG_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 114.410)), module, BrownianBridge::SIG_OUTPUT));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 73.410)), module, BrownianBridge::TIME_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 24.410)), module, BrownianBridge::RANGE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 40.410)), module, BrownianBridge::OFFSET_PARAM));


	}
};


Model* modelBrownianBridge = createModel<BrownianBridge, BrownianBridgeWidget>("BrownianBridge");