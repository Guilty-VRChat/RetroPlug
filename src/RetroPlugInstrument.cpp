#include "RetroPlugInstrument.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "src/ui/EmulatorView.h"

RetroPlugInstrument::RetroPlugInstrument(IPlugInstanceInfo instanceInfo)
: IPLUG_CTOR(0, 0, instanceInfo)
{
	_sampleScratch = new float[1024 * 1024];

#if IPLUG_EDITOR
	mMakeGraphicsFunc = [&]() {
		return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
	};

	mLayoutFunc = [&](IGraphics* pGraphics) {
		//pGraphics->AttachCornerResizer(kUIResizerScale, false);
		pGraphics->AttachPanelBackground(COLOR_BLACK);
		pGraphics->HandleMouseOver(true);
		pGraphics->LoadFont("Roboto-Regular", ROBOTTO_FN);

	const IRECT b = pGraphics->GetBounds();
		float mid = b.H() / 2;
		IRECT topRow(b.L, mid - 25, b.R, mid);
		IRECT bottomRow(b.L, mid, b.R, mid + 25);

		pGraphics->AttachControl(new ITextControl(topRow, "Double click to", IText(23, COLOR_WHITE)));
		pGraphics->AttachControl(new ITextControl(bottomRow, "load a ROM...", IText(23, COLOR_WHITE)));
		pGraphics->AttachControl(new EmulatorView(b, &_plug));
	};
#endif
}

RetroPlugInstrument::~RetroPlugInstrument() {
	delete[] _sampleScratch;
}

#if IPLUG_DSP
void RetroPlugInstrument::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
	if (!_plug.active() || nFrames == 0) {
		return;
	}

	SameboyMin* plug = _plug.plug();
	MessageBus* bus = plug->messageBus();

	generateMidiClock(nFrames);

	size_t frameCount = nFrames;
	size_t sampleCount = frameCount * 2;
	size_t available = bus->audio.readAvailable();
	while (available < sampleCount) {
		plug->update();
		available = bus->audio.readAvailable();
	}

	memset(_sampleScratch, 0, sampleCount * sizeof(float));
	size_t readAmount = bus->audio.read(_sampleScratch, sampleCount);
	assert(readAmount == sampleCount);

	float gain = 1.f;

	for (size_t i = 0; i < frameCount; i++) {
		outputs[0][i] = _sampleScratch[i * 2] * gain;
		outputs[1][i] = _sampleScratch[i * 2 + 1] * gain;
	}
}

void RetroPlugInstrument::OnIdle() {

}

bool RetroPlugInstrument::SerializeState(IByteChunk & chunk) const {
	chunk.Resize(10);
	//chunk.pu
	int v = 20;
	chunk.Put<int>(&v);
	return true;
}

int RetroPlugInstrument::UnserializeState(const IByteChunk & chunk, int startPos) {
	int v;
	chunk.Get<int>(&v, startPos);
	return 0;
}

void RetroPlugInstrument::generateMidiClock(int frameCount) {
	Lsdj& lsdj = _plug.lsdj();
	if (mTimeInfo.mTransportIsRunning) {
		switch (lsdj.syncMode) {
			case LsdjSyncModes::Slave:
				processSync(1, 0xF8);
				break;
			case LsdjSyncModes::SlaveArduinoboy:
				if (lsdj.arduinoboyPlaying) {
					processSync(lsdj.tempoDivisor, 0xF8);
				}
				break;
			case LsdjSyncModes::MidiMap:
				if (lsdj.arduinoboyPlaying) {
					processSync(1, 0xFF);
				}
				break;
		}
	}
}

void RetroPlugInstrument::processSync(int tempoDivisor, char value) {
	double ppq24 = mTimeInfo.mPPQPos * (24 / tempoDivisor);
	bool sync = false;
	if (ppq24 > _lastPpq24) {
		sync = (int)ppq24 != (int)_lastPpq24;
	} else {
		sync = true;
	}

	if (sync) {
		SameboyMin* plug = _plug.plug();
		plug->sendMidiByte(0, value);
	}

	_lastPpq24 = ppq24;
}

void RetroPlugInstrument::OnReset() {
	_plug.setSampleRate(GetSampleRate());
}

void RetroPlugInstrument::ProcessMidiMsg(const IMidiMsg& msg) {
	TRACE;

	SameboyMin* plug = _plug.plug();
	if (!plug) {
		return;
	}

	Lsdj& lsdj = _plug.lsdj();
	if (lsdj.found) {
		switch (lsdj.syncMode) {
			case LsdjSyncModes::SlaveArduinoboy:
				if (msg.StatusMsg() == IMidiMsg::kNoteOn) {
					switch (msg.NoteNumber()) {
						case 24: lsdj.arduinoboyPlaying = true; break;
						case 25: lsdj.arduinoboyPlaying = false; break;
						case 26: lsdj.tempoDivisor = 1; break;
						case 27: lsdj.tempoDivisor = 2; break;
						case 28: lsdj.tempoDivisor = 4; break;
						case 29: lsdj.tempoDivisor = 8; break;
						default:
							if (msg.NoteNumber() > 29) {
								// TODO: Send row number
							}
					}
				}

				break;
			case LsdjSyncModes::MidiMap:
				if (msg.StatusMsg() == IMidiMsg::kNoteOn) {
					// Select row
					plug->sendMidiByte(0, msg.NoteNumber());
				}

				break;
		}
	} else {
		// Presume mGB
		int status = msg.StatusMsg();
		char midiData[3];
		midiData[0] = msg.mStatus;
		midiData[1] = msg.mData1;
		midiData[2] = msg.mData2;

		plug->sendMidiBytes(0, (const char*)midiData, 3);
	}
}

void RetroPlugInstrument::OnParamChange(int paramIdx) {
}
#endif