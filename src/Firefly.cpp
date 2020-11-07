/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"

struct FireflyModule : Module 
{
    enum ParamIds {
        W1R_PARAM,
        W2R_PARAM,
        W3R_PARAM,
        W4R_PARAM,
        K_PARAM,
        GAIN_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
		W1R_INPUT,
        W2R_INPUT,
        W3R_INPUT,
        W4R_INPUT,
        K_INPUT,
		VOCT_INPUT,
        GAIN_INPUT,
		NUM_INPUTS
	};

    enum OutputIds {
		SM_OUTPUT,
		NUM_OUTPUTS
	};

    float y[16][4] = {{0}};
    float out[16] = {0};
	float nt = 6.2831853f;

    FireflyModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(W1R_PARAM, 0.f,  20.f, 1.f, "Ratio 1"); 
        configParam(W2R_PARAM, 0.f,  20.f, 1.01f, "Ratio 2"); 
        configParam(W3R_PARAM, 0.f,  20.f, 0.5f, "Ratio 3"); 
        configParam(W4R_PARAM, 0.f,  20.f, 2.0f, "Ratio 4"); 
        configParam(K_PARAM, -10.f,  10.f, 0.1f, "Coupling"); 
        configParam(GAIN_PARAM, 0.f,  10.f, 1.0f, "Gain"); 
		for (int c = 0; c < 16; c++){
            for (int i=0; i<4; i++){
			    y[c][i] = 0.f;
            }
		}
	};


	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[W1R_INPUT].getChannels(),1);

		float Kp = params[K_PARAM].getValue();
        float W1p = params[W1R_PARAM].getValue();
        float W2p = params[W2R_PARAM].getValue();
        float W3p = params[W3R_PARAM].getValue();
        float W4p = params[W4R_PARAM].getValue();
		float dt = args.sampleTime;
        float gainp = params[GAIN_PARAM].getValue();

		for (int c = 0; c < channels; c++) {
            out[c] = 0.f;
			//float ext = inputs[EXT_INPUT].getVoltage(c);
            float ks[4];
            float voct = inputs[VOCT_INPUT].getVoltage(c);
            float W1 = W1p + inputs[W1R_INPUT].getVoltage(c);
            float W2 = W2p + inputs[W2R_INPUT].getVoltage(c);
            float W3 = W3p + inputs[W3R_INPUT].getVoltage(c);
            float W4 = W4p + inputs[W4R_INPUT].getVoltage(c);
            float K = Kp + inputs[K_INPUT].getVoltage(c);
            float freq = dsp::FREQ_C4 * std::pow(2.f, voct)*nt;
            float gain = gainp + inputs[GAIN_INPUT].getVoltage(c);

            float wlist[4] = {W1*freq, W2*freq, W3*freq, W4*freq};
			for (int i = 0; i < 4; i++) {
                ks[i] = wlist[i];
                for (int j = 0; j < 4; j++) {
                    if (i!=j){
                        ks[i] += wlist[i] * K * std::sin(y[c][j] - y[c][i]);
                    }
                }
                y[c][i] += ks[i]*dt;
                y[c][i] = y[c][i] - std::floor(y[c][i]/nt)*nt;
                out[c] += std::sin(y[c][i]);
            }

			outputs[SM_OUTPUT].setVoltage(out[c]*gain, c);
		}
		outputs[SM_OUTPUT].setChannels(channels);
        
	}
};

struct FireflyWidget : ModuleWidget {
	float xc = 7.0;
	float xc2 = 34;
	FireflyWidget(FireflyModule* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FireflyPlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 20.)), module, FireflyModule::W1R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 34.)), module, FireflyModule::W2R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 48.)), module, FireflyModule::W3R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 62.)), module, FireflyModule::W4R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 76.)), module, FireflyModule::K_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 90.)), module, FireflyModule::GAIN_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 20)), module, FireflyModule::W1R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 34)), module, FireflyModule::W2R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 48)), module, FireflyModule::W3R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 62)), module, FireflyModule::W4R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 76)), module, FireflyModule::K_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 90)), module, FireflyModule::GAIN_INPUT));

		//addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2,100)), module, FireflyModule::EXT_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2,104)), module, FireflyModule::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc2, 116)), module, FireflyModule::SM_OUTPUT));

	}
};


Model* modelFirefly = createModel<FireflyModule, FireflyWidget>("Firefly");