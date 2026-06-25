# wayland_mock

This sandbox has no `libwayland-server-dev`, no `wayland-protocols`, no
`wayland-scanner`, and no network access to install them. That makes it
impossible to compile `src/wayland/*.cpp` against the real headers here.

What's in this directory is a **structural mock**: hand-written headers
that reproduce the *shape* (struct layout, function signatures) of the
real `<wayland-server-core.h>`, `<wayland-server-protocol.h>`, and a
generated `xdg-shell-protocol.h`, plus an `aquamarine/` mock for
`Compositor.cpp`'s sake. `link_test_main.cpp` + `mock_impl.cpp` link the
**real** `src/wayland/Surface.cpp` and `src/wayland/XdgShell.cpp` against
these mocks and actually run, proving:

- The dispatch-table struct declarations have the right number/order of
  function pointers for what's being initialized (a real and easy way to
  get this wrong, and the actual reason this directory exists).
- The C++/C namespace interaction the code relies on -- a struct TAG
  (e.g. `struct wl_surface_interface`, the dispatch table type) and an
  `extern const` variable of the *same name* (the protocol metadata)
  coexisting in one translation unit -- really does work the way the
  comments in `Surface.cpp` claim.
- Construction/destruction of `CompositorGlobal` and `XdgShellGlobal`,
  and the callback wiring between them, doesn't crash.

What it does **NOT** prove:

- That the real wayland-scanner output for your installed
  wayland-protocols version has these exact same field names/order (it's
  been stable for years for the requests used here, but "structurally
  plausible" isn't "verified against your headers").
- Anything about Aquamarine's real API -- that mock is a convenience for
  syntax-checking `Compositor.cpp`'s own control flow, not a claim about
  Aquamarine's actual symbols.

Run it yourself:

```sh
g++ -std=c++23 -I ../../src -I . \
    link_test_main.cpp mock_impl.cpp \
    ../../src/wayland/Surface.cpp ../../src/wayland/XdgShell.cpp \
    -o link_test && ./link_test
```

Once you have the real dev packages installed, build the actual project
with `cmake` (see the main README) instead of this -- this directory's
only job is catching dumb errors before you're standing in front of a
real Wayland session.
