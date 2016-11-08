#ifndef MISO_H
#define MISO_H

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <tuple>

#include <iostream>
#include <typeinfo>

#include <cstddef>
#include <tuple>


#include <type_traits>
#include <utility>

template<int ...> struct seq {};

template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> {};

template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };


template <class... Args>
class signal final
{
public:
	signal() = default;
	signal(Args... args) = delete;
	~signal() = default;

	std::tuple<Args...> call_args;

	signal<Args...>& operator()(Args... args)
	{
		call_args = std::tuple<Args...>(args...);
		// emitter<decltype(this)> dummy(this);
		//emit_signal(std::forward<Args...>(args...));
		return *this;
	}

	void delayed_dipatch()
	{
		callFunc(typename gens<sizeof...(Args)>::type());
	}

	template<int ...S>
	void callFunc(seq<S...>)
	{
		emit_signal(std::get<S>(call_args) ...);
	}

	void emit_signal(Args... args)
	{
		for (auto& sh : sholders) {
			sh->run_slots(args...);
		}
	}

	struct slot_holder_base
	{
		virtual void run_slots(Args...) = 0;
	};

	template<class T>
	struct slot_holder : public slot_holder_base
	{
		using function_type = std::function<typename std::result_of<T(Args...)>::type(Args...)>;
		struct func_and_bool
		{
			std::shared_ptr<function_type> ft;
			bool active;
			void* addr;
		};

		static std::shared_ptr<function_type> make_ptr(function_type ft)
		{
			return std::make_shared<function_type>(ft);
		}

		using slot_vec_type = std::vector<func_and_bool>;
		slot_vec_type slots;

		virtual void run_slots(Args... args) override
		{
			for (auto& s : slots)
			{
				if (s.active)
				{
					auto x = s.ft.get();
					(*x)(args...);
				}
			}
		}
	};

	template<class T>
	void connect(T&& f, bool active = true)
	{
		static slot_holder<T> sh;
		auto pft = slot_holder<T>::make_ptr(std::forward<T>(f));
		typename slot_holder<T>::func_and_bool fb{ pft, active, reinterpret_cast<void*>(&f) };

		using function_type = std::function<typename std::result_of<T(Args...)>::type(Args...)>;


		for (auto& s : sh.slots)
		{
			if (s.addr == fb.addr)
			{
				if (s.active != active)
				{
					s.active = active;
				}
			}
		}

		sh.slots.emplace_back(fb);
		if (std::find(sholders.begin(), sholders.end(), static_cast<slot_holder_base*>(&sh)) == sholders.end())
		{
			sholders.push_back(&sh);
		}
	}

	template<class T>
	void disconnect(T&& f)
	{
		connect<T>(std::forward<T>(f), false);
	}

	std::vector<slot_holder_base*> sholders;
};

template<>
class signal<void>
{
	struct slots_holder
	{
		virtual void run() = 0;
	};

	template <class T>
	struct slots : public slots_holder
	{
		std::vector<T> slot_container;

		virtual void run()
		{
			for (auto & s : slot_container)
			{
				s();
			}
		}
	};

public:

	void connect(void(&fun)())
	{
		using funtype = std::function<void()>;
		connect<funtype>(fun);
	}

	template <class T>
	void connect(T&& f)
	{
		static slots<T> s;
		auto c = std::find(shs.begin(), shs.end(), static_cast<slots_holder*>(&s));
		if (c == shs.end())
		{
			shs.push_back(&s);
		}

		s.slot_container.push_back(std::forward<T>(f));
	}

	signal<void>& operator()()
	{
		emit_signal();
		return *this;
	}

	void emit_signal()
	{
		for (auto& sh : shs)
		{
			sh->run();
		}
	}

	void delayed_dipatch()
	{
		emit_signal();
	}

private:

	std::vector<slots_holder*> shs;
};

/* Just a tiny syntax helper so that we can write emit my_signal(x,y,z); */
template<class T>
struct emitter 
{
public:

	emitter(const T& emtr)
	{
		senderObj = reinterpret_cast<const T*>(&emtr);
		minstance = this;

		std::cout << "Emitter Constructor for " << typeid(emtr).name() << " (minstnace)this=" << (void*)(this) << " sedner=" << (void*)senderObj << std::endl;

	}

	~emitter()
	{
		std::cout << "Emitter Destructor for " << typeid(senderObj).name()  << " (minstnace)this=" << (void*)(this) << " sedner=" << (void*)senderObj << std::endl;
		senderObj = nullptr;
		minstance = nullptr;
	}

	static T* sender()
	{
		return const_cast<T*>(senderObj);
	}

	static emitter<T>* instance()
	{
		std::cout << "Emitter instance for " << typeid(minstance).name() << std::endl;

		return minstance;
	}

private:

	static const T* senderObj;
	static emitter<T>* minstance;
};
template<class T> const T* emitter<T>::senderObj = nullptr;
template<class T> emitter<T>* emitter<T>::minstance = nullptr;

template<class T>
T* sender()
{
	auto emns = emitter<T>::instance();
	return emns->sender();
}



template<class T, class... Args>
emitter<T>&& operator << (emitter<T>&& e, signal<Args...>& s)
{
	s.delayed_dipatch();
	return std::forward<emitter<T>>(e);
}
#define emit emitter<std::remove_pointer<decltype(this)>::type>(*this)<<


template<class Si, class So>
void connect_d(Si&& sig, So&& slo)
{
	std::forward<Si>(sig).connect(std::forward<So>(slo));
}

#define connect(a,b,c) connect_d(a.b,c)


#endif
