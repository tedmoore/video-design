//
//  defines.h
//  fonema video
//
//  Created by Ted Moore on 1/7/21.
//

#ifndef defines_h
#define defines_h

#define N_STATE_SAVES 10
#define N_MAGNITUDES 1
#define MAGNITUDES_LEN 1025

#define N_WAVEFORMS 2
#define WAVEFORM_LEN 1920
#define DESCRIPTORS_VECTOR_LENGTH 106

template <typename T>
T checkJsonKey(ofJson &j, std::string key, T default_val) {
    if (j[key].is_null()) {
        cout << "checkJsonKey: WARNING key " << key << " not found in json, using default value: " << default_val << endl;
        return default_val;
    } else {
        return j[key].get<T>();
    }
}

#endif /* defines_h */
