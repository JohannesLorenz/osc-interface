# osc-interface
A minimal OSC communication interface

## Purpose
osc-interface is a minimal interface for communication between
* producers (e.g. instruments like zynaddsubfx)
* consumers (e.g. DAWs like LMMS)

Producers can typically be plugins, coded as shared object files (`.so`-files
or `.dll`-files).

The protocol used for OSC, e.g. which messages to send or expect, is not
specified. Your applications should find a reasonable standard.

## FAQ
* Where are the cpp files for the library?
  - This interface is header-only (like the LADSPA project)
* Is the plugin only usable for audio?
  - The only audio-only function is `OscConsumer::runSynth`. If you don't
    need it, never call it in the consumer. Aside, everything (including DnD)
    can be used for non audio projects.

## Requirements
* C++11 able compiler

## Interface protocol
The OSC interface works as follows:
* If the producer is provided as a shared library, this must be loaded first.
* Each producer provides a **loader function** of type
  `osc_descriptor_loader_t`. This function returns an osc descriptor (subclass
  of OscDescriptor) which makes
  - it easy for consumers to quickly get information about the producers
    without needing to instantiate it
  - it possible for consumers to instantiate producers (e.g. allocating
    memory, starting threads etc)
* If the consumer wants to **instantiate a producer**, it calls the virtual
  function `instantiate` of the OscDescriptor subclass, which will return
  a subclass of OscConsumer. The subclass' constructor
  - must *start* the procuder, e.g. allocate memory, initialize variables,
    start threads...
  - after the constructor, the producer must be running as if it has been
    started otherwise
  - if the consumer provides a UI, it shall not be started in the constuctor.
    The applications must specify OSC messages to open or close any UI.
* The **cleaning up** is done by the destructor, which must
  - clean everything up that the constructer has set up
  - free any used memory
  - close any opened UI
* The **producer provides methods** about
  - how to send messages to it (`sendOsc`)
  - how to receive audio from it (`runSynth`)
  - how to retreive its audio buffer size (`buffersize`)
* The **consumer provides methods** about
  - how to send messages to it (TODO: unspecified yet)

## Mutexes
Every function call to a function of the OscInterface function shall be
wrapped around a corresponding mutex. I do not recall why (TODO: recall...).

## Preset playback
TODO

## Multiple producers inside one interface
TODO: not yet specified

## Proposal for drag and drop
The following is a sole proposal. It's the currently used standard for OSC
instuments in LMMS.

* Producer's UI elements (knobs, sliders etc) controlling values can be
  dragged onto consumer's
  - automation patterns
  - controllers
* The drag type offered by the producers shall be an atom `x-osc-stringpair`
* If such a drop occurs successfully, the producer saves
  `automatable-model:<ID>:/osc/path/to/element`, where the `<ID>` must be
  - and ID that uniquely identifies the producer inside the consumer
    (e.g. PID for external processes, assuming each process is only used once
     as a producer, or a simple enumeration of producers)
  - has been announced to the producer

## Proposal for OSC protocol
The following is a sole proposal. It's the currently used standard for OSC
instuments in LMMS.

Messages consumer to producer (via the `sendOsc` function):
* **save-master:s** Ask the producer to save its savefile into a file specified
  by the 1st arg. TODO: return value check
* **load-master:s** Ask the producer to load the savefile specified by the 1st
  arg. TODO: return value check
* **show-ui:TF** Ask the producer to show or hide its graphical UI, if it has
  one. If the UI is already open and the argument is `true` or if it is closed
  and the argument is `false`, the producer discards the message.
* **noteOn:iii** Ask the producer to play a note with
  - channel <1st arg> (whatever a channel is)
  - key <2nd arg> (TODO: describe how keys are counted)
  - velocity <3rd arg> (TODO: describe min, max)
* **noteOff:iii** Ask the producer to stop playing every note
  - on channel <1st arg>
  - with key <2nd arg>
* **dnd-requests** TODO
* **All other messages** shall
  - either be strictly specified by your used protocol (in the case of this
    proposal: none)
  - or be previously announced (e.g. by drag and drop) by a producer to the
    consumer for cases like automation. Such messages shall be handled by
    the producer like regular OSC messages.

Message producer to consumer (via TODO):
* **save-master** confirmation TODO
* **load-master** confirmation TODO
* **dnd info** TODO
* **dnd removal** TODO

## Sample implementation

* [LMMS consumer](https://github.com/JohannesLorenz/lmms/tree/osc-plugin/plugins/oscinstrument)
* [zynaddsubfx producer](https://github.com/zynaddsubfx/zynaddsubfx/tree/osc-plugin/src/Output)

