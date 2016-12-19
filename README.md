# miso - Micro Signal/Slot implementation

So you may ask, why another signal/slot implementation? Since we already have the grandfather of all, the Qt signal/slot implementation which, as presented in [Qt4SigSlot] is a very powerful mechanism, invented just for this purpose. 

Or we have the boost signal libraries [BoostSigSlot] which is another excellent implementation of the same mechanism for the users of the boost library.

And we also have other less known signal/slot implementations, such as Sarah Thompsons' signal and slot library (http://sigslot.sourceforge.net) or the VDK signals and slots written in C\++ 11 [VDKSigSlot], GNOME's own libsigc++ (http://libsigc.sourceforge.net/) the nano signal slot (https://github.com/NoAvailableAlias/nano-signal-slot). All these excellent pieces of software were written specifically for this purpose, and they all serve the needs of software developers wanting to use the Observer pattern without too much hassle. So with a good reason, you may ask why...

But please bear with me ... the implementation of this mechanism seemed to be such an interresting and highly challenging research project that I could not resist it. I wanted to use the elegance of the constructs introduced with C++11 in order to avoid as much as possible of the syntactical annoyances I have found in the various signal/slot projects, and I also wanted to keep it short and concise.

# Emitting a signal

Use emit signalname(param1, param2, ...);

This will create in turn a temporary `miso::emitter` object that will call the signal handler method of the signal, which in its turn will call the actual slot connected to this signal.

# Calling the signal handler

The `emitter` object created above will call the `delayed_dispatch` method of the signal, which is very uninteresting in the void signals, however in the signals with arguments calling the signal handler requires a few extra steps. Firstly the `signalname(param1, param2, ...)` is being handled by the 

The [Stackoverflow1] shows how to unpack a tuple holding various values of various types to a function with matching paremeter types. This is necessary in order to have a prefect match between the values the signal was created with and the slots that are supposed to get the same values.

# References

[Stackoverflow1] http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer

[Qt4SigSlot] C++ GUI Programming with Qt 4 by Jasmin Blanchette & Mark Summerfield

[BoostSigSlot] http://www.boost.org/doc/libs/1_61_0/doc/html/signals.html

[VDKSigSlot] http://vdksoft.github.io/signals/index.html

