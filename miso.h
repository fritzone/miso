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
