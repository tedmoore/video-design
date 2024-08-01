//
//  defines.h
//  fonema video
//
//  Created by Ted Moore on 1/7/21.
//

#ifndef defines_h
#define defines_h

#include <iostream>
#include <vector>
#include <numeric>

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

// Function to generate a weighted random index
inline int getWeightedRandomIndex(const std::vector<float>& weights) {
    // Calculate the total sum of weights
    double totalWeight = std::accumulate(weights.begin(), weights.end(), 0.0);
    
    // Generate a random number between 0 and the total weight
    double randomWeight = ofRandom(0, totalWeight);
    
    // Find the index corresponding to the random weight
    double cumulativeWeight = 0.0;
    for (size_t i = 0; i < weights.size(); ++i) {
        cumulativeWeight += weights[i];
        if (randomWeight <= cumulativeWeight) {
            return i;
        }
    }
    
    // In case of rounding errors, return the last index
    return weights.size() - 1;
}

#endif /* defines_h */
