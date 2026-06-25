// CONFIRMED by the user's actual build: the access path
// output->name / output->state->state().mode->pixelSize.{x,y} compiles
// fine against the real Aquamarine 0.12.1 headers (their build got past
// these lines with zero complaints), so -- unlike the IPointer case --
// this guess turned out to be right. Kept as a placeholder shape rather
// than pulling in the real Output.hpp, since there was nothing to fix.
#pragma once
#include "../backend/Backend.hpp"
#include <hyprutils/signal/Signal.hpp>
#include <string>

namespace Aquamarine {

struct Vector2D {
    double x = 0;
    double y = 0;
};

struct SOutputMode {
    Vector2D pixelSize;
};

struct SOutputState {
    SOutputMode* mode = nullptr;
};

class IOutputStateHolder {
public:
    SOutputState state() const { return {}; }
};

class IOutput {
public:
    std::string name;
    IOutputStateHolder* state = nullptr;
    struct {
        Hyprutils::Signal::CSignalT<> frame;
    } events;
};

} // namespace Aquamarine
