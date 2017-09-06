#include "miso.h"
#include <iostream>
/*
struct other_class
{
    void method() const
    {
        std::cout << "Other class method";
    }
};

struct a_class
{
    miso::signal<const char*> m_s;
    miso::signal<other_class> m_s2;

    void say_hello()
    {
        emit m_s("Hello from a class");

        emit m_s2(other_class());

    }
};

void b_function(other_class oc)
{
    oc.method();
}

void a_function(const char* msg)
{
    std::cout << msg << std::endl;
}

int main()
{
    a_class a;
    miso::connect(a.m_s, a_function);
    miso::connect(a.m_s, [](const char* s) { std::cout << "I'm a lambda and still say: " << s << std::endl; });

    miso::connect(a.m_s2, b_function);

    a.say_hello();
}

*/

#include <ctime>
#include <string>

using namespace miso;

struct more_class
{
    more_class() = default;

    void run(int v)
    {
        emit ms(v);
        emit os("Hurra", v);
    }

    miso::signal<int> ms;
    miso::signal<std::string, int> os;
    miso::signal<float, int> float_int_sig;
};

void other_global_int_method(int s)
{
    std::cout << "other global int method:" << s << std::endl;
}

void global_with_two_parameters(const std::string& s, int i)
{
    std::cout << "Two param global: s=" << s << " i=" << i << std::endl;
}

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

	my_class(int f) : mf(f) {}

    void some_method()
    {
        emit click();
    }

    miso::signal<> click;


	int mf = 0;

};


void global_int_method(int s)
{
	more_class* x = sender<more_class>();
	std::cout << "global int method:" << s << std::endl;

	if(s == 7) x->run(1234);
}


void global_void_method()
{
	std::cout << "global void method:" << std::endl;
	auto x = sender<my_class>();
	std::cout << "is:" << x << " " << x->mf << std::endl ;
}

struct functor
{
	void operator()()
	{
		std::cout << "functor class" << std::endl;
	}

	void operator()(int aa)
	{
		std::cout << "functor class's int slot:" << aa << std::endl;
	}
};


int main(int argc, char const *argv[])
{
    my_class src(56);
    std::cout << "mc_addre" << &src << std::endl;
    other_class dst(4);
    functor f;


    auto lambdv = []() {std::cout << "lambvoid" << std::endl; };
    miso::connect(src.click, lambdv);
    miso::connect(src.click, std::bind(&other_class::clicked, dst));
    miso::connect(src.click, global_void_method);
    miso::connect(src.click, f);

    src.some_method();

    src.click.disconnect(lambdv);

    src.some_method();

    more_class mc;
    connect(mc.ms, global_int_method);
    connect(mc.ms, other_global_int_method);
    connect(mc.ms, f);

    connect(mc.os, global_with_two_parameters);

    auto lambdi = [](int c) {std::cout << "lambdint:" << c << std::endl; };
    connect(mc.ms, lambdi);
    connect(mc.ms, std::bind(&other_class::clicked_again, dst, std::placeholders::_1));

    std::cout << "\nOriginal\n" << std::endl;
	
	mc.run(7);

	std::cout << "\nDisconnect\n" << std::endl;

	mc.ms.disconnect(global_int_method);
	mc.ms.disconnect(other_global_int_method);
	mc.ms.disconnect(lambdi);

	std::cout << "\nAfter disconnect\n" << std::endl;

	mc.run(8);
    return 0;
}
/**/