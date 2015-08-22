//
//  main.cpp
//  ArticlePredicateTechniques
//
//  Created by Gabriel Aubut-Lussier on 2015-08-17.
//  Copyright (c) 2015 Gabriel Aubut-Lussier. All rights reserved.
//

#include <functional>
#include <iostream>
#include <vector>

class A
{
};
class B
{
public:
	B(int i, A* a) : i(i), a(a) {}
	
	A* getA() {return a;}
	A* getA() const {return a;}
	
public:
	int i;
	A* a;
};

bool predicate0(B* b);
bool predicate0(B* b)
{
	return b->getA() == nullptr;
}

struct Functor
{
	Functor(A* a) : a(a) {}
	
	bool operator()(B* b) const
	{
		return b->getA() == a;
	}
	
public:
	A* a;
};

struct FunctorWrapper
{
	FunctorWrapper(std::function<bool(A*, A*)> functor, A* a) : functor(functor), a(a) {}
	
	bool operator()(B* b) const
	{
		return functor(a, b->getA());
	}
	
public:
	std::function<bool(A*, A*)> functor;
	A* a;
};

struct MethodWrapper
{
	MethodWrapper(A*(B::*method)()) : method(method) {}
	
	A* operator()(B* instance) const
	{
		return (instance->*method)();
	}
	
	A*(B::*method)();
};

struct ComposingWrapper
{
	ComposingWrapper(std::function<bool(A*)> f, std::function<A*(B*)> g) : f(f), g(g) {}
	
	bool operator()(B* b)
	{
		return f(g(b));
	}
	
public:
	std::function<bool(A*)> f;
	std::function<A*(B*)> g;
};

template<typename R, typename F, typename ... ArgTypes>
auto factory(R* r, F f, ArgTypes... args)
{
	return std::bind(std::equal_to<R*>(), std::bind(f, std::placeholders::_1, args...), r);
}

void printResults(const char* label, std::vector<B*>& result)
{
	std::cout << label << " : {";
	if (result.size() > 0) {
		std::vector<B*>::const_iterator it = result.begin();
		if (result.size() > 1) {
			std::ostream_iterator<int> out(std::cout, ", ");
			for (std::vector<B*>::const_iterator itEnd = std::prev(result.end()); it != itEnd; ++it) {
				*out = (*it)->i;
			}
		}
		std::cout << (*it)->i;
	}
	std::cout << "}" << std::endl;
	result.clear();
}

int main(int argc, const char * argv[])
{
	A* a1 = new A();
	A* a2 = new A();
	std::vector<B*> objects = {
		new B(1, a1),
		new B(2, a2),
		new B(3, a1),
		new B(4, a2),
		new B(5, a1),
		new B(6, nullptr)
	};
	std::vector<B*> result;
	
	// predicate0
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), predicate0);
	printResults("predicate0", result);
	
	// Functor
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), Functor(a1));
	printResults("Functor(a1)", result);
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), Functor(a2));
	printResults("Functor(a2)", result);
	
	// predicate1
	auto predicate1 = [a2](B* b) -> bool {return b->getA() == a2;};
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), predicate1);
	printResults("predicate1", result);
	
	// predicate2
	auto predicate2 = [a1](B* b) -> bool {
		return std::equal_to<A*>()(b->getA(), a1);
	};
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), predicate2);
	printResults("predicate2", result);
	
	// FunctorWrapper
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), FunctorWrapper(std::equal_to<A*>(), a2));
	printResults("FunctorWrapper", result);
	
	// predicate3
	auto equalToA1 = std::bind(std::equal_to<A*>(), std::placeholders::_1, a1);
	auto predicate3 = [equalToA1](B* b) -> bool {
		return equalToA1(b->getA());
	};
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), predicate3);
	printResults("predicate3", result);
	
	// ComposingWrapper + MethodWrapper
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), ComposingWrapper(equalToA1, MethodWrapper(static_cast<A*(B::*)()>(&B::getA))));
	printResults("ComposingWrapper + MethodWrapper", result);
	
	// predicate4
	auto predicate4 = std::bind(std::equal_to<A*>(), std::bind(static_cast<A*(B::*)() const>(&B::getA), std::placeholders::_1), a2);
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(result), predicate4);
	printResults("predicate4", result);
}
