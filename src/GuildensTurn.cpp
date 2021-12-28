/*
 * Copyright (c) 2020 Marshall Hampton <contact hamptonio at gmail.com>
 */
#include "plugin.hpp"
#include <array>

struct GuildensTurn : Module {

	enum ParamIds {
        PAD_PARAM,
        PAB_PARAM,
        PBA_PARAM,
        PBC_PARAM,
        PCB_PARAM,
        PCD_PARAM,
        PDC_PARAM,
        PDA_PARAM,

        aPAD_PARAM,
        aPAB_PARAM,
        aPBA_PARAM,
        aPBC_PARAM,
        aPCB_PARAM,
        aPCD_PARAM,
        aPDC_PARAM,
        aPDA_PARAM,

		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
        A_INPUT,
        B_INPUT,
        C_INPUT,
        D_INPUT,
        PAD_INPUT,
        PAB_INPUT,
        PBA_INPUT,
        PBC_INPUT,
        PCB_INPUT,
        PCD_INPUT,
        PDC_INPUT,
        PDA_INPUT,
        NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
        STATE_OUTPUT,
		NUM_OUTPUTS
	};

    float state[16] = {1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f,1.f}; //1=A, 2=B, 3=C, 4=D
    int indexs[4] = {A_INPUT,B_INPUT,C_INPUT,D_INPUT};
    int prob_inds[8] = {PAD_PARAM,PAB_PARAM,PBA_PARAM,PBC_PARAM,PCB_PARAM,PCD_PARAM,PDC_PARAM,PDA_PARAM};
    int att_inds[8] =  {aPAD_PARAM,aPAB_PARAM,aPBA_PARAM,aPBC_PARAM,aPCB_PARAM,aPCD_PARAM,aPDC_PARAM,aPDA_PARAM};
    int inprob_inds[8] = {PAD_INPUT,PAB_INPUT,PBA_INPUT,PBC_INPUT,PCB_INPUT,PCD_INPUT,PDC_INPUT,PDA_INPUT};
	std::array<rack::dsp::SchmittTrigger,16> inputTrigger;

	GuildensTurn() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam(PAB_PARAM, 0.f, 1.f, 0.333f, "A->B transition probability");
        configParam(PAD_PARAM, 0.f, 1.f, 0.333f, "A->D transition probability");
        configParam(PBA_PARAM, 0.f, 1.f, 0.333f, "B->A transition probability");
        configParam(PBC_PARAM, 0.f, 1.f, 0.333f, "B->C transition probability");
        configParam(PCB_PARAM, 0.f, 1.f, 0.333f, "C->B transition probability");
        configParam(PCD_PARAM, 0.f, 1.f, 0.333f, "C->D transition probability");
        configParam(PDC_PARAM, 0.f, 1.f, 0.333f, "D->C transition probability");
        configParam(PDA_PARAM, 0.f, 1.f, 0.333f, "D->A transition probability");

		configParam(aPAB_PARAM, 0.f, 2.f, 1.f, "A->B transition probability external attenuation");
        configParam(aPAD_PARAM, 0.f, 2.f, 1.f, "A->D transition probability external attenuation");
        configParam(aPBA_PARAM, 0.f, 2.f, 1.f, "B->A transition probability external attenuation");
        configParam(aPBC_PARAM, 0.f, 2.f, 1.f, "B->C transition probability external attenuation");
        configParam(aPCB_PARAM, 0.f, 2.f, 1.f, "C->B transition probability external attenuation");
        configParam(aPCD_PARAM, 0.f, 2.f, 1.f, "C->D transition probability external attenuation");
        configParam(aPDC_PARAM, 0.f, 2.f, 1.f, "D->C transition probability external attenuation");
        configParam(aPDA_PARAM, 0.f, 2.f, 1.f, "D->A transition probability external attenuation");
        
        configInput(TRIG_INPUT, "Trigger for state transition");
        configInput(A_INPUT, "Signal routed through state A");
        configInput(B_INPUT, "Signal routed through state B");
        configInput(C_INPUT, "Signal routed through state C");
        configInput(D_INPUT, "Signal routed through state D");
        configInput(PAD_INPUT, "Modulate A->D transition probability");
        configInput(PAB_INPUT, "Modulate A->B transition probability");
        configInput(PBA_INPUT, "Modulate B->A transition probability");
        configInput(PBC_INPUT, "Modulate B->C transition probability");
        configInput(PCB_INPUT, "Modulate C->B transition probability");
        configInput(PCD_INPUT, "Modulate C->D transition probability");
        configInput(PDC_INPUT, "Modulate D->C transition probability");
        configInput(PDA_INPUT, "Modulate D->A transition probability");

        configOutput(OUT_OUTPUT, "Routed signal");
        configOutput(STATE_OUTPUT, "Current state (A=1,B=2,C=3,D=4 volts)");
	}

	void process(const ProcessArgs& args) override {

		int channels = std::max(inputs[TRIG_INPUT].getChannels(),
        std::max(inputs[A_INPUT].getChannels(),
        std::max(inputs[B_INPUT].getChannels(),
        std::max(inputs[C_INPUT].getChannels(),
        std::max(inputs[D_INPUT].getChannels(),
        1)))));
        for (int c = 0; c < channels; c++) {
            int index = (int) (round(state[c])-1); // 0 to 3
            index = clamp(index,0,3);
            if (inputTrigger[c].process(inputs[TRIG_INPUT].getVoltage(c))){

                float Pback = params[prob_inds[index*2]].getValue() + params[att_inds[index*2]].getValue()*inputs[inprob_inds[index*2]].getVoltage(c);
                float Pforward = params[prob_inds[index*2+1]].getValue() + params[att_inds[index*2+1]].getValue()*inputs[inprob_inds[index*2+1]].getVoltage(c);
                Pback = clamp(Pback,0.f,1.f);
                Pforward = clamp(Pforward,0.f,1.f);
                float Pfb = std::max(Pback + Pforward,1.f);
                Pback = Pback/Pfb;
                Pforward = Pforward/Pfb;

                float Tr = random::uniform(); 
                if (Tr < Pforward){
                    index += 1;
                    if (index > 3){
                        index = 0;     
                    }
                } else if (Tr < (Pforward+Pback)){
                    index = index - 1;
                    if (index < 0){
                        index = 3;     
                    }
                } 
            }
            state[c] = (float) index + 1.f;
            outputs[STATE_OUTPUT].setVoltage(state[c],c);
            int input_ind = indexs[index];
            outputs[OUT_OUTPUT].setVoltage(inputs[input_ind].getVoltage(c),c);
	    }   
        outputs[STATE_OUTPUT].setChannels(channels);   
        outputs[OUT_OUTPUT].setChannels(channels);  
    }
};



struct GuildensTurnWidget : ModuleWidget {
	GuildensTurnWidget(GuildensTurn* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GuildensTurnPlate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 16.0)), module, GuildensTurn::PAD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 27.0)), module, GuildensTurn::PAB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 16.0+25)), module, GuildensTurn::PBA_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 27.0+25)), module, GuildensTurn::PBC_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 16.0+50)), module, GuildensTurn::PCB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 27.0+50)), module, GuildensTurn::PCD_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 16.0+75)), module, GuildensTurn::PDC_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.5, 27.0+75)), module, GuildensTurn::PDA_PARAM));

        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 12.0+2)), module, GuildensTurn::aPAD_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 27.0+2)), module, GuildensTurn::aPAB_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 12.0+2+25)), module, GuildensTurn::aPBA_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 27.0+2+25)), module, GuildensTurn::aPBC_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 12.0+2+50)), module, GuildensTurn::aPCB_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 27.0+2+50)), module, GuildensTurn::aPCD_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 12.0+2+75)), module, GuildensTurn::aPDC_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(34+2, 27.0+2+75)), module, GuildensTurn::aPDA_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 21.5)), module, GuildensTurn::A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 17)), module, GuildensTurn::PAD_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 26)), module, GuildensTurn::PAB_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 21.5+25)), module, GuildensTurn::B_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 17+25)), module, GuildensTurn::PBA_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 26+25)), module, GuildensTurn::PBC_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 21.5+50)), module, GuildensTurn::C_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 17+50)), module, GuildensTurn::PCB_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 26+50)), module, GuildensTurn::PCD_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6, 21.5+75)), module, GuildensTurn::D_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 17+75)), module, GuildensTurn::PDC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 26+75)), module, GuildensTurn::PDA_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.5, 120)), module, GuildensTurn::TRIG_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32, 120)), module, GuildensTurn::OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34.5, 120)), module, GuildensTurn::STATE_OUTPUT));



	}
};


Model* modelGuildensTurn = createModel<GuildensTurn, GuildensTurnWidget>("GuildensTurn");