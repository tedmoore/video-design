//~waveform_file = CSVFileReader.readInterpret("/Volumes/Ted's 10TB My Book (June 2020)/PROJECT FILES/noise gate/reaper/original 210506_01/_bounces/210605_183801/noise_gate_stutteries_waveform_frames_start=128sec.csv_frameSize=3840_maxOffset=1920.csv",true);

(
var folder = thisProcess.nowExecutingPath.dirname+/+"data-renders";
s.options.numInputBusChannels_(8);
s.options.sampleRate_(44100);
s.options.device_("Fireface UC Mac (24006457)");
s.waitForBoot({
	var analysis_bus = Bus.audio(s,2);
	var frame_rate = 30;
	var start_sec = 0;
	var guis = IdentityDictionary.new;
	var outpath = folder+/+Date.localtime.stamp;
	var outfile, render_data = false;
	var freeables = List.new, minBus, maxBus, lagUBus, lagDBus, nVecChans = 106, analyses;

	// var audio_buf = Buffer.read(s,"/Volumes/2110 Seagate 8TB/Project Files/saccades/reaper/_bounces/230103_160733/230103_160733_saccades-tape-mvt-1.wav");
	// var audio_buf = Buffer.read(s,"/Volumes/Ted's 10TB My Book (June 2020)/SOUND DESIGNS/spacy space/_bounces/spacy space test with sax.wav");
	// var audio_buf = Buffer.read(s,"/Users/macprocomputer/Documents/_CREATING/_PROJECT_FILES/three-videos/sounds/FIM-test.wav");
	// var audio_buf = Buffer.read(s,"/Users/macprocomputer/Documents/_CREATING/_PROJECT_FILES/three-videos/sounds/a-movement-itself?.aiff");
	// var audio_buf = Buffer.read(s,"/Users/macprocomputer/Documents/_CREATING/_PROJECT_FILES/three-videos/reaper/_bounces/230214_235123_reverb-feedback.wav");
	var audio_buf = Buffer.read(s,"/Users/macprocomputer/Documents/_CREATING/_PROJECT_FILES/three-videos/reaper/_bounces/230214_235123_ring-mod-drone.wav");

	if(render_data){
		outpath.mkdir;
		outfile = ArrayToCSV.open(outpath+/+"descriptors.csv");
	};

	lagUBus = Bus.control(s,nVecChans).setn(0.dup(nVecChans));
	lagDBus = Bus.control(s,nVecChans).setn(0.dup(nVecChans));
	minBus = Bus.control(s,nVecChans).setn(0.dup(nVecChans));
	maxBus = Bus.control(s,nVecChans).setn(1.dup(nVecChans));

	freeables.add(lagUBus);
	freeables.add(lagDBus);
	freeables.add(minBus);
	freeables.add(maxBus);

	s.sync;

	~oF = NetAddr("127.0.0.1",11000);

	~synth = {
		arg t_force_onset = 0, onset_thresh = 0.2, allow_onsets = 1, nMels = 40;
		// var sig = PlayBuf.ar(2,audio_buf,BufRateScale.ir(audio_buf),startPos:audio_buf.sampleRate * start_sec,loop:1);
		var sig = SoundIn.ar(2).dup;
		// var sig = SoundIn.ar(6).dup;
		var sig_mono = Mix(sig) * 0.5;

		// var sig_mono = SoundIn.ar(0);
		// var sig = sig_mono.dup;

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

		vector = MinMaxScaler.kr(vector);
		vector = LagUD.kr(vector,In.kr(lagUBus,nVecChans),In.kr(lagDBus,nVecChans));
		vector = LinLin.kr(vector,srclo:In.kr(minBus,nVecChans),srchi:In.kr(maxBus,nVecChans),dstlo:0,dsthi:1);

		vector = vector ++ [Trig.kr(onsets,frame_rate.reciprocal)];

		SendReply.kr(trig,"/vector",vector);

		Out.ar(analysis_bus,sig);
		// Out.ar(0,sig * -20.dbamp);
		nil;
	}.play;

	// Window.closeAll;
	guis[\win] = Window(bounds:Rect(0,0,1200,800)).front;
	guis[\dropdown] = PopUpMenu(guis[\win],Rect(0,0,200,20))
	.allowsReselection_(true)
	.action_({
		arg pum;
		guis[\displays].keysValuesDo{
			arg k, v;
			if(k == pum.item){
				v.visible_(true);
			}{
				v.visible_(false);
			};
		};
	});
	guis[\displays] = IdentityDictionary.new;

	// waveforms
	guis[\displays][\Waveform0] = MultiSliderView(guis[\win],Rect(0,20,1190,300)).elasticMode_(1).drawRects_(false).drawLines_(true).visible_(false);
	guis[\displays][\Waveform1] = MultiSliderView(guis[\win],Rect(0,20,1190,300)).elasticMode_(1).drawRects_(false).drawLines_(true).visible_(false);

	// mags
	guis[\displays][\Mags] = CompositeView(guis[\win],Rect(0,20,1190,970)).visible_(false);
	guis[\Mags] = MultiSliderView(guis[\displays][\Mags],Rect(0,0,1190,300)).elasticMode_(1).isFilled_(true);

	// mels
	guis[\displays][\Mels] = CompositeView(guis[\win],Rect(0,20,1190,970)).visible_(false);
	guis[\displays][\Mels].decorator_(FlowLayout(Rect(0,0,guis[\displays][\Mels].bounds.width,guis[\displays][\Mels].bounds.height)));
	guis[\nMels] = EZSlider(guis[\displays][\Mels],Rect(0,0,500,20),"N Mels",ControlSpec(1,40,step:1),{
		arg sl;
		~synth.set(\nMels,sl.value);
	},40);
	guis[\Mels] = MultiSliderView(guis[\displays][\Mels],Rect(0,0,1190,300)).elasticMode_(1).isFilled_(true);

	// mfccs
	guis[\displays][\MFCCs] = CompositeView(guis[\win],Rect(0,20,1190,970)).visible_(false);
	guis[\MFCCs] = MultiSliderView(guis[\displays][\MFCCs],Rect(0,20,1190,300)).elasticMode_(1).drawRects_(false).drawLines_(true);

	// vector
	guis[\displays][\Analyses] = CompositeView(guis[\win],Rect(0,20,1190,970)).visible_(false);
	guis[\displays][\Analyses].decorator_(FlowLayout(Rect(0,0,guis[\displays][\Analyses].bounds.width,guis[\displays][\Analyses].bounds.height)));

	analyses = [\Centroid,\Spread,\Skewness,\Kurtosis,\Rolloff,\Flatness,\Crest,\Pitch,\Confidence,\Loudness,\TruePeak,\Amp,\Dissonance,\ZeroCrossing];

	guis[\sliders] = Array.newClear(analyses.size);
	guis[\thresholds] = Array.newClear(analyses.size);
	guis[\isAbove] = Array.newClear(analyses.size);

	analyses.do{
		arg name, i;
		var slHeight = 165;
		var view = CompositeView(guis[\displays][\Analyses],Rect(0,0,230,200));
		view.decorator_(FlowLayout(view.bounds));
		view.background_(Color.hsv(i.linlin(0,analyses.size,0.55,0.45),1,0.9));
		StaticText(view,Rect(0,0,190,20)).string_(name);
		view.decorator.nextLine;
		EZSlider(view,Rect(0,0,40,slHeight),"LagU",ControlSpec(0,3),{arg sl; lagUBus.setAt(i,sl.value)},0,layout:'vert');
		EZSlider(view,Rect(0,0,40,slHeight),"LagD",ControlSpec(0,3),{arg sl; lagDBus.setAt(i,sl.value)},0,layout:'vert');
		EZRanger(view,Rect(0,0,40,slHeight),"Scale",nil.asSpec,layout:'vert',action:{
			arg ezr;
			minBus.setAt(i,ezr.value[0]);
			maxBus.setAt(i,ezr.value[1]);
		});
		guis[\sliders][i] = EZSlider(view,Rect(0,0,40,slHeight),"Val",layout:'vert',action:{
			arg sl;
			// "% value: %".format(name.asString.padRight(10),sl.value).postln;
			~oF.sendMsg("/analysis/%".format(name.asString.toLower),sl.value);
			if(sl.value > guis[\thresholds][i].value){
				if(guis[\isAbove][i].not){
					~oF.sendMsg("/threshold/%".format(name.asString.toLower),1);
					"% is above the threshold".format(name).postln;
				};
				sl.setColors(sliderBackground:Color.green);
				guis[\isAbove][i] = true;
			}{
				if(guis[\isAbove][i]){
					~oF.sendMsg("/threshold/%".format(name.asString.toLower),0);
				};
				sl.setColors(sliderBackground:Color.white);
				guis[\isAbove][i] = false;
			};
		});
		guis[\sliders][i].sliderView.thumbSize_(2).enabled_(false);
		guis[\sliders][i].numberView.enabled_(false);
		guis[\thresholds][i] = EZSlider(view,Rect(0,0,40,slHeight),"Thrsh",nil.asSpec,{arg sl;},1,layout:'vert');
		guis[\isAbove][i] = false;
	};

	guis[\dropdown].items_(guis[\displays].keys.asArray.sort);
	guis[\dropdown].valueAction_(0);

	// s.sync;
	WaveformToOSC(s,analysis_bus,0,~oF,0,false,outfile:if(render_data){outpath+/+"waveform-0.csv"}{nil},action:{
		arg sy;
		freeables.add(sy);
	},callback:{
		arg samps;
		defer{guis[\displays][\Waveform0].value_(samps + 0.5)};
	});
	WaveformToOSC(s,analysis_bus,1,~oF,1,false,outfile:if(render_data){outpath+/+"waveform-1.csv"}{nil},action:{
		arg sy;
		freeables.add(sy);
	},callback:{
		arg samps;
		defer{guis[\displays][\Waveform1].value_(samps + 0.5)};
	});
	MagsToOSC(s,analysis_bus,2,~oF,0,lag:30,outfile:if(render_data){outpath+/+"mags.csv"}{nil},action:{
		arg sy;
		freeables.add(sy);
	},callback:{
		arg samps;
		defer{guis[\Mags].value_(samps)};
	});

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
			~oF.sendMsg("/onset");
		};
		message = ["/vector"] ++ vec;

		~oF.sendMsg(*message);

		defer{
			guis[\Mels].value_(vec[54..(53 + guis[\nMels].value.asInteger)]);
			guis[\MFCCs].value_(vec[14..53]);
			guis[\sliders].do{
				arg sl, i;
				sl.valueAction_(vec[i]);
			};
		};
	},"/vector");

	freeables.add(~synth);

	// guis[\win].onClose_({freeables.do(_.free)});

	if(render_data){
		audio_buf.duration.wait;
		freeables.do(_.free);
		outfile.close;
		"done rendering data".postln;
	};
});
)

/*~synth.set(\t_force_onset,1);
~synth.set(\onset_thresh,0.5);
~synth.set(\allow_onsets,0);
~synth.set(\allow_onsets,1);



m.getn(5,{arg v; v.postln});*/