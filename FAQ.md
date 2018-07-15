# General
* Is the plugin only usable for audio?
  - No. Everything audio specific in the core specification (sample rates,
    data types, names) would be a bug. Nonetheless, there are additional ports
    dedicated to audio, which are shipped in a separate header + lib
* This is not a comlete visitor implementation?
  - Not completely. The accept functions declarations are conforming, but their
    definitions are not really, as they do a static cast. If you take the cast
    away and see the casted function as the visitor base function, it is a
    correct visitor implementation.
* Why not using LV2? Why another plugin now?
  - Advantages of LV2 over SPA:
    * Currently a way larger user base, both plugins and hosts
    * Does not depend on C++11 (and less on the ABI)
    * LV2 is stable for years, SPA is still very experimental
  - Advantages of SPA over LV2
    * (Simple) C++ only, you don't need to learn anything else
    * Arbitrary initialization parameters. LV2 seems to only allow the sample
      rate, SPA allows everything you can pass through ports. This can also be
      an advantage if you don't work with audio only, where a sample rate might
      not be enough.
    * Easy to implement hosts
    * Quick and easy to implement plugins (compare the zyn SPA and LV2 plugins:
      SPA are a few hundred LOC, LV2 comes with a whole folder plus the DPF lib)
    * Maybe better OSC support (e.g. robust against internal port changes)
    * Static UI can be easily embedded into the plugin (upcoming feature)
    * LMMS: "Native" SPA implementation, LV2 only via Carla (June 2018)
* Isn't it a disadvantage that host must load the whole libraries in order to
  even get their names/descriptions etc?
  - The idea SPA is that a host can use a seperate process to check if the
    libraries load. The host won't abort if the process gets killed. Also,
    hosts should index all libraries only when they see them the first time, or
    afer the libraries changed. All in all, the plan is that it's way easier to
    let the (few) hosts fork a new process than requiring the (many) plugins to
    write text files, learn languages etc for this. Also, host programmers may
    have more programming experience than plugin programmers, who might be more
    artists than programmers
* When should I use the override specifier for the `visit` functions of my
  visitors?
  - If you make a library visitor for visiting newly defined types, some newer
    compilers will reject override for all your new types, as they are not
    overriding the virtual function from the base (they are new virtual
    functions and not virtually called from any base class). So you should not
    use override in this case
  - If you make a host visitor, i.e. a visitor in your host where you defined
    how the ports should be connected, you can and should use the override
    keyword
* As changing a port's content may break compatibility with the host (as the
  byte alignment may change), how do I ever update my library's ports?
  - Don't touch the old ports, simply make new ones: `my_port_v2`, `my_port_v3`
* How do I use template ports with my own template if their visitors don't have
  visit funcs instantiated for my classes?
  - The problem is that their accept template calls their visitor. You need to
    call your visitor, so you need to change their accept func by overriding
    it in an inheriting class, e.g.
    ```
    template<class T> class my_ringbuffer : public spa_ringbuffer<T> {};
    template<> class my_ringbuffer<time_T> : public spa_ringbuffer<time_t>
    {
	SPA_OBJECT
    };
    SPA_MK_ACCEPT(time_t_ringbuffer, my_visitor);
    ```
    See (hopefully) the examples.
* What else do I need to take care of when writing a library?
  - At very least, [checklist.md].
* What do I need to take care of when writing a plugin?
  - You can assume that the host is always doing everything correctly. You can
    build failure safe code, but it's not your job. Plugins should be minimal
    and easy to write.
* What criterions are there for defining a port as OSC port (e.g. using the
  OSC ringbuffer for it) vs defining it as LADSPA like port (e.g. ports
  containing pointers to the data, like `port_ref` ports)?
  - When to use OSC ports:
    * When the port structure is too complex
    * When the number of ports is too high
    * For event like things that only need to be checked when they occur,
      or that only require re-computations when they change
      (e.g. "note-on" would be a candidate, though note there will be a piano port
       for this case, or e.g. changes to zyn's pad synth that require expensive
       re-computation)
  - When to use LADSPA like ports:
    * When it's easier
    * When the ports are used more frequently than LFOs (=> runtime matters)

# Troubleshooting
* I'm getting linker errors, it can not find the descriptor
  => Maybe you forgot the `extern "C" {}` around it?

