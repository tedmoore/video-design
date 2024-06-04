#ifndef VECTORHISTORY_H
#define VECTORHISTORY_H

#include "ofMain.h"

class VectorHistory
{
private:
    size_t frame_index = 0;

public:
    vector<vector<float>> history;
    bool isFull = false;

    void setup(int history_length)
    {
        history.resize(history_length);
        for (int i = 0; i < history.size(); i++)
        {
            history[i].resize(DESCRIPTORS_VECTOR_LENGTH);
            for (int j = 0; j < history[i].size(); j++)
                history[i][j] = 0;
        }
    }

    void updateCurrentFrameAtIndex(int index, float val)
    {
        if (frame_index < history.size())
            history[frame_index][index] = val;
    }

    void incrementFrameIndex()
    {
        // check if we just added the last index to the history and if so set true
        if (frame_index == (history.size() - 1))
            isFull = true;

        // increment and modulous
        frame_index = (frame_index + 1) % history.size();
    }
};

#endif /* VECTORHISTORY_H */