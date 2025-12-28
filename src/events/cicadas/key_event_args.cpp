#include "key_event_args.h"

KeyEventArgs::KeyEventArgs() : NativeKey(0), Key(WindowKey::None), Char(0u), State(KeyState::Pressed), Control(false), Shift(false), Alt(false) {}

KeyEventArgs::KeyEventArgs(int native_key, WindowKey key, unsigned int c, KeyState state, bool control, bool shift, bool alt) : NativeKey(native_key), Key(key), Char(c), State(state), Control(control), Shift(shift), Alt(alt) {}