//~waveform_file = CSVFileReader.readInterpret("/Volumes/Ted's 10TB My Book (June 2020)/PROJECT FILES/noise gate/reaper/original 210506_01/_bounces/210605_183801/noise_gate_stutteries_waveform_frames_start=128sec.csv_frameSize=3840_maxOffset=1920.csv",true);

(
s.options.device_("Fireface UC Mac (24006457)");
s.waitForBoot({
	Task({
		var analysis_bus = Bus.audio(s,2);
		var synth;
		var frame_rate = 30;
		var start_sec = 0;

		var test_buf = Buffer.read(s,"/Users/macprocomputer/Desktop/sound collecting 210715/11 noisy proper --- series of gestures good fat sounds.wav");
		s.sync;

		~oF = NetAddr("127.0.0.1",11000);

		OSCdef(\send_data,{
			arg msg;
			var vec = ["/vector"] ++ msg[3..];
			vec.postln;
			~oF.sendMsg(*vec);
		},"/vector");

		OSCdef(\onset,{
			arg msg;
			~oF.sendMsg("/onset");
			"onset".postln;
		},"/onset");

		synth = {
			var sig = PlayBuf.ar(2,test_buf,BufRateScale.ir(test_buf),startPos:test_buf.sampleRate * start_sec,loop:1);
			var sig_mono = Mix(sig) * 0.5;
			var fft = FFT(LocalBuf(2048),sig_mono);
			var trig = Impulse.kr(frame_rate);
			var frame = PulseCount.kr(trig) - 1;
			var spec = FluidSpectralShape.kr(sig_mono);
			var pitch = FluidPitch.kr(sig_mono);
			var mfcc = FluidMFCC.kr(sig_mono,40);
			var loudness = FluidLoudness.kr(sig_mono);
			var chroma = FluidChroma.kr(sig_mono,maxNumChroma:12);
			var melbands = FluidMelBands.kr(sig_mono,maxNumBands:40);
			var scdesc = [Amplitude.kr(sig_mono),SensoryDissonance.kr(fft),A2K.kr(ZeroCrossing.ar(sig_mono))];
			var vector = spec ++ pitch ++ loudness ++ scdesc ++ mfcc ++ melbands ++ chroma;
			var onsets = Onsets.kr(fft);

			vector = MinMaxScaler.kr(vector);
			SendReply.kr(trig,"/vector",vector);

			SendReply.kr(onsets,"/onset");

			Out.ar(analysis_bus,sig);
			Out.ar(0,sig * -20.dbamp);
		}.play;
		s.sync;
		WaveformToOSC(s,analysis_bus,0,~oF,0,false);
		WaveformToOSC(s,analysis_bus,1,~oF,1,false);
		MagsToOSC(s,analysis_bus,2,~oF,0);
	},SystemClock).play;
});
)

~oF.sendMsg("/setActiveIndices",-1,4,3);