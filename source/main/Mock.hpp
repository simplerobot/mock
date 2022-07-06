#pragma once

#ifndef __cplusplus
#error Mock library requires c++.
#endif

#ifndef __GXX_RTTI
#error Mock library requires run time type information.
#endif

#ifndef TEST
#error Mock library only runs when testing.
#endif


#include <typeinfo>
#include <vector>
#include <string>
#include <iostream>
#include <functional>


#define EXPECT(CALL) mock_begin_expect(#CALL, __FILE__, __LINE__); CALL ; mock_end_expect(#CALL)
#define _AND_RETURN(VALUE) ; mock_add_return(mock_allocate_wrapper(VALUE), #VALUE)
#define _AND_THROW(EXCEPTION) ; mock_add_exception(mock_allocate_wrapper_simple(EXCEPTION))

#define MOCK_CALL(...) std::vector<mock_value_wrapper*> mock_params = mock_allocate_wrappers(__VA_ARGS__); mock_call(mock_params, __PRETTY_FUNCTION__)
#define MOCK_RETURN(TYPE) mock_value_type<TYPE> mock_result; mock_return(&mock_result, __PRETTY_FUNCTION__); return mock_result.get()

#define EXPECT_BEGIN_CALLBACK(CALL) mock_begin_callback([&](){ CALL; })
#define EXPECT_END_CALLBACK() mock_end_callback();

class mock_value_wrapper
{
public:
	virtual ~mock_value_wrapper() = default;

	virtual const std::type_info& get_type() const = 0;
	virtual void write(std::ostream&) const = 0;
	virtual bool equals(const mock_value_wrapper*) const = 0;
	virtual bool set(const mock_value_wrapper*) = 0;
	virtual void throw_exception() const = 0;
};

template <typename T>
class mock_value_simple_type : public mock_value_wrapper
{
public:
	mock_value_simple_type()
		: m_value()
	{
	}

	mock_value_simple_type(const T& value)
		: m_value(value)
	{
	}

	virtual const std::type_info& get_type() const override
	{
		return typeid(T);
	}

	virtual void write(std::ostream& out) const override
	{
		throw std::runtime_error("not implemented");
	}

	virtual bool equals(const mock_value_wrapper* second) const override
	{
		throw std::runtime_error("not implemented");
	}

	virtual bool set(const mock_value_wrapper* second) override
	{
		if (this->get_type() != second->get_type())
			return false;
		const mock_value_simple_type<T>* second_t = (const mock_value_simple_type*)second;
		m_value = second_t->get();
		return true;
	}

	virtual void throw_exception() const override
	{
		throw m_value;
	}

	T get() const
	{
		return m_value;
	}

	void set(const T& value)
	{
		m_value = value;
	}

private:
	T m_value;
};

template <typename T>
class mock_value_type : public mock_value_simple_type<T>
{
public:
	mock_value_type()
	{
	}

	mock_value_type(const T& value)
		: mock_value_simple_type<T>(value)
	{
	}

	virtual void write(std::ostream& out) const override
	{
		out << this->get();
	}

	virtual bool equals(const mock_value_wrapper* second) const override
	{
		if (this->get_type() != second->get_type())
			return false;
		const mock_value_type<T>* second_t = (const mock_value_type*)second;
		return (this->get() == second_t->get());
	}

};

template <>
class mock_value_simple_type<const char*> : public mock_value_simple_type<std::string>
{
public:
	mock_value_simple_type()
	{
	}

	mock_value_simple_type(const char* value)
		: mock_value_simple_type<std::string>(value)
	{
	}
};

template <size_t SIZE>
class mock_value_simple_type<char[SIZE]> : public mock_value_simple_type<std::string>
{
public:
	mock_value_simple_type()
	{
	}

	mock_value_simple_type(const char* value)
		: mock_value_simple_type<std::string>(value)
	{
	}
};

template <typename T>
mock_value_wrapper* mock_allocate_wrapper_simple(const T& value)
{
	return new mock_value_simple_type<T>(value);
}

template <typename T>
mock_value_wrapper* mock_allocate_wrapper(const T& value)
{
	return new mock_value_type<T>(value);
}

void mock_allocate_wrappers_to_vector(std::vector<mock_value_wrapper*>& result);

template <typename T, typename... TS>
void mock_allocate_wrappers_to_vector(std::vector<mock_value_wrapper*>& result, const T& value, TS... ts)
{
	result.push_back(mock_allocate_wrapper(value));
	mock_allocate_wrappers_to_vector(result, ts...);
}

template <typename... TS>
std::vector<mock_value_wrapper*> mock_allocate_wrappers(TS... ts)
{
	std::vector<mock_value_wrapper*> result;
	mock_allocate_wrappers_to_vector(result, ts...);
	return result;
}


extern void mock_reset();
extern void mock_verify();
extern void mock_begin_expect(const char* call_str, const char* file_name, size_t line);
extern void mock_end_expect(const char* call_str);
extern void mock_add_return(mock_value_wrapper* value, const char* value_str);
extern void mock_add_exception(mock_value_wrapper* exception);
extern void mock_call(const std::vector<mock_value_wrapper*>& params, const char* function_name_str);
extern void mock_return(mock_value_wrapper* result, const char* function_name_str);
extern void mock_begin_callback(std::function<void()> callback);
extern void mock_end_callback();
