/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"

struct RosslerRustlerModule : Module 
{
    enum ParamIds {
        A_PARAM,
        B_PARAM,
        C_PARAM,
		EXT_GAIN_PARAM,
		EXT_MIX_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
		PITCH_INPUT,
		EXT_INPUT,
		NUM_INPUTS
	};
    enum OutputIds {
		X_OUTPUT,
		// Y_OUTPUT,
		// Z_OUTPUT,
		NUM_OUTPUTS
	};

    float xout[16] = {0};
	float yout[16] = {0};
	float zout[16] = {0};
	int mProcMode = 1; 
    RosslerRustlerModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(A_PARAM, 0.f, 1.f, 0.2f, "A dynamical parameter");
		configParam(B_PARAM, 0.f, 1.f, 0.2f, "B dynamical parameter");
		configParam(C_PARAM, 0.f, 30.f, 5.7f, "C dynamical parameter");
		configParam(EXT_GAIN_PARAM, 0.f, 10.f, 1.f, "External Gain");
		configParam(EXT_MIX_PARAM, 0.f, 1.f, 0.5f, "Internal/External Mix");

		configInput(PITCH_INPUT, "Set pitch V/oct");
		configInput(EXT_INPUT, "External signal");

		configOutput(X_OUTPUT, "X component of Rossler system");

		for (int c = 0; c < 16; c++){
			xout[c] = 0.f;
			yout[c] = 5.f;
			zout[c] = 0.f;
		}
		
	};
	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "procmode", json_integer(mProcMode));
		return rootJ;
	}
	void dataFromJson(json_t *rootJ) override {
		if (rootJ == nullptr)
			return;
		json_t *procmodej = json_object_get(rootJ, "procmode");
		if (procmodej)
		{
			mProcMode = json_integer_value(procmodej);
		}
	}
	void RosslerSlope(float x, float y, float z,float a, float b, float c, float pert, float* output){
		output[0] = -y-z;
		output[1] = x + a*y + pert;
		output[2] = b + z*(x-c);
	}

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[PITCH_INPUT].getChannels(),1);

		float A = params[A_PARAM].getValue();
		float B = params[B_PARAM].getValue();
		float C = params[C_PARAM].getValue();
		float gain = params[EXT_GAIN_PARAM].getValue();
		float mix = params[EXT_MIX_PARAM].getValue();

		for (int c = 0; c < channels; c++) {
			float pitch = inputs[PITCH_INPUT].getVoltage(c);
			pitch = dsp::FREQ_C4 * std::pow(2.f, pitch)*6.2831853f;
			float dt = args.sampleTime * pitch/2.0f;
			float ext = inputs[EXT_INPUT].getVoltage(c);
			float k[3];
			RosslerSlope(xout[c], yout[c], zout[c], A, B, C, ext*gain,k);
			float k2[3];
			RosslerSlope(xout[c] + k[0] * dt, yout[c] + k[1] * dt, zout[c] + k[2] * dt, A, B, C, ext*gain,k2);
			if (mProcMode == 1)
			{
				xout[c] += (k[0] + k2[0] )* dt;
				yout[c] += (k[1] + k2[1] )* dt;
				zout[c] += (k[2] + k2[2] )* dt;
			} else // old behavior where the k and k2 pointers were both pointing to the same array
			{
				xout[c] += (k2[0] + k2[0] )* dt;
				yout[c] += (k2[1] + k2[1] )* dt;
				zout[c] += (k2[2] + k2[2] )* dt;
			}
			

			xout[c] = clamp(xout[c],-20.f,20.f);
			yout[c] = clamp(yout[c],-20.f,20.f);
			zout[c] = clamp(zout[c],-20.f,20.f);


			outputs[X_OUTPUT].setVoltage(xout[c]/3.0f*(1-mix) + mix*ext,c);
			// outputs[Y_OUTPUT].setVoltage(yout[c]/3.0f,c);
			// outputs[Z_OUTPUT].setVoltage(zout[c]/3.0 - 3.5f,c);
		}
		outputs[X_OUTPUT].setChannels(channels);
		// outputs[Y_OUTPUT].setChannels(channels);
		// outputs[Z_OUTPUT].setChannels(channels);
        
	}
};

struct RosslerRustlerWidget : ModuleWidget {
	float xc = 6.0;
	float xc2 = 14.64;
	RosslerRustlerWidget(RosslerRustlerModule* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RosslerRustlerPlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 20.)), module, RosslerRustlerModule::A_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc2, 34.)), module, RosslerRustlerModule::B_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 48.)), module, RosslerRustlerModule::C_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc2, 62.)), module, RosslerRustlerModule::EXT_GAIN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 76.)), module, RosslerRustlerModule::EXT_MIX_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 90)), module, RosslerRustlerModule::PITCH_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc, 104)), module, RosslerRustlerModule::EXT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc2, 118)), module, RosslerRustlerModule::X_OUTPUT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.32, 88)), module, RosslerRustlerModule::Y_OUTPUT));
		// addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.32, 102)), module, RosslerRustlerModule::Z_OUTPUT));

	}
	void appendContextMenu(Menu *menu) override {
		RosslerRustlerModule *module = dynamic_cast<RosslerRustlerModule*>(this->module);
		
		struct ModeMenuItem : MenuItem
		{
			RosslerRustlerModule* module = nullptr;
			
			void onAction(const event::Action &e) override
			{
				if (module->mProcMode == 0)
					module->mProcMode = 1;
				else module->mProcMode = 0;
			}
		};
		ModeMenuItem *modeItem = createMenuItem<ModeMenuItem>("Updated processing behavior", CHECKMARK(module->mProcMode));
		modeItem->module = module;
		menu->addChild(modeItem);

	}
};


Model* modelRosslerRustler = createModel<RosslerRustlerModule, RosslerRustlerWidget>("RosslerRustler");