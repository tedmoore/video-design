//~waveform_file = CSVFileReader.readInterpret("/Volumes/Ted's 10TB My Book (June 2020)/PROJECT FILES/noise gate/reaper/original 210506_01/_bounces/210605_183801/noise_gate_stutteries_waveform_frames_start=128sec.csv_frameSize=3840_maxOffset=1920.csv",true);

(
var folder = thisProcess.nowExecutingPath.dirname+/+"data-renders";
s.options.numInputBusChannels_(4);
s.options.sampleRate_(44100);
s.options.device_("Fireface UC Mac (24006457)");
s.waitForBoot({
	var analysis_bus = Bus.audio(s,2);
	var frame_rate = 30;
	var start_sec = 0;

	var outpath = folder+/+Date.localtime.stamp;
	var outfile, render_data = false;
	var freeables = List.new;

	var audio_buf = Buffer.read(s,"/Users/macprocomputer/Documents/_CREATING/_PROJECT_FILES/saccades/reaper/_bounces/221121_170959_saccades-01-mvt-1.wav");

	// var audio_buf = Buffer.read(s,"/Volumes/Ted's 10TB My Book (June 2020)/SOUND DESIGNS/spacy space/_bounces/spacy space test with sax.wav");

	// var audio_buf = Buffer.read(s,"/Users/macprocomputer/Music/SuperCollider Recordings/improvPatch/improvPatch_230121_154652/FIM-test.wav");

	if(render_data){
		outpath.mkdir;
		outfile = ArrayToCSV.open(outpath+/+"descriptors.csv");
	};

	s.sync;

	~oF = NetAddr("127.0.0.1",11000);

	OSCdef(\send_data,{
		arg msg;
		var vec = msg[3..];
		if(render_data){
			// "vec size: %".format(vec.size).postln;
			outfile.writeLine(vec);
		};
		if(vec[vec.size - 1] > 0.5){
			"onset".postln;
		};
		vec = ["/vector"] ++ vec;
		~oF.sendMsg(*vec);
	},"/vector");

	~synth = {
		arg t_force_onset = 0, onset_thresh = 0.5, allow_onsets = 1;
		var sig = PlayBuf.ar(2,audio_buf,BufRateScale.ir(audio_buf),startPos:audio_buf.sampleRate * start_sec,loop:1);
		// var sig = SoundIn.ar([2,3]);
		var sig_mono = Mix(sig) * 0.5;
		var fft = FFT(LocalBuf(2048),sig_mono);
		var trig = Impulse.kr(frame_rate);
		var spec = FluidSpectralShape.kr(sig_mono);
		var pitch = FluidPitch.kr(sig_mono);
		var mfcc = FluidMFCC.kr(sig_mono,40);
		var loudness = max(FluidLoudness.kr(sig_mono),-90);
		var chroma = FluidChroma.kr(sig_mono);
		var melbands = FluidMelBands.kr(sig_mono);
		var scdesc = [Amplitude.kr(sig_mono),SensoryDissonance.kr(fft),A2K.kr(ZeroCrossing.ar(sig_mono))];
		var onsets = (Onsets.kr(fft,onset_thresh) * allow_onsets) + t_force_onset;
		var vector = spec ++ pitch ++ loudness ++ scdesc ++ mfcc ++ melbands ++ chroma ++ [Trig.kr(onsets,frame_rate.reciprocal)];

		vector = MinMaxScaler.kr(vector);
		SendReply.kr(trig,"/vector",vector);

		Out.ar(analysis_bus,sig);
		Out.ar(0,sig * -20.dbamp);
		nil;
	}.play;
	// s.sync;
	WaveformToOSC(s,analysis_bus,0,~oF,0,false,outfile:if(render_data){outpath+/+"waveform-0.csv"}{nil},action:{
		arg sy;
		freeables.add(sy);
	});
	WaveformToOSC(s,analysis_bus,1,~oF,1,false,outfile:if(render_data){outpath+/+"waveform-1.csv"}{nil},action:{
		arg sy;
		freeables.add(sy);
	});
	MagsToOSC(s,analysis_bus,2,~oF,0,outfile:if(render_data){outpath+/+"mags.csv"}{nil},action:{
		arg sy;
		freeables.add(sy);
	});

	if(render_data){
		audio_buf.duration.wait;
		freeables.do(_.free);
		outfile.close;
		"done rendering data".postln;
	};
});
)

~synth.set(\t_force_onset,1);
~synth.set(\onset_thresh,0.4);
~synth.set(\allow_onsets,0);
~synth.set(\allow_onsets,1);
