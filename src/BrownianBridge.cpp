/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"

struct BrownianBridge : Module {

	enum ParamIds {
		NOISE_PARAM,
		RANGE_PARAM,
		OFFSET_PARAM,
		TIME_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		RANGE_INPUT,
		OFFSET_INPUT,
		NOISE_INPUT,
		TIME_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIG_OUTPUT,
		NUM_OUTPUTS
	};

	float outsignal = 0.f;
	float internaltime = 0.f;
	float internalmaxtime = 5.f;
	dsp::SchmittTrigger inputTrigger;

	BrownianBridge() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(NOISE_PARAM, 0.f, 1.f, 0.f, "Noise level");
		configParam(RANGE_PARAM, 0.f, 10.f, 5.f, "Range");
		configParam(OFFSET_PARAM, -5.f, 5.f, 0.f, "Offset");
		configParam(TIME_PARAM, 0.0001f, 5.f, 1.f, "Time");
	}

	void process(const ProcessArgs& args) override {
		float range = params[RANGE_PARAM].getValue();
		float offset = params[OFFSET_PARAM].getValue();
		float noise = params[NOISE_PARAM].getValue();
		float timeParam = params[TIME_PARAM].getValue();
	
		if (inputTrigger.process(inputs[TRIG_INPUT].getVoltageSum()) or timeParam!=internalmaxtime){
			internaltime = 0;
			outsignal = offset;
			internalmaxtime = timeParam;
		}

		float r = random::normal(); 

		internaltime += args.sampleTime;
		internaltime = clamp(internaltime,0.0f,timeParam*0.999999f);
		outsignal += std::sqrt(args.sampleTime)*r*noise*range;
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.0, 24.0)), module, BrownianBridge::RANGE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.0, 44.0)), module, BrownianBridge::OFFSET_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.0, 64.0)), module, BrownianBridge::NOISE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.0, 84.0)), module, BrownianBridge::TIME_PARAM));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 24)), module, BrownianBridge::RANGE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 44)), module, BrownianBridge::OFFSET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 64)), module, BrownianBridge::NOISE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 84)), module, BrownianBridge::TIME_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 109)), module, BrownianBridge::TRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24, 109)), module, BrownianBridge::SIG_OUTPUT));



	}
};


Model* modelBrownianBridge = createModel<BrownianBridge, BrownianBridgeWidget>("BrownianBridge");