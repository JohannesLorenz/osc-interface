# Checklist for library writers

Library writers should check this before each release:
* all library defines are undefined after they are used (except those that shall
  be visible on purpose)
* use headers of the std libraries and external libraries only if different
  versions may not cause different data being exchanged between plugin and host;
  this counts also for submodules you use
