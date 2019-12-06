#include <iostream>
#include <memory>
#include <functional>
#include <typeinfo>

using namespace std;

template<typename T>
class Sink;

template<typename S, typename T>
class Func;

template<typename S, typename T>
class FuncSink;

template<typename S, typename M, typename T>
class ComposeFunc;

template<typename T>
class Sink : public std::enable_shared_from_this<Sink<T>> {
public:
	virtual ~Sink() {}
	virtual void operator()(const T& input) = 0;
	shared_ptr<Sink<T>> getPtr() { return this->shared_from_this(); }
};

template<typename S, typename T>
class Func : public std::enable_shared_from_this<Func<S,T>> {
public:
	virtual ~Func() {}
	virtual void operator()(const S& input, Sink<T>& sink) = 0;
	shared_ptr<Func<S,T>> getPtr() { return this->shared_from_this(); }
	
	virtual shared_ptr<Sink<S>> apply(shared_ptr<Sink<T>> sink) {
		return make_shared<FuncSink<S,T>>(getPtr(), sink);
	}
	
	template<typename U>
	shared_ptr<Func<S, U>> compose(shared_ptr<Func<T, U>> next) {
		return make_shared<ComposeFunc<S,T,U>>(getPtr(), next);
	}

};

template<typename S, typename T>
class FuncSink : public Sink<S> {
public:
	FuncSink(shared_ptr<Func<S,T>> func, shared_ptr<Sink<T>> sink) : func_(func), sink_(sink) {}
	void operator()(const S& input) {
		(*func_)(input, *sink_);
	}
	shared_ptr<Func<S,T>> func_;
	shared_ptr<Sink<T>> sink_;
};

template<typename S, typename T>
shared_ptr<Sink<T>> apply(shared_ptr<Func<S,T>> func, shared_ptr<Sink<T>> sink) {
	return func->apply(sink);
}

template<typename S, typename T>
class PtrFuncSink : public Sink<S> {
public:
	PtrFuncSink(Func<S,T>* func) : func_(func), sink_(NULL) {}
	PtrFuncSink(Func<S,T>* func, Sink<T>* sink) : func_(func), sink_(sink) {}
	void operator()(const S& input) {
		(*func_)(input, *sink_);
	}
	Func<S,T>* func_;
	Sink<T>* sink_;
};

template<typename S, typename M, typename T>
class ComposeFunc : public Func<S,T> {
public:
	ComposeFunc(shared_ptr<Func<S,M>> first, shared_ptr<Func<M,T>> second) : first_(first), second_(second) {}
	void operator()(const S& input, Sink<T>& sink) {
		 PtrFuncSink<M,T> m_sink(second_.get(), &sink);
		(*first_)(input, m_sink);
	}
	
	// optimized apply.
	virtual shared_ptr<Sink<S>> apply(shared_ptr<Sink<T>> sink) {
		return first_->apply(second_->apply(sink));
	}
	
	shared_ptr<Func<S,M>> first_;
	shared_ptr<Func<M,T>> second_;
};

template<typename S, typename M, typename T>
shared_ptr<Func<S,T>> compose(shared_ptr<Func<S,M>> first, shared_ptr<Func<M,T>> second) {
	return first->compose(second);
}

template<typename T>
class PrintSink : public Sink<T> {
	void operator()(const T& input) {
		cout << input << endl;
	}
};

class DoubleFunc : public Func<int, int> {
	void operator()(const int& value, Sink<int>& sink) {
		sink(value * 2);
		sink(value * 2 + 1);
	}
};

void simpPrintInt(const int& value) {
	cout << value << endl;
}


void simpDoubleInt(const int& value, Sink<int>& next) {
	next(value * 2);
	next(value * 2 + 1);
}

int main() {
	shared_ptr<Sink<int>> sink = make_shared<PrintSink<int>>();
	shared_ptr<Func<int, int>> func = make_shared<DoubleFunc>();
	auto ret = func->compose(func)->apply(sink);
	// auto ret = apply(compose(func, func), sink);
	// auto ret = apply(func, apply(func, sink));
	// auto ret = func->apply(func->apply(sink));
	int n;
	while(1) {
		cin >> n;
		(*ret)(n);
	}
	return 0;
}

