/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"

struct FireflyModule : Module 
{
    enum ParamIds {
        F1R_PARAM,
        F2R_PARAM,
        F3R_PARAM,
        F4R_PARAM,
        F5R_PARAM,
        W1_PARAM,
        W2_PARAM,
        W3_PARAM,
        W4_PARAM,
        W5_PARAM,
        CH1_PARAM,
        CH2_PARAM,
        CH3_PARAM,
        CH4_PARAM,
        CH5_PARAM,
        K_PARAM,
        KTYPE_PARAM,
        FM_PARAM,
        GAIN_PARAM,
        NUM_PARAMS
    };

    enum InputIds {
		F1R_INPUT,
        F2R_INPUT,
        F3R_INPUT,
        F4R_INPUT,
        F5R_INPUT,
        W1_INPUT,
        W2_INPUT,
        W3_INPUT,
        W4_INPUT,
        W5_INPUT,
        K_INPUT,
        KTYPE_INPUT,
        FM_INPUT,
        GAIN_INPUT,
		VOCT_INPUT,
		NUM_INPUTS
	};

    enum OutputIds {
		SM_OUTPUT,
		NUM_OUTPUTS
	};

    float theta[16][5] = {{0}};
    float out[16] = {0};
	float nt = 6.2831853f;
    float npi = 3.14159265;
    float waves[11][7200] = {{0}};
    float Kcurves[2][102] = {{0}};
    int ctlcount = 121;
    int wind1s[16][5] = {{0}};
    int wind2s[16][5] = {{1}};
    float winners[16][5] = {{0}};
    float C1p = 1.0f;
    float C2p = 1.0f;
    float C3p = 1.0f;
    float C4p = 1.0f;
    float C5p = 1.0f;
    float gradus[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 24, 25, 27};

    FireflyModule() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(F1R_PARAM, 0.f,  10.f, 1.0f, "Freq. Ratio 1"); 
        configParam(F2R_PARAM, 0.f,  10.f, 1.01f, "Freq. Ratio 2"); 
        configParam(F3R_PARAM, 0.f,  10.f, 0.99f, "Freq. Ratio 3"); 
        configParam(F4R_PARAM, 0.f,  10.f, 0.5f, "Freq. Ratio 4"); 
        configParam(F5R_PARAM, 0.f,  10.f, 3.0f, "Freq. Ratio 5"); 

        configParam(W1_PARAM, 0.f,  10.f, 0.0f, "Wave type 1"); 
        configParam(W2_PARAM, 0.f,  10.f, 0.0f, "Wave type 2"); 
        configParam(W3_PARAM, 0.f,  10.f, 0.0f, "Wave type 3"); 
        configParam(W4_PARAM, 0.f,  10.f, 1.0f, "Wave type 4"); 
        configParam(W5_PARAM, 0.f,  10.f, 5.0f, "Wave type 5"); 

        configParam(CH1_PARAM, 0.f,  2.f, 1.25f, "Charm 1"); 
        configParam(CH2_PARAM, 0.f,  2.f, 1.0f, "Charm 2"); 
        configParam(CH3_PARAM, 0.f,  2.f, 1.0f, "Charm 3"); 
        configParam(CH4_PARAM, 0.f,  2.f, 0.5f, "Charm 4"); 
        configParam(CH5_PARAM, 0.f,  2.f, 0.25f, "Charm 5"); 

        configParam(K_PARAM, -0.2f,  0.2f, 0.01f, "Coupling Strength"); 
        configParam(KTYPE_PARAM, 0.f,  1.f, 0.0f, "Coupling Type"); 
        configParam(FM_PARAM, 0.f,  1.f, 0.0f, "FM Index"); 
        configParam(GAIN_PARAM, 0.f,  10.f, 1.0f, "Gain"); 

        configInput(F1R_INPUT, "Add to Freq. ratio 1");
        configInput(F2R_INPUT, "Add to Freq. ratio 2");
        configInput(F3R_INPUT, "Add to Freq. ratio 3");
        configInput(F4R_INPUT, "Add to Freq. ratio 4");
        configInput(F5R_INPUT, "Add to Freq. ratio 5");
        configInput(W1_INPUT, "Wavetable 1 modulation");
        configInput(W2_INPUT, "Wavetable 2 modulation");
        configInput(W3_INPUT, "Wavetable 3 modulation");
        configInput(W4_INPUT, "Wavetable 4 modulation");
        configInput(W5_INPUT, "Wavetable 5 modulation");
        configInput(K_INPUT, "Coupling constant modulation");
        configInput(KTYPE_INPUT, "Modulate coupling curve morph parameter");
        configInput(FM_INPUT, "FM V/oct");
        configInput(GAIN_INPUT, "Gain modulation (additive)");
		configInput(VOCT_INPUT, "V/oct reference frequency");

        configOutput(SM_OUTPUT, "Combined signal");

        // initialize coupling curves
		for (int i=0; i<102; i++){
			    Kcurves[0][i] = std::sin((i-50.f)*npi/50.f);
                Kcurves[1][i] = std::sin(2.0f*(i-50.f)*npi/50.f);
        }

        // initialize phases
		for (int c = 0; c < 16; c++){
            for (int i=0; i<5; i++){
			    theta[c][i] = 0.f;
            }
		}
        // initialize waveform lookup tables
        float wave_amps[11][20] ={{1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.689,0.228,0.064,0.015,0.003,0.001,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000},
{0.358,0.251,0.164,0.103,0.061,0.034,0.017,0.008,0.003,0.001,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000},
{0.262,0.213,0.159,0.117,0.085,0.061,0.042,0.027,0.017,0.009,0.005,0.002,0.001,0.000,0.000,0.000,0.000,0.000,0.000,0.000},
{0.211,0.192,0.151,0.116,0.092,0.070,0.053,0.039,0.029,0.020,0.013,0.008,0.004,0.002,0.001,0.000,0.000,0.000,0.000,0.000},
{0.171,0.178,0.149,0.114,0.093,0.076,0.059,0.046,0.036,0.027,0.019,0.014,0.009,0.005,0.003,0.001,0.000,0.000,0.000,0.000},
{0.136,0.161,0.153,0.116,0.091,0.079,0.065,0.050,0.040,0.032,0.024,0.018,0.013,0.009,0.006,0.003,0.002,0.001,0.000,0.000},
{0.106,0.137,0.155,0.126,0.092,0.079,0.071,0.056,0.043,0.035,0.029,0.022,0.016,0.012,0.009,0.006,0.003,0.002,0.001,0.000},
{0.084,0.108,0.147,0.141,0.100,0.076,0.072,0.064,0.049,0.038,0.032,0.026,0.020,0.015,0.011,0.008,0.005,0.003,0.001,0.000},
{0.071,0.079,0.127,0.150,0.119,0.079,0.067,0.068,0.058,0.042,0.033,0.028,0.024,0.018,0.013,0.010,0.007,0.004,0.002,0.001},
{0.067,0.041,0.069,0.126,0.152,0.117,0.070,0.057,0.063,0.060,0.045,0.031,0.026,0.024,0.019,0.014,0.009,0.007,0.004,0.002}
        };
        for (int i=0; i<7200; i++){
            for (int k=0; k<11; k++){
                waves[k][i] = 0.f;
                for (int j=0; j<20; j++){
                    waves[k][i] += std::sin(nt * i * (gradus[j]+1.f)/7200.0f + j/10.0)*wave_amps[k][j];
                }
            }
        }
	};

    void ctrl_process(){
        int channels = 
            std::max(inputs[F1R_INPUT].getChannels(),
            std::max(inputs[F2R_INPUT].getChannels(),
            std::max(inputs[F3R_INPUT].getChannels(),
            std::max(inputs[F4R_INPUT].getChannels(),
            std::max(inputs[F5R_INPUT].getChannels(),
            std::max(inputs[VOCT_INPUT].getChannels(),
            std::max(inputs[W1_INPUT].getChannels(),1)))))));

        C1p = params[CH1_PARAM].getValue();
        C2p = params[CH2_PARAM].getValue();
        C3p = params[CH3_PARAM].getValue();
        C4p = params[CH4_PARAM].getValue();
        C5p = params[CH5_PARAM].getValue();

        float W1p = params[W1_PARAM].getValue();
        float W2p = params[W2_PARAM].getValue();
        float W3p = params[W3_PARAM].getValue();
        float W4p = params[W4_PARAM].getValue();
        float W5p = params[W5_PARAM].getValue();

        for (int c = 0; c < channels; c++) {
            float W1 = W1p + inputs[W1_INPUT].getVoltage(c);
            float W2 = W2p + inputs[W2_INPUT].getVoltage(c);
            float W3 = W3p + inputs[W3_INPUT].getVoltage(c);
            float W4 = W4p + inputs[W4_INPUT].getVoltage(c);
            float W5 = W5p * inputs[W5_INPUT].getVoltage(c);

            wind1s[c][0] = clamp((int) floor(W1), 0, 10);
            wind1s[c][1] = clamp((int) floor(W2), 0, 10);
            wind1s[c][2] = clamp((int) floor(W3), 0, 10);
            wind1s[c][3] = clamp((int) floor(W4), 0, 10);
            wind1s[c][4] = clamp((int) floor(W5), 0, 10);

            wind2s[c][0] = clamp((int) floor(W1)+1, 0, 10);
            wind2s[c][1] = clamp((int) floor(W2)+1, 0, 10);
            wind2s[c][2] = clamp((int) floor(W3)+1, 0, 10);
            wind2s[c][3] = clamp((int) floor(W4)+1, 0, 10);
            wind2s[c][4] = clamp((int) floor(W5)+1, 0, 10);

            winners[c][0] = clamp(W1 - floor(W1), 0.f, 1.f);
            winners[c][1] = clamp(W2 - floor(W2), 0.f, 1.f);
            winners[c][2] = clamp(W3 - floor(W3), 0.f, 1.f);
            winners[c][3] = clamp(W4 - floor(W4), 0.f, 1.f);
            winners[c][4] = clamp(W5 - floor(W5), 0.f, 1.f);
        }
    }

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[F1R_INPUT].getChannels(),
        std::max(inputs[VOCT_INPUT].getChannels(),1));

		float Kp = params[K_PARAM].getValue();
        float Ktype = params[KTYPE_PARAM].getValue();
        float F1Rp = params[F1R_PARAM].getValue();
        float F2Rp = params[F2R_PARAM].getValue();
        float F3Rp = params[F3R_PARAM].getValue();
        float F4Rp = params[F4R_PARAM].getValue();
        float F5Rp = params[F5R_PARAM].getValue();


        float FMp = params[FM_PARAM].getValue();

		float dt = args.sampleTime;
        float gainp = params[GAIN_PARAM].getValue();

        ctlcount += 1;
        if (ctlcount > 120){
            ctrl_process();
            ctlcount = 0;
        }

		for (int c = 0; c < channels; c++) {
            out[c] = 0.f;
            float ks[5];
            float voct = inputs[VOCT_INPUT].getVoltage(c);
            float F1 = F1Rp + inputs[F1R_INPUT].getVoltage(c);
            F1 = round(720*F1)/720.0f;
            float F2 = F2Rp + inputs[F2R_INPUT].getVoltage(c);
            F2 = round(720*F2)/720.0f;
            float F3 = F3Rp + inputs[F3R_INPUT].getVoltage(c);
            F3 = round(720*F3)/720.0f;
            float F4 = F4Rp + inputs[F4R_INPUT].getVoltage(c);
            F4 = round(720*F4)/720.0f;
            float F5 = F5Rp + inputs[F5R_INPUT].getVoltage(c);
            F5 = round(720*F5)/720.0f;
            float K = Kp + inputs[K_INPUT].getVoltage(c);
            float Kt = clamp(Ktype + inputs[KTYPE_INPUT].getVoltage(c), 0.f, 1.f);

            float FMI = FMp * inputs[FM_INPUT].getVoltage(c);
            float freq = dsp::FREQ_C4 * std::pow(2.f, voct*(1.0f + FMI))*nt;
            float gain = gainp + inputs[GAIN_INPUT].getVoltage(c);

            float wlist[5] = {F1*freq, F2*freq, F3*freq, F4*freq, F5*freq};
            float clist[5] = {C1p, C2p, C3p, C4p, C5p};
			for (int i = 0; i < 5; i++) {
                ks[i] = wlist[i];
                for (int j = 0; j < 5; j++) {
                    if (i!=j){
                        int kindex = 50 + 50*((theta[c][j] - theta[c][i])/nt);
                        float coupling = Kcurves[0][kindex]*Kt;
                        coupling += Kcurves[1][kindex]*(1.f - Kt);
                        ks[i] += clist[j] * wlist[i] * K * coupling;  
                    }
                }
                theta[c][i] += ks[i]*dt;
                theta[c][i] = theta[c][i] - std::floor(theta[c][i]/nt)*nt;
                int windex = (int) std::floor(7200*theta[c][i]/nt);
                out[c] += waves[wind1s[c][i]][windex]*(1.0f - winners[c][i])*clist[i];
                out[c] += waves[wind2s[c][i]][windex]* winners[c][i]*clist[i];
            }

			//outputs[SM_OUTPUT].setVoltage(out[c]*gain, c);
            outputs[SM_OUTPUT].setVoltage(clamp(out[c]*gain,-5.f,5.0f), c);
		}
		outputs[SM_OUTPUT].setChannels(channels);
        
	}
};

struct FireflyWidget : ModuleWidget {
	float xc = 7.0;
	float xc2 = 22.0;
    float xc3 = 37.0;
    float xc4 = 56.0;
    float xc5 = 71.0;
    float xc6 = 32.0;
    float xc7 = 47.0;
	FireflyWidget(FireflyModule* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/FireflyPlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 24.)), module, FireflyModule::F1R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 38.)), module, FireflyModule::F2R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 52.)), module, FireflyModule::F3R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 66.)), module, FireflyModule::F4R_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 80.)), module, FireflyModule::F5R_PARAM));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc3, 24.)), module, FireflyModule::CH1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc3, 38.)), module, FireflyModule::CH2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc3, 52.)), module, FireflyModule::CH3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc3, 66.)), module, FireflyModule::CH4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc3, 80.)), module, FireflyModule::CH5_PARAM));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc4, 24.)), module, FireflyModule::W1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc4, 38.)), module, FireflyModule::W2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc4, 52.)), module, FireflyModule::W3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc4, 66.)), module, FireflyModule::W4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc4, 80.)), module, FireflyModule::W5_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc, 99.)), module, FireflyModule::K_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc6, 99.)), module, FireflyModule::KTYPE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(58, 99.)), module, FireflyModule::FM_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(xc6, 116.)), module, FireflyModule::GAIN_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 24)), module, FireflyModule::F1R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 38)), module, FireflyModule::F2R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 52)), module, FireflyModule::F3R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 66)), module, FireflyModule::F4R_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2, 80)), module, FireflyModule::F5R_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc5, 24)), module, FireflyModule::W1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc5, 38)), module, FireflyModule::W2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc5, 52)), module, FireflyModule::W3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc5, 66)), module, FireflyModule::W4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc5, 80)), module, FireflyModule::W5_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2-3, 99)), module, FireflyModule::K_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44, 99)), module, FireflyModule::KTYPE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(70, 99)), module, FireflyModule::FM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc7, 116)), module, FireflyModule::GAIN_INPUT));

		//addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc2,100)), module, FireflyModule::EXT_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(xc,116)), module, FireflyModule::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(xc5, 116)), module, FireflyModule::SM_OUTPUT));

	}
};


Model* modelFirefly = createModel<FireflyModule, FireflyWidget>("Firefly");