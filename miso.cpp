#include "miso.h"
#include <iostream>

/* This is a file you definitely don't need in your project */

struct d_class
{
    int hh = 67;
};

struct a_class {
    miso::signal<int> m_s;
    miso::signal<d_class> m_s2;

    void say_hello() {
        emit m_s(42);
        emit m_s2(d_class());
    }
};


struct functor {
    void operator()(int aa) {
        std::cout << "functor class's int slot:" << aa << std::endl;
    }

    void operator()(d_class aa) {
        std::cout << "functor class's d_class slot:" << aa.hh << std::endl;
    }

};

 auto retFun = []() {
     return []() -> functor {return functor(); };
 };

int main() {
    a_class a;
    functor f;

    miso::connect(a.m_s, f);
    miso::connect(a.m_s2, retFun()());

    a.say_hello();
}
/*
struct d_class
{
    int hh = 67;
};

struct a_class {
    miso::signal<d_class> m_s;

    void say_hello() {
        emit m_s(d_class());
    }
    int x = 45;
};
struct functor {
    void operator()(int aa) {
        std::cout << "functor class's int slot:" << aa << std::endl;
        auto ap = miso::sender<a_class>();
        std::cout << "x in emitter class:" << ap->x << std::endl;    
    }
    int x = 6;
};
int main() {
    a_class a;
    functor f;
    miso::connect(a.m_s, f);
    a.say_hello();
} */

/**/


/*
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
