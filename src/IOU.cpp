/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"
#include <array>

struct IOU : Module {

	enum ParamIds {
		NOISE_PARAM,
		SPRING_PARAM,
        DAMP_PARAM,
		MEAN_PARAM,
        MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NOISE_INPUT,
		SPRING_INPUT,
        DAMP_INPUT,
		MEAN_INPUT,
		EXT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
        RAND_OUTPUT,
		OU_OUTPUT,
        IOU_OUTPUT,
		NUM_OUTPUTS
	};

	float outrands[16] = {0};
    float outous[16] = {0};
    float outious[16] = {0};
	float sqrtdelta = 1.0/std::sqrt(APP->engine->getSampleRate());

	IOU() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(NOISE_PARAM, 0.f, 5.f, 2.f, "Noise level");
		configParam(SPRING_PARAM, 0.f, 10.f, 1.f, "Mean reverting strength");
        configParam(DAMP_PARAM, 0.f, 10.f, 1.f, "Velocity damping");
		configParam(MEAN_PARAM, -10.f, 10.f, 0.f, "Mean");
        configParam(MIX_PARAM, 0.f, 1.f, 0.f, "INT/EXT mix");

		configInput(NOISE_INPUT, "Modulate noise/step level");
		configInput(SPRING_INPUT, "Modulate spring constant (restoring force strength)");
		configInput(DAMP_INPUT, "Modulate damping coefficient");
		configInput(MEAN_INPUT, "Modulate mean target value");
		configInput(EXT_INPUT, "External signal (additive)");
		
		configOutput(RAND_OUTPUT, "White noise (mixed additively with external in)");
		configOutput(OU_OUTPUT, "Ornstein-Uhlenbeck process signal");
        configOutput(IOU_OUTPUT, "Integrated (smoothed) Ornstein-Uhlenbeck signal");
	}

	void onSampleRateChange() override {
		sqrtdelta = 1.0/std::sqrt(APP->engine->getSampleRate());
	}

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[SPRING_INPUT].getChannels(),
			std::max(inputs[NOISE_INPUT].getChannels(),
			std::max(inputs[DAMP_INPUT].getChannels(),
			std::max(inputs[MEAN_INPUT].getChannels(),
			std::max(inputs[EXT_INPUT].getChannels(),
			1)))));

		for (int c = 0; c < channels; c++) {
			float noise = params[NOISE_PARAM].getValue() + inputs[NOISE_INPUT].getVoltage(c)/10.0f;
			float spring = params[SPRING_PARAM].getValue() + inputs[SPRING_INPUT].getVoltage(c);
			float mean = params[MEAN_PARAM].getValue() + inputs[MEAN_INPUT].getVoltage(c);
			float damp = params[DAMP_PARAM].getValue() + inputs[DAMP_INPUT].getVoltage(c);
            float mix = params[MIX_PARAM].getValue();
            float ext = inputs[EXT_INPUT].getVoltage(c);


			float r = random::normal(); 
            outrands[c] = r*noise;
            outious[c] += outous[c]*args.sampleTime;
			outous[c] += sqrtdelta*outrands[c];
			outous[c] += (-spring*outous[c] + damp*(mean - outious[c]))*args.sampleTime;

			outputs[RAND_OUTPUT].setVoltage(outrands[c]*(1.0f-mix) + mix*ext,c);
            outputs[OU_OUTPUT].setVoltage(outous[c]*(1.0f-mix) + mix*ext,c);
            outputs[IOU_OUTPUT].setVoltage(outious[c]*(1.0f-mix) + mix*ext,c);
		}
		outputs[RAND_OUTPUT].setChannels(channels);
        outputs[OU_OUTPUT].setChannels(channels);
        outputs[IOU_OUTPUT].setChannels(channels);
        
	}
};



struct IOUWidget : ModuleWidget {
	float x1 = 8.4f;
	float x2 = 22.4f;
	IOUWidget(IOU* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/IOUPlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x1, 18)), module, IOU::NOISE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x1, 46)), module, IOU::SPRING_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x1, 72)), module, IOU::DAMP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x1, 100)), module, IOU::MEAN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(x2, 100)), module, IOU::MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, 31)), module, IOU::NOISE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, 59)), module, IOU::SPRING_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, 85)), module, IOU::DAMP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, 113)), module, IOU::MEAN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x2, 85)), module, IOU::EXT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x2, 25)), module, IOU::RAND_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x2, 42)), module, IOU::OU_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x2, 61)), module, IOU::IOU_OUTPUT));
	}
};


Model* modelIOU = createModel<IOU, IOUWidget>("IOU");