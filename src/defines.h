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

inline glm::vec3 limit(glm::vec3 &v, float max) {
    float lengthSquared = (v.x*v.x + v.y*v.y + v.z*v.z);
    if( lengthSquared > max*max && lengthSquared > 0 ) {
        float ratio = max/(float)sqrt(lengthSquared);
        v.x *= ratio;
        v.y *= ratio;
        v.z *= ratio;
    }
    return v;
}

inline bool endsWith(const std::string& str, const std::string& suffix) {
    return str.rfind(suffix) == (str.size() - suffix.size());
}

#endif /* defines_h */
