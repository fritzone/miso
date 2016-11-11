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
#include <stack>

namespace miso
{
	template<int ...> struct sequence {};
	template<int N, int ...S> struct sequence_generator : sequence_generator<N - 1, N - 1, S...> {};
	template<int ...S> struct sequence_generator<0, S...> { typedef sequence<S...> type; };

	template<typename function_type>
	struct func_and_bool
	{
		std::shared_ptr<function_type> ft;
		bool active;
		void* addr;
	};

	template<typename function_type>
	static std::shared_ptr<function_type> make_ptr(function_type ft)
	{
		return std::make_shared<function_type>(ft);
	}

	struct common_slot_base
	{
		virtual ~common_slot_base() = default;
	};

	template<typename T, typename U>
	constexpr bool types_equal() { return std::is_same<T, U>::value; }

	template<class T, class function_type, class SHT>
	void connect_i(T&& f, std::vector<common_slot_base*>& sholders, bool active = true)
	{
		static SHT sh;
		auto pft = make_ptr<function_type>(std::forward<T>(f));
		func_and_bool<function_type> fb{ pft, active, reinterpret_cast<void*>(&f) };

		std::for_each(sh.slots.begin(), sh.slots.end(), [&](func_and_bool<function_type>& s) {if (s.addr == fb.addr) s.active = active; });
		sh.slots.emplace_back(fb);
		if (std::find(sholders.begin(), sholders.end(), static_cast<common_slot_base*>(&sh)) == sholders.end())
		{
			sholders.push_back(&sh);
		}
	}

	template <class... Args>
	class signal final
	{
	public:
		signal() = default;
		signal(Args... args) = delete;
		~signal() = default;
		signal<Args...>& operator()(Args... args)
		{
			call_args = std::tuple<Args...>(args...);
			return *this;
		}

		void delayed_dipatch()
		{
			callFunc(typename sequence_generator<sizeof...(Args)>::type());
		}

		template<int ...S>
		void callFunc(sequence<S...>)
		{
			emit_signal(std::get<S>(call_args) ...);
		}

		void emit_signal(Args... args)
		{
			for (auto& sh : sholders)
			{
				(dynamic_cast<slot_holder_base*>(sh))->run_slots(args...);
			}
		}

		struct slot_holder_base : public common_slot_base
		{
			virtual void run_slots(Args...) = 0;
		};

		template<class T>
		struct slot_holder : public slot_holder_base
		{
			using function_type = std::function<typename std::result_of<T(Args...)>::type(Args...)>;
			using slot_vec_type = std::vector<func_and_bool<function_type>>;
			slot_vec_type slots;

			virtual void run_slots(Args... args) override
			{
				std::for_each(slots.begin(), slots.end(), [&](func_and_bool<function_type>& s) 
					{
						if (s.active) (*(s.ft.get()))(args...); 
					}
				);
			}
		};

		template<class T>
		void connect(T&& f, bool active = true)
		{
			connect_i<T, typename slot_holder<T>::function_type, 
			          slot_holder<T>> (std::forward<T>(f), sholders, active);
		}

		template<class T>
		void disconnect(T&& f)
		{
			connect<T>(std::forward<T>(f), false);
		}

	private:
		std::vector<common_slot_base*> sholders;
		std::tuple<Args...> call_args;

	};

	template<>
	class signal<void> final
	{
	public:
		signal() = default;
		~signal() = default;

		signal<void>& operator()()
		{
			return *this;
		}

		void delayed_dipatch()
		{
			emit_signal();
		}

		struct slot_holder_base : public common_slot_base
		{
			virtual void run_slots() = 0;
		};

		template<class T>
		struct slot_holder : public slot_holder_base
		{
			using function_type = std::function<void(void)>;
			using slot_vec_type = std::vector<func_and_bool<function_type>>;
			slot_vec_type slots;

			virtual void run_slots() override
			{
				std::for_each(slots.begin(), slots.end(), [&](func_and_bool<function_type>& s) 
					{ 
						if (s.active)
						{
							(*(s.ft.get()))();
						}
					} 
				);
			}
		};
		
		void emit_signal()
		{
			std::for_each(sholders.begin(), sholders.end(), [](common_slot_base* sh) {(dynamic_cast<slot_holder_base*>(sh))->run_slots(); });
		}

		template<class T>
		void connect(T&& f, bool active = true)
		{
			connect_i<T, typename slot_holder<T>::function_type, slot_holder<T>>(std::forward<T>(f), sholders, active);
		}

		template<class T>
		void disconnect(T&& f)
		{
			connect<T>(std::forward<T>(f), false);
		}

		std::vector<common_slot_base*> sholders;
	};

	template<class T>
	struct emitter final
	{
		emitter(const T& emtr)
		{
			senderObjs.push(reinterpret_cast<const T*>(&emtr));
			minstance = this;
		}

		~emitter()
		{
			senderObjs.pop();
			minstance = nullptr;
		}

		static T* sender()
		{
			return const_cast<T*>(senderObjs.top());
		}

		static emitter<T>* instance()
		{
			return minstance;
		}

	private:

		static std::stack<const T*> senderObjs;
		static emitter<T>* minstance;
	};
	template<class T> std::stack<const T*> emitter<T>::senderObjs;
	template<class T> emitter<T>* emitter<T>::minstance = nullptr;

	template<class T>
	T* sender()
	{
		return emitter<T>::instance()->sender();
	}

	template<class T, class... Args>
	emitter<T>&& operator << (emitter<T>&& e, signal<Args...>& s)
	{
		s.delayed_dipatch();
		return std::forward<emitter<T>>(e);
	}

	template<class Si, class So>
	void connect_p(Si&& sig, So&& slo)
	{
		static_assert(! std::is_same<So, std::nullptr_t>::value , "cannot use nullptr as slot");
		std::forward<Si>(sig).connect(std::forward<So>(slo));
	}
}

#define connect(a,b,c) miso::connect_p(a.b,c)
#define emit miso::emitter<std::remove_pointer<decltype(this)>::type>(*this)<<

#endif
