#include "plugin.hpp"


using simd::float_4;

template <typename T>
static T clip(T x) {
	// return std::tanh(x);
	// Pade approximant of tanh
	x = simd::clamp(x, -3.f, 3.f);
	return x * (27 + x * x) / (27 + 9 * x * x);
}

template <typename T>
struct Rossler {
	T Ap = 0.1;
	T Bp = 0.2;
	T Cp = 5.7;
	T Fs = 1.0;
	T state[3];
	T input;
	T gain = 1.0;

	Rossler() {
		reset();
	}

	void reset() {
		for (int i = 0; i < 3; i++) {
			state[i] = 0.5;
		}
	}

	void process(T input, T dt) {
		dsp::stepRK4(T(0), dt, state, 3, [&](T t, const T x[], T dxdt[]) {
			
			//dxdt[0] = Fs*(-x[1]-x[2] - x[0]*x[0]*x[0]/5) + gain*input;
			//dxdt[1] = Fs*(x[0] + Ap*x[1] - x[1]*x[1]*x[1]/5);
			//dxdt[2] = Fs*(Bp + x[2]*(Fs*x[0]-Cp) - x[2]*x[2]*x[2]/5);
			dxdt[0] = Fs*(x[1] - (x[0]*x[0] + x[1]*x[1]));
			dxdt[1] = Fs*(-x[0]);
			dxdt[2] = input*gain;
		});

		for (int i = 0; i < 3; i++) {
			state[i] = clip(state[i]);
		}
		this->input = input;
	}

	T xout() {
		return state[0];
	}
	T zout() {
		//return clip(state2[0]);
		return state[2];
	}
};


//static const int UPSAMPLE = 2;

struct Dynamo : Module {
	enum ParamIds {
		FREQ_PARAM,
		FS_PARAM,
		RES_PARAM,
		FREQ_CV_PARAM,
		DRIVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FREQ_INPUT,
		RES_INPUT,
		DRIVE_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		XOUT,
		ZOUT,
		NUM_OUTPUTS
	};

	Rossler<float_4> filters[4];

	Dynamo() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		// Multiply and offset for backward patch compatibility
		configParam(FREQ_PARAM, 0.f, 1.f, 0.5f, "Frequency", " Hz", std::pow(2, 10.f), dsp::FREQ_C4 / std::pow(2, 5.f));
		configParam(FS_PARAM, 0.f, 1.f, 0.5f, "Fs");
		configParam(RES_PARAM, 0.f, 1.f, 0.f, "Resonance", "%", 0.f, 100.f);
		configParam(FREQ_CV_PARAM, -1.f, 1.f, 0.f, "Frequency modulation", "%", 0.f, 100.f);
		configParam(DRIVE_PARAM, 0.f, 1.f, 0.f, "Drive", "", 0, 11);
	}

	void onReset() override {
		for (int i = 0; i < 4; i++)
			filters[i].reset();
	}

	void process(const ProcessArgs& args) override {
		if (!outputs[XOUT].isConnected() && !outputs[ZOUT].isConnected()) {
			return;
		}

		float driveParam = params[DRIVE_PARAM].getValue();
		float resParam = params[RES_PARAM].getValue();
		float fsParam = params[FS_PARAM].getValue();
		fsParam = dsp::quadraticBipolar(fsParam * 2.f - 1.f) * 7.f / 12.f;
		float freqCvParam = params[FREQ_CV_PARAM].getValue();
		freqCvParam = dsp::quadraticBipolar(freqCvParam);
		float freqParam = params[FREQ_PARAM].getValue();
		freqParam = freqParam * 10.f - 5.f;

		int channels = std::max(1, inputs[IN_INPUT].getChannels());

		for (int c = 0; c < channels; c += 4) {
			auto* filter = &filters[c / 4];

			float_4 input = float_4::load(inputs[IN_INPUT].getVoltages(c)) / 5.f;

			// Get pitch
			float_4 pitch = freqParam + fsParam + inputs[FREQ_INPUT].getPolyVoltageSimd<float_4>(c) * freqCvParam;

			filter->gain = driveParam*1000;
			filter->Fs = 1 + fsParam*800;
			// Set outputs
			filter->process(input, args.sampleTime);
			float_4 xout = 5.f * filter->xout();
			xout.store(outputs[XOUT].getVoltages(c));
			float_4 zout = 5.f * filter->zout();
			zout.store(outputs[ZOUT].getVoltages(c));
		}

		outputs[XOUT].setChannels(channels);
		outputs[ZOUT].setChannels(channels);
	}
};


struct DynamoWidget : ModuleWidget {
	DynamoWidget(Dynamo* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Dynamo.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		addParam(createParam<RoundHugeBlackKnob>(Vec(33, 61), module, Dynamo::FREQ_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(12, 143), module, Dynamo::FS_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(71, 143), module, Dynamo::RES_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(12, 208), module, Dynamo::FREQ_CV_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(71, 208), module, Dynamo::DRIVE_PARAM));

		addInput(createInput<PJ301MPort>(Vec(10, 276), module, Dynamo::FREQ_INPUT));
		addInput(createInput<PJ301MPort>(Vec(48, 276), module, Dynamo::RES_INPUT));
		addInput(createInput<PJ301MPort>(Vec(85, 276), module, Dynamo::DRIVE_INPUT));
		addInput(createInput<PJ301MPort>(Vec(10, 320), module, Dynamo::IN_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(48, 320), module, Dynamo::XOUT));
		addOutput(createOutput<PJ301MPort>(Vec(85, 320), module, Dynamo::ZOUT));
	}
};


Model* modelDynamo = createModel<Dynamo, DynamoWidget>("Dynamo");
