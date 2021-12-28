/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"
#include <array>

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

	float outsignal[16] = {0};
	float internaltime[16] = {0};
	float internalmaxtime[16] = {5};
	std::array<rack::dsp::SchmittTrigger,16> inputTrigger;
	float sqrtdelta = 1.0/std::sqrt(APP->engine->getSampleRate());

	BrownianBridge() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(NOISE_PARAM, 0.f, 1.f, 0.f, "Noise level");
		configParam(RANGE_PARAM, -10.f, 10.f, 5.f, "Output range");
		configParam(OFFSET_PARAM, -10.f, 10.f, 0.f, "Offset (minimum value)");
		configParam(TIME_PARAM, -10.f, 10.f, 1.f, "Transition Time","", 2.0);

		configInput(RANGE_INPUT, "Output range modulation");
		configInput(OFFSET_INPUT, "Offset (minimum value) modulation");
		configInput(NOISE_INPUT, "Noise level modulation");
		configInput(TIME_INPUT, "Transition time modulation");
		configInput(TRIG_INPUT, "Reset to minimum trigger");

		configOutput(SIG_OUTPUT, "Signal");
	}

	void onSampleRateChange() override {
		sqrtdelta = 1.0/std::sqrt(APP->engine->getSampleRate());
	}

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[TRIG_INPUT].getChannels(),
			std::max(inputs[RANGE_INPUT].getChannels(),
			std::max(inputs[OFFSET_INPUT].getChannels(),
			std::max(inputs[NOISE_INPUT].getChannels(),
			std::max(inputs[TIME_INPUT].getChannels(),
			1)))));
		for (int c = 0; c < channels; c++) {
			float range = params[RANGE_PARAM].getValue() + inputs[RANGE_INPUT].getVoltage(c);
			float offset = params[OFFSET_PARAM].getValue() + inputs[OFFSET_INPUT].getVoltage(c);
			float noise = params[NOISE_PARAM].getValue() + inputs[NOISE_INPUT].getVoltage(c)/10.0f;
			float timeParam = std::pow(2.0,params[TIME_PARAM].getValue()) + inputs[TIME_INPUT].getVoltage(c);
		
			if (inputTrigger[c].process(inputs[TRIG_INPUT].getVoltage(c)) || timeParam!=internalmaxtime[c]){
				internaltime[c] = 0.f;
				outsignal[c] = offset;
				internalmaxtime[c] = timeParam;
			}

			float r = random::normal(); 
			//float maxout = std:max(range+offset,offset);
			//float minout = std:min(range+offset,offset);

			internaltime[c] += args.sampleTime;
			internaltime[c] = clamp(internaltime[c],0.0f,timeParam);
			if(internaltime[c] < timeParam*0.999999f){
				outsignal[c] += sqrtdelta*r*noise*range;
				outsignal[c] += args.sampleTime*(range+offset-outsignal[c])/(timeParam - internaltime[c]);
				// outsignal[c] = clamp(outsignal[c], offset, range+offset); 
			}
			else{
				outsignal[c] = range+offset;
			}
			outputs[SIG_OUTPUT].setVoltage(outsignal[c],c);
		}
		outputs[SIG_OUTPUT].setChannels(channels);
        
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