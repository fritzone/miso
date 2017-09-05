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
            std::for_each(sh.slots.begin(), sh.slots.end(),
                          [&](func_and_bool<FT> &s) { if (s.addr == fb.addr) s.active = active; });
            sh.slots.emplace_back(fb);
            if (std::find(sholders.begin(), sholders.end(), static_cast<common_slot_base *>(&sh)) == sholders.end()) {
                sholders.push_back(&sh);
            }
        }
    }

	template <class... Args>
	class signal final
	{
        std::vector<internal::common_slot_base*> slot_holders;
        std::tuple<Args...> call_args;
	public:
		signal() = default;
		signal(Args... args) = delete;
		~signal() = default;

		signal<Args...>& operator()(Args... args) {
			call_args = std::tuple<Args...>(args...);
			return *this;
		}

		void delayed_dispatch()	{
			delayed_call(typename internal::sequence_generator<sizeof...(Args)>::type());
		}

		template<int ...S>
		void delayed_call(internal::sequence<S...>)	{
			emit_signal(std::get<S>(call_args) ...);
		}

		void emit_signal(Args... args) {
			for (auto& sh : slot_holders) {
				(dynamic_cast<slot_holder_base*>(sh))->run_slots(args...);
			}
		}

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

		template<class T>
		void connect(T&& f, bool active = true) {
			internal::connect_i<T, typename slot_holder<T>::FT,
			          slot_holder<T>> (std::forward<T>(f), slot_holders, active);
		}

		template<class T>
		void disconnect(T&& f) {
			connect<T>(std::forward<T>(f), false);
		}
	};

    template<>
	class signal<void> final {
        std::vector<internal::common_slot_base*> slot_holders;
    public:
		signal() = default;
		~signal() = default;

		signal<void>& operator()() {
			return *this;
		}

		void delayed_dispatch() {
			emit_signal();
		}

		struct slot_holder_base : public internal::common_slot_base {
			virtual void run_slots() = 0;
		};

		template<class T>
		struct slot_holder : public slot_holder_base {
			using FT = std::function<void(void)>;
			using slot_vec_type = std::vector<internal::func_and_bool<FT>>;
			slot_vec_type slots;

			void run_slots() override {
				std::for_each(slots.begin(), slots.end(),
					[&](internal::func_and_bool<FT>& s) {
						if (s.active) {
							(*(s.ft.get()))();
						}
					}
				);
			}
		};

		void emit_signal() {
			std::for_each(slot_holders.begin(), slot_holders.end(),
				[](internal::common_slot_base* sh)
				{
					(dynamic_cast<slot_holder_base*>(sh))->run_slots(); 
				}
			);
		}

		template<class T>
		void connect(T&& f, bool active = true)	{
            internal::connect_i<T, typename slot_holder<T>::FT, slot_holder<T>>(std::forward<T>(f), slot_holders, active);
		}

		template<class T>
		void disconnect(T&& f) {
			connect<T>(std::forward<T>(f), false);
		}
	};

    namespace internal {
        template<class T>
        struct emitter final
        {
            explicit emitter(const T &emtr) {
                senderObjs.push(&emtr);
                minstance = this;
            }

            ~emitter() {
                senderObjs.pop();
                minstance = nullptr;
            }

            static T *sender() {
                return const_cast<T *>(senderObjs.top());
            }

            static emitter<T> *instance() {
                return minstance;
            }

        private:

            static std::stack<const T *> senderObjs;
            static emitter<T> *minstance;
        };

        template<class T> std::stack<const T *> emitter<T>::senderObjs;
        template<class T> emitter<T> *emitter<T>::minstance = nullptr;

        template<class T, class... Args>
        emitter<T> &&operator<<(emitter<T> &&e, signal<Args...> &s) {
            s.delayed_dispatch();
            return std::forward<emitter<T>>(e);
        }
    }
    template<class Si, class So>
    void connect(Si &&sig, So &&slo) {
        static_assert(!std::is_same<So, std::nullptr_t>::value, "cannot use nullptr as slot");
        std::forward<Si>(sig).connect(std::forward<So>(slo));
    }

    template<class T>
    T *sender() {
        return internal::emitter<T>::instance()->sender();
    }

#define emit miso::internal::emitter<std::remove_pointer<decltype(this)>::type>(*this)<<

}


#endif
