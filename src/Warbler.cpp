/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"
#include <vector>

struct WarblerModule : Module 
{
    enum ParamIds {
        NOISE_PARAM,
		DETUNE_PARAM,
		GAIN_PARAM,
		HARMN_PARAM,
		RGAIN_PARAM,
		DGAIN_PARAM,
		GGAIN_PARAM,
		HGAIN_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
		NOISE_INPUT,
		DETUNE_INPUT,
		GAIN_INPUT,
		HARMN_INPUT,
		PITCH_INPUT,
		EXT_INPUT,
		NUM_INPUTS
	};
    enum OutputIds {
		X_OUTPUT,
		Y_OUTPUT,
		NUM_OUTPUTS
	};
	static const int normalRandomTableSize = 1000003;
	std::vector<float> normalRandomTable;
	int normalRandomTableIndex = 0;
    float xoutsignal[16] = {0};
	float youtsignal[16] = {0};
	float xint[128] = {0};
	float yint[128] = {1};
    float sqrtdelta = 1.0f/std::sqrt(APP->engine->getSampleRate());
	float dets[8] = {0,-1,2,-3,4,-5,6,-7};
	//float dets[8] = {0.000929f,0.000377f,0.000076f,0.0f,0.000081f,0.000108f,0.000153f,0.000487f};
    float indets[128] = {0.001f};
	float mults[168] = {
		0.03125, 0.0625,0.125,0.25,0.5, 0.5, 1.0, 1.0,
		0.0625,  0.125, 0.25, 0.25,0.5, 0.5, 1.0, 1.0,
		0.125,   0.25,  0.25, 0.5, 0.5, 0.5, 1.0, 1.0,
		0.125,   0.25,  0.25, 0.5, 0.5, 1.0, 1.0, 1.0,
		0.25,    0.25,  0.5,  0.5, 1.0, 1.0, 1.0, 1.0,
		0.25,    0.5,   0.5,  0.5, 1.0, 1.0, 1.0, 2.0,
		0.25,    0.5,   0.5,  1.0, 1.0, 1.0, 1.0, 2.0,
		0.25,    0.5,   0.5,  1.0, 1.0, 1.0, 2.0, 2.0,
		0.25,    0.5,   1.0,  1.0, 1.0, 1.0, 1.0, 2.0,
		0.50,    0.5,   1.0,  1.0, 1.0, 1.0, 1.0, 2.0,
		1.00,    1.0,   1.0,  1.0, 1.0, 1.0, 1.0, 1.0, 
		1.00,    1.0,   1.0,  1.0, 1.0, 1.0, 1.0, 2.0,
		0.50,    1.0,   1.0,  1.0, 1.0, 1.0, 2.0, 2.0,
		0.50,    1.0,   1.0,  1.0, 1.0, 2.0, 2.0, 2.0,
		0.50,    1.0,   1.0,  1.0, 2.0, 2.0, 2.0, 3.0,
		// 0.50,    1.0,   1.0,  1.0, 2.0, 2.0, 3.0, 3.0,
		1.00,    1.0,   1.0,  1.0, 2.0, 2.0, 3.0, 4.0,
		1.00,    1.0,   1.0,  2.0, 2.0, 2.0, 3.0, 5.0,
		1.00,    1.0,   2.0,  2.0, 3.0, 4.0, 5.0, 6.0,
		1.00,    1.0,   2.0,  3.0, 4.0, 5.0, 6.0, 7.0,
		1.00,    2.0,   3.0,  4.0, 5.0, 6.0, 7.0, 7.0,
		1.00,    2.0,   3.0,  4.0, 5.0, 6.0, 7.0, 8.0,
	};

    WarblerModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(NOISE_PARAM, 0.f, 1.f, 0.01f, "Stochasticity");
		configParam(DETUNE_PARAM, 0.f, 2.f, 0.0001f, "Variation/detune amount");
		configParam(GAIN_PARAM, 0.f, 10.f, 1.0f, "Input influence");
		configParam(HARMN_PARAM, 0, 20, 10, "(Sub)Harmonics");
		configParam(RGAIN_PARAM, 0.f, 2.f, 0.1f, "Attenuator for external random input");
		configParam(DGAIN_PARAM, 0.f, 2.f, 0.1f, "Attenuator for external detune input");
		configParam(GGAIN_PARAM, 0.f, 2.f, 0.1f, "Attenuator for external gain input");
		configParam(HGAIN_PARAM, 0.f, 2.f, 0.1f, "Attenuator for external harmonic input");

		configInput(NOISE_INPUT, "Modulate additive noise level");
		configInput(DETUNE_INPUT, "Modulate detune");
		configInput(GAIN_INPUT, "Modulate gain");
		configInput(HARMN_INPUT, "Modulate harmonics");
		configInput(PITCH_INPUT, "Set pitch V/oct");
		configInput(EXT_INPUT, "External signal");

		configOutput(X_OUTPUT, "X value of summed oscillators");
		configOutput(Y_OUTPUT, "Y value of summed oscillators");
		normalRandomTable.resize(normalRandomTableSize);
		for (size_t i=0;i<normalRandomTable.size();++i)
			normalRandomTable[i] = random::normal();
	};

    void onSampleRateChange() override {
		sqrtdelta = 1.0/std::sqrt(APP->engine->getSampleRate());
	};

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[NOISE_INPUT].getChannels(),
			std::max(inputs[DETUNE_INPUT].getChannels(),
			std::max(inputs[EXT_INPUT].getChannels(),
			std::max(inputs[PITCH_INPUT].getChannels(),
			1))));

		for (int c = 0; c < channels; c++) {
			float noise = params[NOISE_PARAM].getValue() + params[RGAIN_PARAM].getValue()*inputs[NOISE_INPUT].getVoltage(c);
			float detune = params[DETUNE_PARAM].getValue()/10.f + params[DGAIN_PARAM].getValue()*inputs[DETUNE_INPUT].getVoltage(c);
			float pitch = inputs[PITCH_INPUT].getVoltage(c);
			float extin = inputs[EXT_INPUT].getVoltage(c)/10.0f; // was /40.0f;
			float ingain = params[GAIN_PARAM].getValue() + params[GGAIN_PARAM].getValue()*inputs[GAIN_INPUT].getVoltage(c);
			int hp = round(params[HARMN_PARAM].getValue() + params[HGAIN_PARAM].getValue()*inputs[HARMN_INPUT].getVoltage(c));
			hp = clamp(hp,0,20);
			
			xoutsignal[c] = 0.f;
			youtsignal[c] = 0.f;
			for (int ri = 0; ri < 8; ri++) {
				float rf = mults[hp*8 + ri];
				pitch = clamp(pitch + indets[c*8 + ri], -5.f, 5.f);
				float rad2 = xint[c*8 + ri]*xint[c*8 + ri] + yint[c*8 + ri]*yint[c*8 + ri];
				
				float kf = dsp::FREQ_C4 * std::pow(2.f, pitch)*6.2831853f;
				float r = normalRandomTable[normalRandomTableIndex]*noise*sqrtdelta; 
				++normalRandomTableIndex;
				if (normalRandomTableIndex==normalRandomTableSize)
					normalRandomTableIndex = 0;
				float xdnew = rf*kf*(-yint[c*8 + ri] + 2.f*xint[c*8 + ri]*(1.0f - rad2) + ingain*extin)*args.sampleTime + r;
				
				yint[c*8 + ri] += rf*kf*(xint[c*8 + ri] + 2.f*yint[c*8 + ri]*(1.0f - rad2))*args.sampleTime;
				xint[c*8 + ri] += xdnew;
				indets[c*8 + ri] += rf*kf*(r + dets[ri]*detune - indets[c*8 + ri])*args.sampleTime;

				xint[c*8 + ri] = clamp(xint[c*8 + ri], -1.25f, 1.25f);
				yint[c*8 + ri] = clamp(yint[c*8 + ri], -1.25f, 1.25f);

				xoutsignal[c] += xint[c*8 + ri];
				youtsignal[c] += yint[c*8 + ri];
				
			}

			xoutsignal[c] = clamp(xoutsignal[c]/2.f,-5.f,5.f);
			youtsignal[c] = clamp(youtsignal[c]/2.f,-5.f,5.f);

			outputs[X_OUTPUT].setVoltage(xoutsignal[c],c);
			outputs[Y_OUTPUT].setVoltage(youtsignal[c],c);
		}
		outputs[X_OUTPUT].setChannels(channels);
		outputs[Y_OUTPUT].setChannels(channels);
        
	}
};

struct WarblerWidget : ModuleWidget {
	WarblerWidget(WarblerModule* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WarblerPlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6, 19.)), module, WarblerModule::NOISE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, 17.)), module, WarblerModule::RGAIN_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6, 38.)), module, WarblerModule::DETUNE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, 36.)), module, WarblerModule::DGAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6, 57.)), module, WarblerModule::GAIN_PARAM));	
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, 55.)), module, WarblerModule::GGAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6, 76.)), module, WarblerModule::HARMN_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, 74.)), module, WarblerModule::HGAIN_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17, 17)), module, WarblerModule::NOISE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17, 36)), module, WarblerModule::DETUNE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17, 55)), module, WarblerModule::GAIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17, 74)), module, WarblerModule::HARMN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 95)), module, WarblerModule::PITCH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(21, 95)), module, WarblerModule::EXT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10, 114)), module, WarblerModule::X_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21, 114)), module, WarblerModule::Y_OUTPUT));



	}
};


Model* modelWarbler = createModel<WarblerModule, WarblerWidget>("Warbler");