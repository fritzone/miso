#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <tuple>

#include <iostream>

template <class... Args>
class signal final
{
public:
    signal() = default;
	signal(Args... args) = delete;
	~signal() = default;

	signal<Args...>& operator()(Args... args)
	{
		emit_signal(std::forward<Args...>(args...));
		return *this;
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
		slot_holder<T>::func_and_bool fb{ pft, active, static_cast<void*>(&f) };

		using function_type = std::function<typename std::result_of<T(Args...)>::type(Args...)>;


		for (auto& s : sh.slots)
		{
			if(s.addr == fb.addr)
			{
				if (s.active != active)
				{
					s.active = active;
				}
			}
		}

		sh.slots.emplace_back( fb );
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

    void connect( void(&fun)() )
    {
        using funtype = std::function<void()>;
        connect<funtype>(fun);
    }

    template <class T>
    void connect (T&& f)
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

private:

    std::vector<slots_holder*> shs;
};

/* Just a tiny syntax helper so that we can write emit my_signal(x,y,z); */
class emitter {};
template<class... Args>
emitter& operator << (emitter& e, const signal<Args...>& s)
{
	return e;
}
#define emit emitter() <<


struct more_class
{
    more_class() = default;

    void run()
    {
        emit ms(8);
        emit ms(9);
    }

    signal<int> ms;
};


void global_int_method(int s)
{
    std::cout << "global int method:" << s << std::endl;
}

void other_global_int_method(int s)
{
	std::cout << "other global int method:" << s << std::endl;
}


void global_void_method()
{
    std::cout << "global void method:" << std::endl;
}

class slot
{

};

class other_class
{
public:
    other_class() = default;
    other_class(int v) : nr(v) {}

    void clicked()
    {
        std::cout << "clicked:" << nr << std::endl;
    }

    void clicked_again(int v)
    {
        std::cout << "clicked_again:" << v << std::endl;
    }


private:

    int nr = 23;
};

class my_class
{
public:

    void some_method()
    {
        emit click();
    }

    signal<void> click;

};

template<class Si, class So>
void connect_d(Si&& sig, So&& slo)
{
	std::forward<Si>(sig).connect(std::forward<So>(slo));
}


#define connect(a,b,c) connect_d(a.b,c)

int main(int argc, char const *argv[])
{
    my_class src;
    other_class dst(4);

	connect(src, click, std::bind(&other_class::clicked, dst));
	connect(src, click, global_void_method);

//    src.click.connect(std::bind(&other_class::clicked, dst));
//    src.click.connect(global_void_method);
    src.some_method();

    more_class mc;
    connect(mc, ms, global_int_method);
	connect(mc, ms, other_global_int_method);

	auto lambdi = [](int c) {std::cout << "lambdint:" << c << std::endl; };

    connect(mc, ms, lambdi);
    connect(mc, ms, std::bind(&other_class::clicked_again, dst, std::placeholders::_1));

	std::cout << "\nOriginal\n" << std::endl;
	emit mc.ms(8);

	std::cout << "\nDisconnect\n" << std::endl;

	mc.ms.disconnect(global_int_method);
	mc.ms.disconnect(other_global_int_method);
	mc.ms.disconnect(lambdi);

	std::cout << "\nAfter disconnect\n" << std::endl;

	emit mc.ms(6);

    return 0;
}
