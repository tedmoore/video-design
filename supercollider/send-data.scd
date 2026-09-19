
(
var folder = thisProcess.nowExecutingPath.dirname+/+"data-renders";
s.options.numInputBusChannels_(10);
s.options.numWireBufs_(64*2);
s.options.sampleRate_(44100);
s.options.device_("Fireface UC Mac (24006457)");
// s.options.device_("ZoomAudioD");
s.waitForBoot({
	var analysis_bus = Bus.audio(s,2);
	var frame_rate = 30;
	var start_sec = 20;
	var outpath = folder+/+Date.localtime.stamp;
	var outfile, render_data = false, loop = true;
	var nVecChans = 106;

	// var audio_buf = Buffer.read(s,"/Users/ted/Documents/_PROJECT-FILES/arco/perf-materials/240317/240317_183254-mvt-2-tape.wav");
	// var audio_buf = Buffer.read(s,"/Users/ted/Documents/_PROJECT-FILES/_ideas/from-with-luke/from-with-luke-000.wav");
	// var audio_buf = Buffer.read(s,"/Users/ted/Documents/_PROJECT-FILES/arco/perf-materials/240317/240317_183450-mvt-3-tape.wav");
	// var audio_buf = Buffer.read(s,"/Users/ted/Documents/_PROJECT-FILES/arco/perf-materials/240317/240317_183202-mvt-1-tape.wav");
	// var audio_buf = Buffer.read(s,"/Users/ted/Documents/_PROJECT-FILES/arco/perf-materials/240317/240317_183254-mvt-2-tape.wav");
	// var audio_buf = Buffer.read(s,"/Users/ted/Documents/_PROJECT-FILES/arco/perf-materials/240317/240317_183450-mvt-3-tape.wav");
	// var audio_buf = Buffer.read(s,"/Users/ted/Documents/_PROJECT-FILES/arco/perf-materials/240317/240317_183911-mvt-5-tape.wav");

	outpath.postln;

	if(render_data){
		outpath.mkdir;
		outfile = ArrayToCSV.open(outpath+/+"descriptors.csv");
	};

	s.sync;

	~oF = NetAddr("127.0.0.1",11000);

	~synth = {
		arg t_force_onset = 0, onset_thresh = 0.2, allow_onsets = 1, nMels = 40;
		// var sig = PlayBuf.ar(2,audio_buf,BufRateScale.ir(audio_buf),startPos:audio_buf.sampleRate * start_sec,loop:loop.asInteger);
		var sig = SoundIn.ar([8,9]);
		var sig_mono = Mix(sig) * 0.5;

		var fft = FFT(LocalBuf(2048),sig_mono);
		var trig = Impulse.kr(frame_rate);
		var spec = FluidSpectralShape.kr(sig_mono);
		var pitch = FluidPitch.kr(sig_mono);
		var mfcc = FluidMFCC.kr(sig_mono,40);
		var loudness = max(FluidLoudness.kr(sig_mono),-90);
		var chroma = FluidChroma.kr(sig_mono);
		var melbands = FluidMelBands.kr(sig_mono,nMels,maxNumBands:40);
		var scdesc = [Amplitude.kr(sig_mono),SensoryDissonance.kr(fft),A2K.kr(ZeroCrossing.ar(sig_mono))];
		var onsets = (Onsets.kr(fft,onset_thresh) * allow_onsets) + t_force_onset;

		//           0-6     7-8      9-10        11-13     14-53   54-93       94-105    106
		var vector = spec ++ pitch ++ loudness ++ scdesc ++ mfcc ++ melbands ++ chroma;

		vector = MinMaxScaler.kr(vector).lag(0.3);

		vector = vector ++ [Trig.kr(onsets,frame_rate.reciprocal)];

		SendReply.kr(trig,"/vector",vector);

		Out.ar(analysis_bus,sig);
		// Out.ar(0,sig * -20.dbamp);
		nil;
	}.play;

	~waveformDisplay = Window(bounds:Rect(0,0,1000,400));
	~msViews = Array.newClear(2);

	// s.sync;
	2.do{
		arg i;
		~msViews[i] = MultiSliderView()
		.elasticMode_(true)
		.drawRects_(false)
		.drawLines_(true)
		.reference_(0.5);

		WaveformToOSC(s,analysis_bus,i,~oF,i,phase_tracking:false,outfile:if(render_data){outpath+/+"waveform-%.csv".format(i)}{nil},callback:{
			arg samps;
			// samps.size.postln;
			// defer{~msViews[i].value_(samps.linlin(-1,1,0,1))};
		});
	};

	~waveformDisplay.layout = VLayout(*~msViews);
	// ~waveformDisplay.front;

	MagsToOSC(s,analysis_bus,2,~oF,0,outfile:if(render_data){outpath+/+"mags.csv"}{nil});

	OSCdef(\send_data,{
		arg msg;
		var vec = msg[3..];
		var message;

		if(render_data){
			// "vec size: %".format(vec.size).postln;
			outfile.writeLine(vec);
		};
		if(vec[vec.size - 1] > 0.5){
			// "onset".postln;
			~oF.sendMsg("/o");
		};
		message = ["/vector"] ++ vec;

		~oF.sendMsg(*message);

	},"/vector");

	if(render_data){
		// audio_buf.duration.wait;
		s.quit({
			outfile.close;
			"done rendering data".postln;
		});
	};
});
)

~synth.set(\t_force_onset,1);
~synth.set(\onset_thresh,0.5);
~synth.set(\allow_onsets,0);
~synth.set(\allow_onsets,1);



m.getn(5,{arg v; v.postln});