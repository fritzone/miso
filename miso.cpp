#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <tuple>

#include <iostream>

template <class... Args>
class signal
{
public:
    signal() = default;
    signal(Args... args) = delete;

    void emit(Args... args)
    {
        for (auto sh : sholders)
        {
            sh->run_slots(args...);
        }
    }

    struct slot_holder_base
    {
        virtual void run_slots(Args...) = 0;
    };

    template <class R, class... FArgs>
    struct multislot
    {
        typedef std::function<R(FArgs...)> slot_type;
    };


    template<class T>
    struct slot_holder : public slot_holder_base
    {

        using ret_type = typename std::result_of<T(Args...)>::type;
        using ms_type = typename multislot<ret_type, Args...>::slot_type;
        std::vector<ms_type> slots;

        virtual void run_slots(Args... args) override
        {
            for (auto s : slots)
            {
                s(args...);
            }
        }
    };

    template<class T>
    void connect(T&& f)
    {
        using ret_type = typename std::result_of<T(Args...)>::type;
        using ms_type = typename multislot<ret_type, Args...>::slot_type;
        static std::vector<ms_type> slots;

        static slot_holder<T> sh;

        sh.slots.push_back(std::forward<T>(f));

        auto c = std::find(sholders.begin(), sholders.end(), static_cast<slot_holder_base*>(&sh));
        if (c == sholders.end())
        {
            sholders.push_back(&sh);
        }
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

    void emit()
    {
        for (auto& sh : shs)
        {
            sh->run();
        }
    }

private:

    std::vector<slots_holder*> shs;

};

struct more_class
{
    more_class() = default;

    void run()
    {
        ms.emit(8);
        ms.emit(9);
    }

    signal<int> ms;

};


void global_int_method(int s)
{
    std::cout << "global int method:" << s << std::endl;
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
        click.emit();
    }

    signal<void> click;

};

int main(int argc, char const *argv[])
{
    my_class src;
    other_class dst(4);

    src.click.connect(std::bind(&other_class::clicked, dst));
    src.click.connect(global_void_method);
    src.some_method();

    more_class mc;
    mc.ms.connect(global_int_method);
    mc.ms.connect([](int c) {std::cout << "lambdint:" << c << std::endl; });
    mc.ms.connect(std::bind(&other_class::clicked_again, dst, std::placeholders::_1));

    mc.run();

    return 0;
}
