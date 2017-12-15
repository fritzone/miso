# miso - Micro Signal/Slot implementation

**miso** shortly stays for  **mi**cro **s**ignals and sl**o**ts, and certainly as the name suggests, it is an implementation of the well known language construct popularized by Qt: The signals and slots mechanism [Wikipedia]. As the article suggests, the signal-slot construct is a short, concise and pain-free implementation of the Observer pattern, ie. it provides the possibility for objects (called observers) to be recipients of automatic notifications from objects (called subject) upon a change of state, or any other notification worth event.

## Reasoning

So you may ask, why another signal/slot implementation? Since we already have the grandfather of all, the Qt signal/slot implementation which, as presented in [Qt4SigSlot] is a very powerful mechanism, invented just for this purpose and which was enhanced with Qt5's new syntax for signals and slots [Qt5SigSlot]. 

Or we have the boost signal libraries [BoostSigSlot] which is another excellent implementation of the same mechanism for the users of the boost library.

And we also have other less known signal/slot implementations, such as Sarah Thompsons' signal and slot library (http://sigslot.sourceforge.net) or the VDK signals and slots written in C++ 11 [VDKSigSlot], GNOME's own libsigc++ (http://libsigc.sourceforge.net/) [libsigc++], the nano signal slot (https://github.com/NoAvailableAlias/nano-signal-slot) [nanosigslot]. All these excellent pieces of software were written specifically for this purpose, and they all serve the needs of software developers wanting to use the Observer pattern without too much hassle. So with a good reason, you may ask why...

But please bear with me ... the implementation of this mechanism seemed to be such an interesting and highly challenging research project that I could not resist it. I wanted to use the elegance of the constructs introduced with C++11 in order to avoid as much as possible of the syntactical annoyances I have found in the various signal/slot projects, which were bound to old-style C++ syntax, and I also wanted to keep it short and concise. Hence, this header only micro library appeared, and in the spirit of keeping it simple, it is under 150 lines, but still tries to offer the full functionality one would expect from a signal/slot library.

This article will not only provide a good overview of the usage of and operations permitted by this tiny library, but also will present a few interesting C++11 techniques I have stumbled upon while implementing the library and I have considered them to have a caliber worth of mentioning here.

## The library itself

**miso** being a single header library, is very easy to use, you just have to include the header file into your project and you're good to go: `#include <miso.h>` and from this point on you have access to the `namespace miso` which contains all relevant declarations that you need to use it. Later in this article we will present all the important details of this namespace.

The library was written with portability and standard conformance in mind, and it is compilable for both Linux and Windows, it just needs a C++11 capable compiler. 

## Signals, slots, here and there

The notion of a slot is sort of uniform between all signal-slot libraries: It must be something that can be called. Regardless if it's a function, a functor, a lambda or some anomalous monstrosity returned by `std::bind` and placed into a `std::function`... at the end: It must be a callable. With or without parameters. Since this is what happens when you emit a signal: a "slot" is called.

However, there is no real consensus regarding the very nature of signals. Qt adopted the most familiar, clear and easy to understand syntax of all the signatures:
 
 ```cpp
 signals:
     void signalToBeEmitted(float floatParameter, int intParameter);
 ```

Simple, and clean, just like a the definition of a member function, with a unique signature, representing the parameters this signal can pass to the slots when it is `emit`ed. And the Qt meta object compiler takes care of it, by implementing the required supporting background operations (ie: the connection from the signal to actually calling the slot function), thus removing the burden from the programmer who can concentrate on implementing the actual program.

The other big player in platform independent C++ library solutions, boost, on the other end has chosen a somewhat more complex approach to defining the same signal:

```cpp
  boost::signals2::signal<void (float, int)> sig;
```

This way of defining a signal feels very similar to the declaration of a function packed in a signal box, and due to the fact that it can be widely understood what it means, it was also adopted by [VDKSigSlot], [neosigslot] and [nanosigslot]. This syntax has the advantage of not requiring an extra step in the compilation phase (like `moc` of Qt) since this is already syntactically correct C++ which the compiler can handle without too much hassle. This declaration also has the side effect that comparing with Qt's signal declaration we have a tangible C++ variable which possibly is a class with methods and properties we can act upon.

### Signals in **miso**    

The signal definition of **miso** uses the following syntax in order to declare the same signal:

```cpp
signal<float, int> float_int_sig;
```

Achieving the simplicity of Qt's signal syntax seemed to not to be possible without using an extra step in the compilation phase (I am thinking of `moc`) and personally I have found including a function signature in the declaration of my signal not be working for me, so I went for the simplest syntax that was able to express the desired type of my signal (such as having a `float` and an `int` parameter) and with the supporting help of the variadic templates introduced in C++11 this seemed to be the ideal combination.

So, from this above we see that a signal in the **miso** framework will be an object, constructed from a templated class which handles a various number of types. As easily deducedm the signals which carry no parameters must be declared as:

```cpp
signal<> void_signal;
```

The decision to not have to explicitly specify the void signal as a template specialization has its advantages, both from the users' point of view, and also the library's internal design gained a bit of ruggedness by it. 

## A tiny application

The easiest way to introduce a new library is to present a small and simple which showcases the basic usage of the library, so here is the "Hello world" equivalent of miso:

```cpp
#include "miso.h"
#include <iostream>

struct a_class
{
    miso::signal<const char*> m_s;

    void say_hello()
    {
        emit m_s("Hello from a class");
    }
};

void a_function(const char* msg)
{
    std::cout << msg << std::endl;
}

int main()
{
    a_class a;
    miso::connect(a.m_s, a_function);
    a.say_hello();
}
```

After skipping the mandatory inclusions, let's analyze the important pieces:

Firstly, we declare a class (for with the struct keyword to keep the code short and uncluttered): `struct a_class`. In the **miso** framework the signals belong to classes, it is not possible to have a signal living outside of an enclosing entity. This sort of resonates on the same frequency as Qt's signal and slot mechanism, however the boost signals are more independently lived items who are not bound to a class.

As mentioned above, the **miso** signals are to be bound to a class so now is the perfect time to declare the signal object itself `miso::signal<const char*> m_s;`. All the **miso** types live in the `miso` namespace in order to avoid global namespace pollution, however this does not stop you from using the namespace as per your needs. The signal we have declared is expected to come with a paramater, which is of type `const char*`.

The next line in the class is a plain method, which has just one role: to emit the signal. This is done with the intriguing line: `emit m_s("Hello from a class");`. After spending several years with Qt, it just feel so natural to emit a signal and since I wanted to keep the essence of the library close to already existing constructs to facilitate the transition easily, the `emit` was born. `emit` will be dissected later in the article to understand how it works. 

The global method `void a_function(const char* msg)` will be the slot which is connected to this signal. It does not do too much, except prints to stdout the message it receives from the signal, but for demonstration purposes this is acceptable.

And now we have reached to the main method of the application, which creates an object of type `a_class` and connects its signal: `a.m_s` to the global function `a_function`. And last, but not least the `say_hello` method of the class is called, which in its turn will emit the signal. Upon emitting, the mechanism hidden in the library will kick in and the `a_function` will be called.

This was a short example, now it is time to break down the application into tiny pieces, and start examining it.  

## The miso namespace

There are the following interesting elements in the `miso` namespace
  
 1. The `signal` class 
 2. The `connect` and the `sender` methods
 3. The macro definition for `emit`. Although this is not namespace dependant, it just felt right to place it there.
 
and also another namespace, named `internal` with the intention of not to be used by the end-users.

Due to these being interconnected, I will present them one by one, however be prepared for several jumps between various components, and since the namespace level entities use the internals very heavily there will be necessarry to dig into it too. 

### The signal class

The class responsible for creating signals has the following declaration:

```cpp
template <class... Args> class signal final
```

My intention was to keep the signal objects final, in order to have a clean interface and easy implementation, however this does not stop you from removing the final and providing good implementation for use cases for signal derived classes. A short overview of the public members is as follows: The default constructor and destructors are marked `default`, we just let the compiler do its default work for this case. 

The following two methods are `connect` and `disconnect`, and as their name suggest these will connect (or disconnect) the signal to (from) a slot. Right now the following slots can be used:

#### A function

The function must be declared with parameters corresponsing to the parameters of the signal, and it must not be restricted to basic C++ types. Using `std::function` values also works, and so do the `static` methods of various classes.

```cpp
struct other_class {
    void method() const {
        std::cout << "Hello from the Other class method";
    }
};

struct a_class {
    miso::signal<other_class> m_s;

    void say_hello() {
        emit m_s(other_class());

    }
};

void b_function(other_class oc) {
    oc.method();
}

int main() {
    a_class a;
    std::function<void (other_class)> f = b_function;
    miso::connect(a.m_s, f);
    miso::connect(a.m_s, b_function);
    a.say_hello();
}
```

The example from above will call `b_function` two times which will print two times the "Hello from the Other class method" due to it being connected two times to the same signal.
 

#### A lambda expression

The lambda expression can be either coming from an `auto l = []() {...}` expression, or just simply written as a parameter to the connect. Again, correct matching of lambda parameters is required. So, an example for the above source code would be:

```cpp
miso::connect(a.m_s, [](other_class b) { b.method(); });
```

#### A functor

A function object allows to invoke the instantiation of a functor class in a manner similar to functions by providing an overload to `operator ()`. So, in order to use a functor as a slot we can have the following code:     

```cpp

struct functor {
    void operator()(int aa) {
        std::cout << "functor class's int slot:" << aa << std::endl;
    }
};

struct a_class {
    miso::signal<int> m_s;

    void say_hello() {
        emit m_s(42);
    }
};

int main() {
    a_class a;
    functor f;

    miso::connect(a.m_s, f);

    a.say_hello();
}
```

As a side note, if there are more than overloads of `operator ()` it is possible to connect more than one signals to the same functor.   

3. 

# Emitting a signal

Use emit signalname(param1, param2, ...);

This will create in turn a temporary `miso::emitter` object that will call the signal handler method of the signal, which in its turn will call the actual slot connected to this signal.

# Calling the signal handler

The `emitter` object created above will call the `delayed_dispatch` method of the signal, which is very uninteresting in the void signals, however in the signals with arguments calling the signal handler requires a few extra steps. Firstly the `signalname(param1, param2, ...)` is being handled by the 

The [Stackoverflow1] shows how to unpack a tuple holding various values of various types to a function with matching paremeter types. This is necessary in order to have a prefect match between the values the signal was created with and the slots that are supposed to get the same values.

# References

[Stackoverflow1] http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer

[Qt4SigSlot] C++ GUI Programming with Qt 4 by Jasmin Blanchette & Mark Summerfield

[Qt5SigSlot] http://doc.qt.io/qt-5/signalsandslots.html

[BoostSigSlot] http://www.boost.org/doc/libs/1_61_0/doc/html/signals2.html

[VDKSigSlot] http://vdksoft.github.io/signals/index.html

[Wikipedia] https://en.wikipedia.org/wiki/Signals_and_slots

[libsigc++] http://libsigc.sourceforge.net/

connect(mc, os, sourceforge.net/

    neosigslot] http://i42.co.uk/stuff/neosigslot.htm    
:    
nnvim    mi	c	
    [nanosigs   _globa_with_two_parameters);l    
 htt    s://github.com/NoAvailableAlias/nano-signal-slot
