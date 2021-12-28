/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"
#include <array>

struct OrnsteinUhlenbeck : Module {

	enum ParamIds {
		NOISE_PARAM,
		SPRING_PARAM,
		MEAN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NOISE_INPUT,
		SPRING_INPUT,
		MEAN_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIG_OUTPUT,
		NUM_OUTPUTS
	};

	float outsignal[16] = {0};
	std::array<rack::dsp::SchmittTrigger,16> inputTrigger;
	float sqrtdelta = 1.0/std::sqrt(APP->engine->getSampleRate());

	OrnsteinUhlenbeck() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(NOISE_PARAM, 0.f, 5.f, 0.f, "Noise level");
		configParam(SPRING_PARAM, 0.f, 10.f, 0.f, "Mean reverting strength");
		configParam(MEAN_PARAM, -10.f, 10.f, 1.f, "Mean");

		configInput(NOISE_INPUT, "Modulate noise/step level");
		configInput(SPRING_INPUT, "Modulate spring constant (restoring force strength)");
		configInput(MEAN_INPUT, "Modulate mean target value");
		configInput(TRIG_INPUT, "Trigger resets to mean");

		configOutput(SIG_OUTPUT, "Ornstein-Uhlenbeck process signal");
	}

	void onSampleRateChange() override {
		sqrtdelta = 1.0/std::sqrt(APP->engine->getSampleRate());
	}

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[TRIG_INPUT].getChannels(),
			std::max(inputs[NOISE_INPUT].getChannels(),
			std::max(inputs[SPRING_INPUT].getChannels(),
			std::max(inputs[MEAN_INPUT].getChannels(),
			1))));

		for (int c = 0; c < channels; c++) {
			float noise = params[NOISE_PARAM].getValue() + inputs[NOISE_INPUT].getVoltage(c)/10.0f;
			float spring = params[SPRING_PARAM].getValue() + inputs[SPRING_INPUT].getVoltage(c);
			float mean = params[MEAN_PARAM].getValue() + inputs[MEAN_INPUT].getVoltage(c);
		
			if (inputTrigger[c].process(inputs[TRIG_INPUT].getVoltage(c))){
				outsignal[c] = mean;
			}

			float r = random::normal(); 
			outsignal[c] += sqrtdelta*r*noise;
			outsignal[c] += spring*(mean-outsignal[c])*args.sampleTime;
			outputs[SIG_OUTPUT].setVoltage(outsignal[c],c);
		}
		outputs[SIG_OUTPUT].setChannels(channels);
        
	}
};



struct OrnsteinUhlenbeckWidget : ModuleWidget {
	OrnsteinUhlenbeckWidget(OrnsteinUhlenbeck* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/OrnsteinUhlenbeckPlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 22.2)), module, OrnsteinUhlenbeck::NOISE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 46.4)), module, OrnsteinUhlenbeck::SPRING_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 71.8)), module, OrnsteinUhlenbeck::MEAN_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 34.8)), module, OrnsteinUhlenbeck::NOISE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 59)), module, OrnsteinUhlenbeck::SPRING_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 84.4)), module, OrnsteinUhlenbeck::MEAN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 96.3)), module, OrnsteinUhlenbeck::TRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 113.76)), module, OrnsteinUhlenbeck::SIG_OUTPUT));



	}
};


Model* modelOrnsteinUhlenbeck = createModel<OrnsteinUhlenbeck, OrnsteinUhlenbeckWidget>("OrnsteinUhlenbeck");