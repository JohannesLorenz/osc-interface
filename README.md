# spa

Simple plugin API

## Purpose

spa is a minimal interface for communication between
* hosts (e.g. DAWs like LMMS)
* plugins (e.g. instruments like zynaddsubfx)

Plugins are usually (always?? TODO) coded as shared object files (`.so`-files
or `.dll`-files).

## Principles

* extension = inheritation
* KISS
  * only C++
  * easy to learn and use
* move difficulties to host writers, not to plugin writers
  * plugins are written much more often
  * plugins are written faster (e.g. for musicians, where you just want to try
    out how it sounds)
  * plugins may be more often written by non-programmers than hosts (musicians
    with less experience in programming/specs should still be able to write
    plugins)
* power and simplicity
  * give large apps (e.g. with many OSC ports) their full power
  * still make writing minimal effects easy
* unique identifiers = URLs in the www

## FAQ

See [README.md].

## Requirements

* C++11 able compiler

## Mutexes

TODO: Must every function call to a function of the `plugin` be
wrapped around a corresponding mutex? In what cases?

## Multiple plugins inside one lib

TODO: not yet specified

## Proposal for OSC based drag and drop

The following is a sole proposal. It's currently a used standard for OSC
instuments in an LMMS branch.

* plugin's UI elements (knobs, sliders etc) controlling values can be
  dragged onto the hosts's UI elements, e.g. automation patterns or controllers
* The plugin offers data of MIME type `x-osc-stringpair`, with content
  `automatable-model:osc.udp://IP:PORT/osc/path/to/element`
* The host finds out which of its plugins did the drop by lookin up that URL.
  If it can not do that (maybe because UDP is not supported), the window ID of
  the plugin should be used when receiving the drag
* The host can then use `plugin::port(const char*)` function on the right
  plugin, getting the right port structure for the OSC path

## Proposal for OSC protocol

spa supplies an OSC ringbuffer for audio applications. Using this OSC ringbuffer
requires a unique standard for messages. The following is a proposal.
It's the currently used standard for OSC instuments in an LMMS branch.

Messages host to plugin:
* **noteOn:iii** Ask the plugin to play a note with
  - channel (whatever a channel is, it could be a MIDI part)
  - key (0..127) (69 being the 440 Hz "a" note if no keyshift or detuning
                  is involved)
  - velocity (0..127)
* **noteOff:ii** Ask the plugin to stop playing every (the last?) note
  - on channel
  - with key
* **All other messages** shall be strictly documented by your used protocol and
  - either be used where ports don't suffice, as permanently polling ports would
    cause too much overhead
  - or be previously announced (e.g. by drag and drop) by a plugin to the
    host for cases like automation. Such messages shall be handled by
    the plugin like regular OSC messages. In this case, still, using "real"
    ports is still encouraged

Note: noteOn and noteOff might be replaced with "note ports" in the future.

Messages plugin to host:
* **connection-removal:s** (TODO: likely deprecated)
  Information that <1st arg> has been removed from
  its previous connection

## Sample implementation

* Minimal host and plugin examples are in the [examples](examples) folder
* [LMMS host](https://github.com/JohannesLorenz/lmms/tree/osc-plugin/plugins/oscinstrument)
* [zynaddsubfx plugin](https://github.com/zynaddsubfx/zynaddsubfx/tree/osc-plugin/src/Output)

