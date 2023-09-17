# Video-Design OSC API

### Outline Format:

* **functionName**
    - arg 1
    - arg 2
    - arg 3
    - etc.

## Software-Wide Settings

* **o** (onset)
* **sai** (set active indices)
    - active index 0 (int)
    - active index 1 (int)
    - active index 2 (int)
    - active index 3 (int)
    - (the number of arguments provided needs to be equal to the software's `MAX_ACTIVE_MODULES`)
* **loadState**
    - index (int) of which of the 10 memory-saved states to load
* **loadStateFromDisk**
    - file name (without `.yaml` extension) of a disk-saved state that is in the `data` folder
* **sp** (set parameter)
    - moduleIndex (int)
    - parameterName (string)
    - value (float)

## Module Settings

### Video Module

* **speed**
    - speed
* **position**
    - normalized position
* **showHap**
    - 0 or 1 as boolean (false or true)
* **tile**
    - 0 or 1 as boolean (false or true)