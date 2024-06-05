#ifndef AUDIO_FEATURES_H
#define AUDIO_FEATURES_H

struct AudioFeatures {
    AudioFeatures() : descriptors_vector(106), waveforms(N_WAVEFORMS, vector<float>(WAVEFORM_LEN)), magnitudes(N_MAGNITUDES, vector<float>(MAGNITUDES_LEN)) {}

    float spectral_centroid;
    float spectral_spread;
    float spectral_skewness;
    float spectral_kurtosis;
    float spectral_rolloff;
    float spectral_flatness;
    float spectral_crest;
    float pitch;
    float pitch_confidence;
    float loudness;
    float true_peak;
    float amplitude;
    float sensory_dissonance;
    float zero_crossings;
    vector<float> descriptors_vector;
    vector<vector<float>> waveforms;
    vector<vector<float>> magnitudes;
};

#endif  // AUDIO_FEATURES_H