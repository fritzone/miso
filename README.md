# miso - Micro Signal/Slot implementation

**miso** shortly stays for  **mi**cro **s**ignals and sl**o**ts, and certainly as the name suggests, it is an implementation of the well known language construct largely popularized by Qt: The signals and slots mechanism [Wikipedia]. As the Wikipedia article suggests, the signal-slot construct is a short, concise and pain-free implementation of the Observer pattern, ie. it provides the possibility for objects (called observers) to be recipients of automatic notifications from objects (called subject) upon a change of state, or any other notification worth event.

## Reasoning

So you may ask, why another signal/slot implementation? Since we already have the granddaddy of all, the Qt signal/slot implementation which, as presented in [Qt4SigSlot] is a very powerful mechanism, invented just for this purpose and which was even further enhanced with Qt5's new syntax for signals and slots [Qt5SigSlot]. 

Or we have the boost signal libraries [BoostSigSlot] which is another excellent implementation of the same mechanism for the users of the boost library.

And we also have other less known signal/slot implementations, such as Sarah Thompsons' signal and slot library (http://sigslot.sourceforge.net) or the VDK signals and slots written in C++ 11 [VDKSigSlot], GNOME's own libsigc++ (http://libsigc.sourceforge.net/) [libsigc++], the nano signal slot (https://github.com/NoAvailableAlias/nano-signal-slot) [nanosigslot] or several other ones which I was not able to discover even with Google's powerful search algorithm. 

All these excellent pieces of software were written specifically for this purpose, and they all serve the needs of software developers wanting to use the signals and slots mechanism without too much hassle. 

And on the other end, the Observer pattern has been a very widely adopted and successful pattern which also was widely studied in various articles, including Overloads' own ones, such as Phil Bass's articles in Overload 52 and 53: "Implementing the Observer Pattern" [PhilBass] or Pete Goodliffe's articles in Overload 37, 38 and 41 (Experiences of Implementing the Observer Design Pattern) [PeteGoodliffe] - both excellent articles which were not backed up by the power of C++11's syntax and standard library yet ... due to the fact they have been written in the first years of this century - but also Alan Griffiths' article from 2014 (Designing Observers in C++11) [AlanGriffiths] which lifted this pattern into the modern age using C++11 constructs.

So with a good reason, you may ask why...

But please bear with me ... the implementation of this mechanism seemed to be such an interesting and highly challenging research project that I could not resist it. I wanted to use the elegance of the constructs introduced with C++11 in order to avoid as much as possible of the syntactical annoyances I have found in the various signal/slot projects, which were bound to old-style C++ syntax, and I also wanted to keep it short and concise. Hence, this header only micro library appeared, and in the spirit of keeping it simple, it is under 150 lines, but still tries to offer the full functionality one would expect from a usable signal/slot library.

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

Simple, and clean, just like a the definition of a member function, with a unique signature, representing the parameters this signal can pass to the slots when it is `emit`ed. And the Qt meta object compiler takes care of it, by implementing the required supporting background operations (ie: the connection from the signal to actually calling the slot function), thus removing the burden from the programmer who can concentrate on implementing the actual functionality of the program.

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

Achieving the simplicity of Qt's signal syntax seemed to not to be possible without using an extra step in the compilation phase (I am thinking at the convenience offered by `moc`) and personally I have found including a function signature in the declaration of my signal not to be working for me, so I went for the simplest syntax that was able to express the desired type of my signal (such as a signal, having a `float` and an `int` parameter) and with the supporting help of the variadic templates introduced in C++11 this seemed to be the ideal combination.

So, from this above we see that a signal in the **miso** framework will be an object, constructed from a templated class which handles a various number of types. As easily deduced the signals which carry no extra information in form of parameters must be declared as:

```cpp
signal<> void_signal;
```

The decision to not have to explicitly specify the void signal as a template specialization has its advantages, both from the users' point of view, and also the library's internal design gained a bit of ruggedness by it. 

## A tiny application

The easiest way to introduce a new library is to present a small and simple example which showcases the basic usage of the library, so here is the "Hello world" equivalent of miso:

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

Firstly, we declare a class (for now with the `struct` keyword to keep the code short and uncluttered): `struct a_class`. In the **miso** framework the signals belong to classes, it is not possible to have a signal living outside of an enclosing entity. This sort of resonates on the same frequency as Qt's signal and slot mechanism, however the boost signals are more independently lived items who are not bound to a class.

As mentioned above, the **miso** signals are to be bound to a class so now is the perfect time to declare the signal object itself `miso::signal<const char*> m_s;`. All the **miso** types live in the `miso` namespace in order to avoid global namespace pollution, however this does not stop you from using the namespace as per your needs. The signal we have declared is expected to come with a parameter, which is of type `const char*`.

The next line in the class is a plain method, which has just one role: to emit the signal. This is done with the intriguing line: `emit m_s("Hello from a class");`. After spending several years with Qt, it just feel so natural to`emit`a signal and since I wanted to keep the essence of the library close to already existing constructs to facilitate the transition easily, the `emit` was born. `emit` will be dissected later in the article to understand how it works. 

The global method `void a_function(const char* msg)` will be the slot which is connected to this signal. It does not do too much, except prints to stdout the message it receives from the signal, but for demonstration purposes this is acceptable.

And now we have reached to the main method of the application, which creates an object of type `a_class` and connects its signal: `a.m_s` to the global function `a_function`. And last, but not least the `say_hello` method of the class is called, which in its turn will emit the signal. Upon emitting, the mechanism hidden in the library will kick in and the `a_function` will be called. There is support in the library to obtain the object which emitted the signal, the current slot is handling by calling the `miso::sender` method, however this is not presented in this short example.

This was a short example, now it is time to break down the application into tiny pieces, and start examining it.  

## The miso namespace

There are the following interesting elements in the `miso` namespace

 1. The `signal` class 
 2. The `connect` and the `sender` methods
 3. The macro definition for `emit`. Although this is not namespace dependent, it just felt right to place it there.

and also another namespace, named `internal` with the intention of not to be used by the end-users.

Due to these being interconnected, I will present them one by one, however be prepared for several jumps between various components, and since the namespace level entities use the internals very heavily there will be necessary to dig into it too. 

### The signal class

The class responsible for creating signals has the following declaration:

```cpp
template <class... Args> class signal final
```

My intention was to keep the signal objects final, in order to have a clean interface and easy implementation, however this does not stop you from removing the final and providing good implementation for use cases for signal derived classes. 

Since the class is a template class, nothing stops you from creating signals for your own data types and using the properly in the emit and the receiving slot declaration.  

A short overview of the public members is as follows: The default constructor and destructors are marked `default`, we just let the compiler do its default work for this case. 

The following two methods are `connect` and `disconnect`, and as their name suggest these will connect (or disconnect) the signal to (from) a slot. 

Right now the following entities can be used as slots:

#### A function

The function must be declared with parameters corresponding to the parameters of the signal, and it must not be restricted to basic C++ types. Using `std::function` values also works, and so do the `static` methods of various classes.

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

As a side note, if there are more than one overloads of `operator()` it is possible to connect more than one signals to the same functor, each being handled by its own `operator()`. And since this is an overly over templatized solution, the compiler will take care that the required slots with matching signatures to the ones the signal requires are actually available, otherwise it will spectacularly fail with a long list of cryptic messages.

#### Connect internals

In the `signal` class,`connect` and `disconnect` are implemented both using `internal::connect_i`, by calling it like:

```cpp
template<class T>
void connect(T&& f, bool active = true) {
    internal::connect_i<T, typename slot_holder<T>::FT, slot_holder<T>>
                       (std::forward<T>(f), slot_holders, active);
}
```
Where the `T&& f` is just the slot where we want this signal to reach upon emitting, and `active` tells whether this signal is active or not (disconnect calls the same function with `active = false`).

The parameters to the internal function come up as following, by using forward on the `f` parameter to the current function, then `slot_holders` which is a local variable of type:

``` cpp
std::vector<internal::common_slot_base*> slot_holders;
```

And finally, `active` to tell the framework whether this signal is active or not (ie: should be called upon `emit` or not).

Since `common_slot_base` has appeared now, here is a definition for it:

```cpp
struct common_slot_base {
    virtual ~common_slot_base() = default;
};
```
so, basically it is just an interface to be used by all the different kind of signals to have a mean to call their corresponding slots. An immediate usage of it is in the `signal` class:

```cpp
    struct slot_holder_base : public internal::common_slot_base {
        virtual void run_slots(Args... args) = 0;
    };
```
with further specialization following in:

```cpp
template<class T>
struct slot_holder : public slot_holder_base {
    using FT = std::function<typename 
                             std::result_of<T(Args...)>::type(Args...)>;
    using slot_vec_type = std::vector<internal::func_and_bool<FT>>;
    slot_vec_type slots;
        
    void run_slots(Args... args) override 
    {
        std::for_each(slots.begin(), slots.end(), 
                      [&](internal::func_and_bool<FT>& s)
                          { if (s.active) (*(s.ft.get()))(args...); }
                         );
    }
};
```
Reading the last method, it is obvious that the main action happens here, i.e. the actual call of a slot as per the corresponding signal takes place in these lines. 

A bit more investigation of this structure gives us the declaration of `FT` being an `std::function` which at compile time identifies its return type from the template parameter of the `slot_holder` class (`T` which is supposed to be a "Callable") which is fed into the `std::result_of` of the `<type_traits>` header having the parameters `Args...` of the signal class, this `slot_holder` resides in, combined again with the `Args...` of the signal to obtain a fully understandable expression. Just a clarification, `FT` stands fro **F**unction **T**ype. And last but not least about this construct: Personally, this piece of code I consider one of the small wonders of the powers of modern C++... (read: even after writing it, and knowing, that it's syntactically correct and valid code in my weaker moments I still wonder that it compiles...)

Since in this structure we have introduced a new structure (`func_and_bool`), here is its definition:

```cpp
    template<typename FT>
    struct func_and_bool final {
        std::shared_ptr<FT> ft;
        bool active;
        void *addr;
    };
```
which roughly holds the lowest level of a slot, i.e.: a function object, whether it is active or not and its address, thus revealing that at the lowest level all slots are decaying into an `std::function` (the one which was declared in the type name `FT` of the `struct slot_holder`).

Now, that we have covered the necessary structures and functions of a signal, it is time that we look at the actual function from the internals, which performs the real connect:

```cpp
template<class T, class FT, class SHT>
void connect_i(T &&f, std::vector<common_slot_base *> &sholders, 
               bool active = true) 
{
    static SHT sh;
    func_and_bool<FT> fb{
        std::make_shared<FT>(std::forward<T>(f)), 
        active, 
        reinterpret_cast<void *>(&f)
    };
    bool already_in = false;
    std::for_each(sh.slots.begin(), sh.slots.end(),
                  [&](func_and_bool<FT> &s) 
                  { 
                      if (s.addr == fb.addr)
                      {
                          s.active = active; 
                          already_in = true; 
                      } 
                  }
                 );
    if (!already_in)
    {
        sh.slots.emplace_back(fb);
    }
    if (std::find(sholders.begin(), 
                  sholders.end(), 
                  static_cast<common_slot_base *>(&sh)) == sholders.end()) 
    {
        sholders.push_back(&sh);
    }
}

```

So, dissecting it into bits we can observe the following:

- The type of the `static SHT sh;` local variable came in via the template parameters, and for our case it will be structure `slot_holder` declared in the `signal` class. Now, this `sh` (slot holder, for the uninitiated) variable will be common for all the `connect_i` functions sharing a common prototype (hence, the static). For the peculiar ones, `SHT` stands for **S**lot **H**older **T**ype. Don't you dare thinking at anything else.
- The next step is to create a `func_and_bool` object with the type of the `FT` we discussed in the `slot_holder` class, and we do a check whether the newly created object is in the slot holder already (by comparing it's physical address to those already in the container), if yes we set it's activeness state to the one required in the parameter, but since we don't want to add it again we also flip a boolean flag for later usage.
- The next step is updating the incoming parameter `sholders` in order to append the local `sh` object. This is where the magic happens since this parameter is the same that is declared in the `signal` class, and since ` slot_holder<T>` is a `common_slot_base` specialization we successfully managed to gather all the slots regardless of their parameters, this type of signal class is connected to into one common entity we can operate on.

With these covered we have successfully surveyed the mechanisms behind the connection of a slot to a specific signal, so we can jump to the next stage of our library, namely, emitting a signal.

### Emitting a signal

The syntax, as seen from the tiny example application provided, is:

```cpp 
emit signalname(param1, param2, ...);
```

By digging further in the header file, we find that `emit` basically is:

```cpp
    #define emit \
              miso::internal::emitter\
                   <std::remove_pointer<decltype(this)>::type>(*this) <<
```

(Yes, that is a `<<` operator at the end of the line)

So, a simple `emit` will create in its turn a temporary `miso::internal::emitter` object which is a helper class, like:

```cpp
template<class T>
struct emitter final {
    explicit emitter(const T &emtr) {
        sender_objs.push(&emtr);
        minstance = this;
    }
    ~emitter() {
        sender_objs.pop();
        minstance = nullptr;
    }
    static T *sender() {
        return const_cast<T *>(sender_objs.top());
    }
    static emitter<T> *instance() {
        return minstance;
    }
private:
    static std::stack<const T *> sender_objs;
    static emitter<T> *minstance;
};
```
whose role is to keep track of the current object that emitted the signal. The logic for this I fearlessly embrace is written in a nice and self explanatory code above, so let's just give an example of how can we retrieve the sender of the current signal:

``` cpp
struct a_class {
    miso::signal<int> m_s;

    void say_hello() {
        emit m_s(42);
    }
    int x = 45;
};
struct functor {
    void operator()(int aa) {
        std::cout << "functor class's int slot:" << aa << std::endl;
        a_class* ap = miso::sender<a_class>();
        std::cout << "x in emitter class:" << ap->x << std::endl;    
    }
};
int main() {
    a_class a;
    functor f;
    miso::connect(a.m_s, f);
    a.say_hello();
}
```

So, in the slot, we just simply call:

```cpp
a_class* ap = miso::sender<a_class>();
```

and this is supposed to give us the class of the required type that has emitted a signal due to whom we are in the current slot. Be aware, that if we are not handling the slot class due to an emit from a signal, and we call the `miso::sender` we will get a `std::runtime_error` exception.

#### The internals of calling the signal handler

If you wonder about the `<<` operator in the macro definition of emit, please note the signal class has a very complex friend declaration, in the form of:

```cpp
template<class T, class... Brgs> friend
internal::emitter<T> && internal::operator << (internal::emitter<T> &&e,
                                               signal<Brgs...> &s);
```

which looks like:

```cpp
template<class T, class... Args>
emitter<T> &&operator <<(internal::emitter<T> &&e, signal<Args...> &s) {
    s.delayed_dispatch();
    return std::forward<internal::emitter<T>>(e);
}
```

In order, to make the syntax possible, please also note the following in the signal class:

```cpp
std::tuple<Args...> call_args;

signal<Args...>& operator()(Args... args) {
    call_args = std::tuple<Args...>(args...);
    return *this;
}
```
(so, the signal class in its turn is also a functor :) ) otherwise the required syntax for the `emit` wouldn't have been possible. The `call_args` member is nothing else, than just the calling arguments of emitting the signal populated by this `operator()`.

Now we can see that the temporary `emitter` object created above will call the overloaded  `<<` operator with the signal (which in its turn has already consumed the input parameters via the `operator()` call) , and in there the `delayed_dispatch` method of the signal is called.

When it comes about delayed dispatch [Stackoverflow1] shows how to unpack a tuple holding various values of various types to a function with matching parameter types. This is necessary in order to have a perfect match between the values the signals' call arguments was populated with and the slots that are supposed to get the same values.

When the delayed dispatch method runs in its turn it calls the `run_slots` from the slot holders vector (which if you remember, was populated in the connect step).

## The future

With this lengthy overview, I am confident, that everyone in need to use a lightweight signal-slot library has at least one more choice to select from, making the decision even harder. At the same time I'm hoping that this article has shed some light into how to use this library, and whether it is a good choice for your team or not that entirely depends on you.

For the ones relying on a clone from github more than copying 150 lines of code from the journal (Appendix A) into their project please use the following link:

https://github.com/fritzone/miso

where you can find the implementation of the library in `miso.h` (released under MIT license).

Also, please note: For now, this is far from a full fledged signal-slot library, offering the power and functionality you would expect from Qt or Boost. Depending on time and resources I would be happy to add features you request (or even approve your pull request in case you consider it worth fixing a few bugs here and there, or adding a new nice to have item to it) but till then kindly treat it lightly.

## Appendix A

The full source of the library.

```cpp
#ifndef MISO_H
#define MISO_H

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <tuple>
#include <stack>

namespace miso
{
    template <class... Args> class signal;
    namespace internal {
        template<int ...> struct sequence {};
        template<int N, int ...S> struct sequence_generator : sequence_generator<N - 1, N - 1, S...> {};
        template<int ...S> struct sequence_generator<0, S...> { typedef sequence<S...> type; };
        template<typename FT>
        struct func_and_bool final {
            std::shared_ptr<FT> ft;
            bool active;
            void *addr;
        };
        struct common_slot_base {
            virtual ~common_slot_base() = default;
        };
        template<class T, class FT, class SHT>
        void connect_i(T &&f, std::vector<common_slot_base *> &sholders, bool active = true) {
            static SHT sh;
            func_and_bool<FT> fb{std::make_shared<FT>(std::forward<T>(f)), active, reinterpret_cast<void *>(&f)};
            bool already_in = false;
            std::for_each(sh.slots.begin(), sh.slots.end(),
                          [&](func_and_bool<FT> &s) { if (s.addr == fb.addr){s.active = active; already_in = true; } });
            if (!already_in) sh.slots.emplace_back(fb);
            if (std::find(sholders.begin(), sholders.end(), static_cast<common_slot_base *>(&sh)) == sholders.end()) {
                sholders.push_back(&sh);
            }
        }
        template<class T>
        struct emitter final {
            explicit emitter(const T &emtr) {
                sender_objs.push(&emtr);
                minstance = this;
            }
            ~emitter() {
                sender_objs.pop();
                minstance = nullptr;
            }
            static T *sender() {
                return const_cast<T *>(sender_objs.top());
            }
            static emitter<T> *instance() {
                return minstance;
            }
        private:
            static std::stack<const T *> sender_objs;
            static emitter<T> *minstance;
        };
        template<class T> std::stack<const T *> emitter<T>::sender_objs;
        template<class T> emitter<T> *emitter<T>::minstance = nullptr;
        template<class T, class... Args>
        emitter<T> &&operator <<(internal::emitter<T> &&e, signal<Args...> &s) {
            s.delayed_dispatch();
            return std::forward<internal::emitter<T>>(e);
        }
    }
	template <class... Args>
	class signal final
	{
        struct slot_holder_base : public internal::common_slot_base {
            virtual void run_slots(Args... args) = 0;
        };
        template<class T>
        struct slot_holder : public slot_holder_base {
            using FT = std::function<typename std::result_of<T(Args...)>::type(Args...)>;
            using slot_vec_type = std::vector<internal::func_and_bool<FT>>;
            slot_vec_type slots;

            void run_slots(Args... args) override {
                std::for_each(slots.begin(), slots.end(), [&](internal::func_and_bool<FT>& s)
                              { if (s.active) (*(s.ft.get()))(args...); }
                );
            }
        };
        std::vector<internal::common_slot_base*> slot_holders;
        std::tuple<Args...> call_args;
        void emit_signal(Args... args) {
            for (auto& sh : slot_holders) {
                (dynamic_cast<slot_holder_base*>(sh))->run_slots(args...);
            }
        }
        template<int ...S>
        void delayed_call(internal::sequence<S...>)	{
            emit_signal(std::get<S>(call_args) ...);
        }
        void delayed_dispatch()	{
            delayed_call(typename internal::sequence_generator<sizeof...(Args)>::type());
        }
    public:
        template<class T, class... Brgs> friend
        internal::emitter<T> && internal::operator <<(internal::emitter<T> &&e, signal<Brgs...> &s);

		explicit signal() = default;
		~signal() noexcept = default;
		template<class T>
		void connect(T&& f, bool active = true) {
			internal::connect_i<T, typename slot_holder<T>::FT,
			          slot_holder<T>> (std::forward<T>(f), slot_holders, active);
		}
		template<class T>
		void disconnect(T&& f) {
			connect<T>(std::forward<T>(f), false);
		}
        signal<Args...>& operator()(Args... args) {
            call_args = std::tuple<Args...>(args...);
            return *this;
        }
	};
    template<class Si, class So>
    void connect(Si &&sig, So &&slo) {
        static_assert(!std::is_same<So, std::nullptr_t>::value, "cannot use nullptr as slot");
        std::forward<Si>(sig).connect(std::forward<So>(slo));
    }
    template<class T>
    T *sender() {
        if(internal::emitter<T>::instance()) {
            return internal::emitter<T>::instance()->sender();
        }
        throw std::runtime_error("not in an emit call");
    }
    #define emit miso::internal::emitter<std::remove_pointer<decltype(this)>::type>(*this) <<
}
#endif
```

## References

[Stackoverflow1] http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer

[Qt4SigSlot] C++ GUI Programming with Qt 4 by Jasmin Blanchette & Mark Summerfield, **ISBN-13:** 978-0131872493

[Qt5SigSlot] http://doc.qt.io/qt-5/signalsandslots.html

[BoostSigSlot] http://www.boost.org/doc/libs/1_61_0/doc/html/signals2.html

[neosigslot] http://i42.co.uk/stuff/neosigslot.htm

[VDKSigSlot] http://vdksoft.github.io/signals/index.html

[Wikipedia] https://en.wikipedia.org/wiki/Signals_and_slots

[libsigc++] http://libsigc.sourceforge.net/

[PhilBass] https://accu.org/var/uploads/journals/overload52-FINAL.pdf 

[AlanGriffiths] https://accu.org/var/uploads/journals/Overload124.pdf#page=5

[PeteGoodliffe] https://accu.org/index.php/journals/488


