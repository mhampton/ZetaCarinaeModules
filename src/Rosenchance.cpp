/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"
#include <array>

struct Rosenchance : Module {

	enum ParamIds {
		PA_PARAM,
        PAE1_PARAM,
        AE1_PARAM,
        AE2_PARAM,
        PB_PARAM,
        PBE1_PARAM,
        BE1_PARAM,
        BE2_PARAM,

        aPA_PARAM,
        aPAE1_PARAM,
        aAE1_PARAM,
        aAE2_PARAM,
        aPB_PARAM,
        aPBE1_PARAM,
        aBE1_PARAM,
        aBE2_PARAM,

		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
        PA_INPUT,
        PAE1_INPUT,
        AE1_INPUT,
        AE2_INPUT,
        PB_INPUT,
        PBE1_INPUT,
        BE1_INPUT,
        BE2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
        STATE_OUTPUT,
        A_OUTPUT,
        B_OUTPUT,
		NUM_OUTPUTS
	};

    float state[16] = {1.f}; //1=A, 2=B
    int counter = 0;
	std::array<rack::dsp::SchmittTrigger,16> inputTrigger;

	Rosenchance() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(PA_PARAM, 0.f, 1.f, 0.5f, "A->A transition probability");
        configParam(PB_PARAM, 0.f, 1.f, 0.5f, "B->B transition probability");
        configParam(PAE1_PARAM, 0.f, 1.f, 0.5f, "A's e1 emission probability");
        configParam(AE1_PARAM, -10.f, 10.f, 0.f, "A's e1 emission value");
        configParam(AE2_PARAM, -10.f, 10.f, 1.f, "A's e2 emission value");
        configParam(PBE1_PARAM, 0.f, 1.f, 0.5f, "B's e1 emission probability");
        configParam(BE1_PARAM, -10.f, 10.f, 2.f, "B's e1 emission value");
        configParam(BE2_PARAM, -10.f, 10.f, 3.f, "B's e2 emission value");

        configParam(aPA_PARAM, 0.f, 1.f, 1.f, "A->A transition probability external attenuation");
        configParam(aPB_PARAM, 0.f, 1.f, 1.f, "B->B transition probability external attenuation");
        configParam(aPAE1_PARAM, 0.f, 1.f, 1.f, "A's e1 emission probability external attenuation");
        configParam(aAE1_PARAM, -10.f, 10.f, 1.f, "A's e1 emission value external attenuation");
        configParam(aAE2_PARAM, -10.f, 10.f, 1.f, "A's e2 emission value external attenuation");
        configParam(aPBE1_PARAM, 0.f, 1.f, 1.f, "B's e1 emission probability external attenuation");
        configParam(aBE1_PARAM, -10.f, 10.f, 1.f, "B's e1 emission value external attenuation");
        configParam(aBE2_PARAM, -10.f, 10.f, 1.f, "B's e2 emission value external attenuation");
		//configParam(TIME_PARAM, -10.f, 10.f, 1.f, "Time","", 2.0);

        configInput(TRIG_INPUT, "Trigger for state transition (and emission)");
        configInput(PA_INPUT, "Modulation of A->A state transition probability");
        configInput(PAE1_INPUT, "Modulation of state A's e1 emission probability");
        configInput(AE1_INPUT, "Modulation of state A's  e1 emission value");
        configInput(AE2_INPUT, "Modulation of state A's e2 emission value");
        configInput(PB_INPUT, "Modulation of B->B state transition probability");
        configInput(PBE1_INPUT, "Modulation of state B's e1 emission probability");
        configInput(BE1_INPUT, "Modulation of state B's  e1 emission value");
        configInput(BE2_INPUT, "Modulation of state B's  e2 emission value");

        configOutput(OUT_OUTPUT, "Emission value");
        configOutput(STATE_OUTPUT, "Current state (A=1, B=2 Volts)");
        configOutput(A_OUTPUT, "Triggers when entering state A");
        configOutput(B_OUTPUT, "Triggers when entering state B");
	}

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[TRIG_INPUT].getChannels(),1);
        for (int c = 0; c < channels; c++) {
            if (inputTrigger[c].process(inputs[TRIG_INPUT].getVoltage(c))){
                float PA = params[PA_PARAM].getValue() + params[aPA_PARAM].getValue()*inputs[PA_INPUT].getVoltage(c);
                float PB = params[PB_PARAM].getValue() + params[aPB_PARAM].getValue()*inputs[PB_INPUT].getVoltage(c);
                float PAE1 = params[PAE1_PARAM].getValue() + params[aPAE1_PARAM].getValue()*inputs[PAE1_INPUT].getVoltage(c);
                float PBE1 = params[PBE1_PARAM].getValue() + params[aPBE1_PARAM].getValue()*inputs[PBE1_INPUT].getVoltage(c);
                float AE1 = params[AE1_PARAM].getValue() + params[aAE1_PARAM].getValue()*inputs[AE1_INPUT].getVoltage(c);
                float BE1 = params[BE1_PARAM].getValue() + params[aBE1_PARAM].getValue()*inputs[BE1_INPUT].getVoltage(c);
                float AE2 = params[AE2_PARAM].getValue() + params[aAE2_PARAM].getValue()*inputs[AE2_INPUT].getVoltage(c);
                float BE2 = params[BE2_PARAM].getValue() + params[aBE2_PARAM].getValue()*inputs[BE2_INPUT].getVoltage(c);
               
                float Tr = random::uniform(); 
                float Er = random::uniform(); 
                if ((round(state[c])==1 && Tr < PA) || (round(state[c])==2 && Tr > PB)){ // now in state A
                    state[c] = 1.f;
                    outputs[STATE_OUTPUT].setVoltage(1.0f,c);
                    outputs[A_OUTPUT].setVoltage(5.0f,c);
                    outputs[B_OUTPUT].setVoltage(0.f,c);
                    
                    if (Er < PAE1){
                        outputs[OUT_OUTPUT].setVoltage(AE1,c);
                    }
                    else{
                        outputs[OUT_OUTPUT].setVoltage(AE2,c);
                    }
                }
                else{ // state is B
                    state[c] = 2.f;
                    outputs[STATE_OUTPUT].setVoltage(2.0f,c);
                    outputs[A_OUTPUT].setVoltage(0.f,c);
                    outputs[B_OUTPUT].setVoltage(5.f,c);
                    if (Er < PBE1){
                        outputs[OUT_OUTPUT].setVoltage(BE1,c);
                    }
                    else{
                        outputs[OUT_OUTPUT].setVoltage(BE2,c);
                    }
                }
            
            }
            else{
                counter += 1;
                if (counter > 10){
                    counter = 0;
                    outputs[A_OUTPUT].setVoltage(0.f,c);
                    outputs[B_OUTPUT].setVoltage(0.f,c);
                }
            }
        }
		outputs[STATE_OUTPUT].setChannels(channels);   
        outputs[OUT_OUTPUT].setChannels(channels);  
        outputs[A_OUTPUT].setChannels(channels);  
        outputs[B_OUTPUT].setChannels(channels); 
	}
};



struct RosenchanceWidget : ModuleWidget {
	RosenchanceWidget(Rosenchance* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RosenchancePlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 15.0)), module, Rosenchance::PA_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 26.0)), module, Rosenchance::PAE1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 37.0)), module, Rosenchance::AE1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 48.0)), module, Rosenchance::AE2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 59.0)), module, Rosenchance::PB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 70.0)), module, Rosenchance::PBE1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 81.0)), module, Rosenchance::BE1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.0, 92.0)), module, Rosenchance::BE2_PARAM));

        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 13.0)), module, Rosenchance::aPA_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 24.0)), module, Rosenchance::aPAE1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 35.0)), module, Rosenchance::aAE1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 46.0)), module, Rosenchance::aAE2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 57.0)), module, Rosenchance::aPB_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 68.0)), module, Rosenchance::aPBE1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 79.0)), module, Rosenchance::aBE1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(16.75, 90.0)), module, Rosenchance::aBE2_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 15)), module, Rosenchance::PA_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 26)), module, Rosenchance::PAE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 37)), module, Rosenchance::AE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 48)), module, Rosenchance::AE2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 59)), module, Rosenchance::PB_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 70)), module, Rosenchance::PBE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 81)), module, Rosenchance::BE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, 92)), module, Rosenchance::BE2_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.5, 102)), module, Rosenchance::TRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.5, 111)), module, Rosenchance::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23, 111)), module, Rosenchance::STATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6, 120)), module, Rosenchance::A_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25, 120)), module, Rosenchance::B_OUTPUT));



	}
};


Model* modelRosenchance = createModel<Rosenchance, RosenchanceWidget>("Rosenchance");