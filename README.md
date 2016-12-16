# miso - Micro Signal/Slot implementation

So you may ask, why another signal/slot implementation? Since we already have the grandfather of all, the Qt signal/slot implementation which as presented in [Qt4SigSlot] is a very powerful mechanism, invented just for this purpose. Or we have the [BoostSigSlot] which is another excellent implementation of the same mechanism. Or we have other less known signal/slot implementations, such as [SigSlot] or 

# Emitting a signal

Use emit signalname(param1, param2, ...);

This will create in turn a temporary `miso::emitter` object that will call the signal handler method of the signal, which in its turn will call the actual slot connected to this signal.

# Calling the signal handler

The `emitter` object created above will call the `delayed_dispatch` method of the signal, which is very uninteresting in the void signals, however in the signals with arguments calling the signal handler requires a few extra steps. Firstly the `signalname(param1, param2, ...)` is being handled by the 

The [Stackoverflow1] shows how to unpack a tuple holding various values of various types to a function with matching paremeter types. This is necessary in order to have a prefect match between the values the signal was created with and the slots that are supposed to get the same values.

# References

[Stackoverflow1] http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer

[Qt4SigSlot] C++ GUI Programming with Qt 4 by Jasmin Blanchette & Mark Summerfield
 
[Qt5SigSlot] Application Development with Qt Creator, Ray Rischpater

[BoostSigSlot]

[SigSlot] http://sigslot.sourceforge.net/sigslot.pdf by Sarah Thompson