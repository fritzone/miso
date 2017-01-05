# miso - Micro Signal/Slot implementation

**miso** shortly stays for  **mi**cro **s**ignals and sl**o**ts, and certainly as the name suggests, it is an implementation of the well known language construct introduced by Qt: The signals and slots mechanism [Wikipedia]. As the article suggests, the signal-slot construct is a short, concise and pain-free implementation of the Observer pattern, ie. it provides the possibility for objects (called observers) to be recipients of automatic notifications from objects (called subject) upon a change of state, or any other notification worth event.

## Reasoning

So you may ask, why another signal/slot implementation? Since we already have the grandfather of all, the Qt signal/slot implementation which, as presented in [Qt4SigSlot] is a very powerful mechanism, invented just for this purpose. 

Or we have the boost signal libraries [BoostSigSlot] which is another excellent implementation of the same mechanism for the users of the boost library.

And we also have other less known signal/slot implementations, such as Sarah Thompsons' signal and slot library (http://sigslot.sourceforge.net) or the VDK signals and slots written in C\++ 11 [VDKSigSlot], GNOME's own libsigc++ (http://libsigc.sourceforge.net/) [libsigc++], the nano signal slot (https://github.com/NoAvailableAlias/nano-signal-slot) [nanosigslot]. All these excellent pieces of software were written specifically for this purpose, and they all serve the needs of software developers wanting to use the Observer pattern without too much hassle. So with a good reason, you may ask why...

But please bear with me ... the implementation of this mechanism seemed to be such an interesting and highly challenging research project that I could not resist it. I wanted to use the elegance of the constructs introduced with C++11 in order to avoid as much as possible of the syntactical annoyances I have found in the various signal/slot projects, and I also wanted to keep it short and concise.

This article will not only provide a good overview of the usage of and operations permitted by this tiny library, but also will present a few interesting C++11 techniques I have stumbled upon while implementing the library and I have considered them to have a caliber worth of mentioning here.

## The library itself

miso being a single header library, is very easy to use, you just have to include the header file into your project and you're good to go: `#include <miso.h>` and from this point on you have access to the `namespace miso` which contains all relevant declarations that you need to use it. 

The library was written with portability in mind, and it is compilable for both Linux and Windows, it just needs a C++11 capable compiler. 

### Signals, slots, here and there

The notion of a slot is sort of uniform between all signal-slot libraries: It must be something that can be called. Regardless if it's a function, a functor, a lambda or some anomalous monstrosity returned by `std::bind` and placed into a `std::function`... at the end: It must be a callable. With or without parameters. Since this is what happens when you emit a signal: a "slot" is called.

However, there is no real consensus regarding the very nature of signals. Qt adopted the most familiar, clear and easy to understand syntax of all the signatures:
 
 ```cpp
 signals:
     void signalToBeEmitted(int someValue);
 ```

Simple, and clean, just like a the definition of a member function, with a unique signature, representing the parameters this signal can pass to the slots when it is `emit`ed. And the Qt meta object compiler takes care of it, by implementing the required supporting background operations (ie: the connection from the signal to actually calling the slot function), thus removing the burden from the programmer who can concentrate on implementing the actual program.

The other big player in platform independent C++ library solutions, boost, on the other end has chosen a somewhat more complex approach to defining a signal:

```cpp
  boost::signals2::signal<void (float, float)> sig;
```

This way of defining a signal feels very similar to the declaration of a function packed in a signal box, and due to the fact that it can be widely understood what it means, it was also adopted by [VDKSigSlot], [neosigslot] and [nanosigslot].

# Emitting a signal

Use emit signalname(param1, param2, ...);

This will create in turn a temporary `miso::emitter` object that will call the signal handler method of the signal, which in its turn will call the actual slot connected to this signal.

# Calling the signal handler

The `emitter` object created above will call the `delayed_dispatch` method of the signal, which is very uninteresting in the void signals, however in the signals with arguments calling the signal handler requires a few extra steps. Firstly the `signalname(param1, param2, ...)` is being handled by the 

The [Stackoverflow1] shows how to unpack a tuple holding various values of various types to a function with matching paremeter types. This is necessary in order to have a prefect match between the values the signal was created with and the slots that are supposed to get the same values.

# References

[Stackoverflow1] http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer

[Qt4SigSlot] C++ GUI Programming with Qt 4 by Jasmin Blanchette & Mark Summerfield

[BoostSigSlot] http://www.boost.org/doc/libs/1_61_0/doc/html/signals2.html

[VDKSigSlot] http://vdksoft.github.io/signals/index.html

[Wikipedia] https://en.wikipedia.org/wiki/Signals_and_slots

[libsigc++] http://libsigc.sourceforge.net/

[neosigslot] http://i42.co.uk/stuff/neosigslot.htm

[nanosigslot] https://github.com/NoAvailableAlias/nano-signal-slot