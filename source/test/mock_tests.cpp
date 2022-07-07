#include "Test.hpp"
#include <memory>
#include "Mock.hpp"


TEST_CASE(mock_value_type_contructor_happy_case)
{
	mock_value_type<int> test_a(10);
	mock_value_type<int> test_b;
	mock_value_type<const char*> test_c("test");
	mock_value_type<const char*> test_d;
	mock_value_type<double> test_e(1.234);
	mock_value_type<char> test_f('x');

	ASSERT(test_a.get() == 10);
	ASSERT(test_a.get_type() == typeid(int));
	ASSERT(test_b.get() == 0);
	ASSERT(test_b.get_type() == typeid(int));
	ASSERT(test_c.get() == std::string("test"));
	ASSERT(test_c.get_type() == typeid(std::string));
	ASSERT(test_d.get() == std::string());
	ASSERT(test_d.get_type() == typeid(std::string));
	ASSERT(test_e.get() == 1.234);
	ASSERT(test_e.get_type() == typeid(double));
	ASSERT(test_f.get() == 'x');
	ASSERT(test_f.get_type() == typeid(char));
}

TEST_CASE(mock_value_type_set)
{
	mock_value_type<int> test_a;
	mock_value_type<const char*> test_b;

	test_a.set(15);
	test_b.set("Hello World");

	ASSERT(test_a.get() == 15);
	ASSERT(test_a.get_type() == typeid(int));
	ASSERT(test_b.get() == std::string("Hello World"));
	ASSERT(test_b.get_type() == typeid(std::string));
}

TEST_CASE(mock_allocate_wrapper_happy_case)
{
	std::shared_ptr<mock_value_wrapper> test_a(mock_allocate_wrapper(10));
	std::shared_ptr<mock_value_wrapper> test_b(mock_allocate_wrapper(1.234));
	std::shared_ptr<mock_value_wrapper> test_c(mock_allocate_wrapper('x'));
	std::shared_ptr<mock_value_wrapper> test_d(mock_allocate_wrapper("abcd"));

	ASSERT(test_a->get_type() == typeid(int));
	ASSERT(test_b->get_type() == typeid(double));
	ASSERT(test_c->get_type() == typeid(char));
	ASSERT(test_d->get_type() == typeid(std::string));
}

TEST_CASE(mock_allocate_wrappers_happy_case)
{
	auto result_a = mock_allocate_wrappers();
	auto result_b = mock_allocate_wrappers(10);
	auto result_c = mock_allocate_wrappers(10, 1.234, 'x', "abcd");

	ASSERT(result_a.size() == 0);
	ASSERT(result_b.size() == 1);
	ASSERT(result_c.size() == 4);

	ASSERT(result_b[0]->get_type() == typeid(int));
	ASSERT(result_c[0]->get_type() == typeid(int));
	ASSERT(result_c[1]->get_type() == typeid(double));
	ASSERT(result_c[2]->get_type() == typeid(char));
	ASSERT(result_c[3]->get_type() == typeid(std::string));
}

static int MockTestFx(int x, int y, int z)
{
	MOCK_CALL(x, y, z);
	MOCK_RETURN(int);
}

static void MockTestGx(int x, int y)
{
	MOCK_CALL(x, y);
}

static void MockTestHx(const char* data, size_t size)
{
	MOCK_CALL(MockData(data, size));
}

static void MockTestIx(int* out)
{
	MOCK_CALL();
	MOCK_OUT(*out);
}

//static void MockTestJx(const char* in_data, size_t in_size, char* in_out_data, size_t in_out_size, char* out_data, size_t out_size)
//{
//	MOCK_CALL(MockData(in_data, in_size), MockData(in_out_data, in_out_size));
//	MOCK_OUT(MockData(in_out_data, in_out_size), MockData(out_data, out_size));
//}

TEST_CASE(MOCK_HappyCase)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_RETURN(10);
		EXPECT(MockTestGx(3, 4));

		int value = MockTestFx(1, 2, 3);
		MockTestGx(3, 4);

		ASSERT(value == 10);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(test_case.Run());
}

TEST_CASE(MOCK_InvalidInput)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_RETURN(10);
		EXPECT(MockTestGx(3, 4));

		MockTestFx(1, 5, 3);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_MismatchedCalls)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_RETURN(10);
		EXPECT(MockTestGx(3, 4));

		MockTestGx(3, 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_AndThrowWithReturn)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_THROW(std::runtime_error("test"));
		EXPECT(MockTestGx(3, 4));

		ASSERT_THROWS(MockTestFx(1, 2, 3));
		MockTestGx(3, 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(test_case.Run());
}

TEST_CASE(MOCK_AndThrowNoReturn)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_RETURN(10);
		EXPECT(MockTestGx(3, 4))_AND_THROW(std::runtime_error("test"));

		int value = MockTestFx(1, 2, 3);
		ASSERT_THROWS(MockTestGx(3, 4));

		ASSERT(value == 10);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(test_case.Run());
}

TEST_CASE(MOCK_MultipleActions)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_RETURN(10)_AND_RETURN(11);
		EXPECT(MockTestGx(3, 4));

		int value = MockTestFx(1, 2, 3);
		MockTestGx(3, 4);

		ASSERT(value == 10);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_MissingCall)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_RETURN(10);
		EXPECT(MockTestGx(3, 4));

		int value = MockTestFx(1, 2, 3);

		ASSERT(value == 10);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_NoReturn)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3));

		MockTestFx(1, 2, 3);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

void MockTestCallback()
{
	MockTestGx(9, 8);
	MockTestGx(7, 6);
}

TEST_CASE(MOCK_Callback_HappyCase)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_DO(MockTestCallback())_AND_RETURN(10);
		EXPECT(MockTestGx(9, 8));
		EXPECT(MockTestGx(7, 6));
		EXPECT(MockTestGx(3, 4));

		MockTestFx(1, 2, 3);
		MockTestGx(3, 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(test_case.Run());
}

TEST_CASE(MOCK_Callback_MissingCall)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_DO(MockTestCallback())_AND_RETURN(10);
		EXPECT(MockTestGx(9, 8));
		EXPECT(MockTestGx(7, 6));
		EXPECT(MockTestGx(5, 4));
		EXPECT(MockTestGx(3, 4));

		MockTestFx(1, 2, 3);
		MockTestGx(3, 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_Callback_ExtraCall)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_DO(MockTestCallback())_AND_RETURN(10);
		EXPECT(MockTestGx(9, 8));
		EXPECT(MockTestGx(3, 4));

		MockTestFx(1, 2, 3);
		MockTestGx(3, 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_Callback_EmptyCallback)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_DO()_AND_RETURN(10);
		EXPECT(MockTestGx(3, 4));

		MockTestFx(1, 2, 3);
		MockTestGx(3, 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(test_case.Run());
}

TEST_CASE(MOCK_Callback_DoubleCallback)
{
	auto test = [] {
		EXPECT(MockTestFx(1, 2, 3))_AND_DO(MockTestCallback())_AND_DO(MockTestCallback())_AND_RETURN(10);
		EXPECT(MockTestGx(9, 8));
		EXPECT(MockTestGx(7, 6));
		EXPECT(MockTestGx(9, 8));
		EXPECT(MockTestGx(7, 6));
		EXPECT(MockTestGx(3, 4));

		MockTestFx(1, 2, 3);
		MockTestGx(3, 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_MockData_HappyCase)
{
	auto test = [] {
		EXPECT(MockTestHx("ABCD", 4));

		MockTestHx("ABCD", 4);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(test_case.Run());
}

TEST_CASE(MOCK_MockData_WrongSize)
{
	auto test = [] {
		EXPECT(MockTestHx("ABCD", 4));

		MockTestHx("ABCD", 3);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(!test_case.Run());
}

TEST_CASE(MOCK_OUT_HappyCase)
{
	auto test = [] {
		int in = 0x1234;
		EXPECT(MockTestIx(&in));
		ASSERT(in == 0x1234);

		int out = 0;
		MockTestIx(&out);
		ASSERT(out == 0x1234);
	};
	TestCaseListItem test_case(test, __FUNCTION__, __FILE__, __LINE__);

	ASSERT(test_case.Run());
}

