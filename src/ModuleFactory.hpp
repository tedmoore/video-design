#ifndef MODULE_FACTORY_H
#define MODULE_FACTORY_H

#include "ofMain.h"

using BuildModule = std::function<VisualModule*(SystemState &s, ofJson &j)>;

class ModuleFactory
{
public:
    std::unordered_map<string,BuildModule> functions;

    void registerFunction(string name, BuildModule f) {
        functions[name] = f;
    }

    VisualModule* createModule(SystemState &s, ofJson &j) {
        string name = checkJsonKey(j,"module-type",ofToString("not-found"));
        if (functions.find(name) != functions.end()) return functions[name](s, j);
        cout << "Module string name not found in ModuleFactory: " << name << endl;
        assert(false);
        return nullptr;
    }
};

#endif /* MODULE_FACTORY_H */