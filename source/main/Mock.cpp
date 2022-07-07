#include "Mock.hpp"
#include "Test.hpp"
#include <queue>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <memory>
#include <iomanip>


enum MockState
{
	MOCK_STATE_IDLE,
	MOCK_STATE_RECORD_BEGIN,
	MOCK_STATE_RECORD_CALLED,
	MOCK_STATE_RECORD_DONE,
	MOCK_STATE_RECORD_DONE_WAITING_RETURN,
	MOCK_STATE_PLAY_WAITING_OUTPUT,
	MOCK_STATE_PLAY_WAITING_RETURN,
};


static const char* to_string(MockState state)
{
	switch (state)
	{
	case MOCK_STATE_IDLE: return "idle";
	case MOCK_STATE_RECORD_BEGIN: return "begin";
	case MOCK_STATE_RECORD_CALLED: return "called";
	case MOCK_STATE_RECORD_DONE: return "done";
	case MOCK_STATE_RECORD_DONE_WAITING_RETURN: return "done_wait_return";
	case MOCK_STATE_PLAY_WAITING_OUTPUT: return "play_wait_output";
	case MOCK_STATE_PLAY_WAITING_RETURN: return "play_wait_return";
	default: return "<invalid>";
	}
}


class MockFunctionCall
{
public:
	MockFunctionCall(const char* function_name, const std::vector<std::shared_ptr<mock_value_wrapper>>& params, const char* call_str, const char* filename, size_t line);
	MockFunctionCall(const char* function_name, const std::vector<std::shared_ptr<mock_value_wrapper>>& params);

	const char* get_function_name() const { return m_function_name; }
	const char* get_call_string() const { return m_call_string; }
	const char* get_filename() const { return m_filename; }
	size_t get_line() const { return m_line; }

	bool has_return_type() const { return (m_return_type != nullptr); }
	bool has_return_value() const { return (bool)m_return_value; }
	bool has_exception() const { return (bool)m_exception; }
	bool has_callback() const { return (bool)m_callback; }
	bool has_output() const { return (bool)m_output; }

	void set_return_type(const std::type_info& type) { m_return_type = &type; }
	void set_return_value(const std::shared_ptr<mock_value_wrapper>& value) { m_return_value = value; }
	void set_exception(const std::shared_ptr<mock_value_wrapper>& exception) { m_exception = exception; }
	void set_callback(std::function<void()> callback) { m_callback = callback; }
	void set_output(const std::shared_ptr<mock_value_wrapper>& output) { m_output = output; }

	const std::type_info& get_return_type() const;
	std::shared_ptr<mock_value_wrapper> get_return_value() const { return m_return_value; }
	std::shared_ptr<mock_value_wrapper> get_exception() const { return m_exception; }
	std::function<void()> get_callback() const { return m_callback; }
	std::shared_ptr<mock_value_wrapper> get_output() const { return m_output; }

	void call_callback() const { m_callback(); }

	std::string to_string() const;

	bool match(const MockFunctionCall&) const;

private:
	const char* m_function_name;
	const char* m_call_string;
	const char* m_filename;
	size_t m_line;

	std::vector<std::shared_ptr<mock_value_wrapper>> m_parameters;
	const std::type_info* m_return_type;
	std::shared_ptr<mock_value_wrapper> m_return_value;
	std::shared_ptr<mock_value_wrapper> m_exception;
	std::function<void()> m_callback;
	std::shared_ptr<mock_value_wrapper> m_output;
};


static MockState g_mock_state = MOCK_STATE_IDLE;
static const char* g_expect_call_str = nullptr;
static const char* g_expect_filename = nullptr;
static size_t g_expect_line = 0;
static std::queue<MockFunctionCall> g_expected_calls;


MockFunctionCall::MockFunctionCall(const char* function_name, const std::vector<std::shared_ptr<mock_value_wrapper>>& params, const char* call_str, const char* filename, size_t line)
	: m_function_name(function_name)
	, m_call_string(call_str)
	, m_filename(filename)
	, m_line(line)
	, m_parameters(params)
	, m_return_type(nullptr)
{
}

MockFunctionCall::MockFunctionCall(const char* function_name, const std::vector<std::shared_ptr<mock_value_wrapper>>& params)
	: MockFunctionCall(function_name, params, "", "", 0)
{
}

const std::type_info& MockFunctionCall::get_return_type() const
{
	if (m_return_type == nullptr)
		throw std::runtime_error("MockFunctionCall asking for return type when none");
	return *m_return_type;
}

std::string MockFunctionCall::to_string() const
{
	std::ostringstream out;
	out << m_function_name << "(";
	for (size_t i = 0; i < m_parameters.size(); i++)
	{
		if (i != 0)
			out << ", ";
		m_parameters[i]->write(out);
	}
	out << ")";
	return out.str();
}

bool MockFunctionCall::match(const MockFunctionCall& second) const
{
	if (std::strcmp(m_function_name, second.m_function_name) != 0)
		return false;
	if (m_parameters.size() != second.m_parameters.size())
		return false;
	for (size_t i = 0; i < m_parameters.size(); i++)
		if (!m_parameters[i]->equals(second.m_parameters[i]))
			return false;
	return true;
}

MockData::MockData(const uint8_t* ptr, size_t size)
	: m_pointer(nullptr)
	, m_data(ptr, ptr + size)
{
}

MockData::MockData(uint8_t* ptr, size_t size)
	: m_pointer(ptr)
	, m_data(ptr, ptr + size)
{
}

MockData::MockData(const char* ptr, size_t size)
	: MockData((const uint8_t*)ptr, size)
{
}

MockData::MockData(char* ptr, size_t size)
	: MockData((uint8_t*)ptr, size)
{
}

MockData& MockData::operator=(const MockData& second)
{
	if (m_data.size() < second.m_data.size())
	{
		FAIL("MockData setting %zd byte buffer with %zd bytes of data.", m_data.size(), second.m_data.size());
		throw std::runtime_error("Mock Data buffer overflow");
	}
	if (m_pointer == nullptr)
	{
		FAIL("Setting data in constant MockData.");
		throw std::runtime_error("Setting data in constant MockData");
	}
	m_data = second.m_data;
	for (size_t i = 0; i < m_data.size(); i++)
		m_pointer[i] = m_data[i];

	return *this;
}

bool MockData::operator==(const MockData& second) const
{
	return (m_data == second.m_data);
}

extern std::ostream& operator<<(std::ostream& out, const MockData& data)
{
	out << "0x";
	for (uint8_t x : data.get())
		out << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)x;
	out << " '";
	for (uint8_t x : data.get())
		out.put(std::isprint(x) ? x : '?');
	out << "'";
	return out;
}

static void mock_set_state(MockState new_state)
{
	g_mock_state = new_state;
}

extern void mock_reset()
{
	mock_set_state(MOCK_STATE_IDLE);
	while (!g_expected_calls.empty())
		g_expected_calls.pop();
}

extern void mock_verify()
{
	if (g_mock_state == MOCK_STATE_RECORD_DONE)
		mock_set_state(MOCK_STATE_IDLE);
	if (g_mock_state != MOCK_STATE_IDLE)
	{
		FAIL("Mock internal error: state error (mock_verify %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	if (!g_expected_calls.empty())
	{
		auto& expected = g_expected_calls.front();
		FAIL("Mock missing %zd expected calls.  Next: '%s' %s:%zd", g_expected_calls.size(), expected.get_call_string(), expected.get_filename(), expected.get_line());
		throw std::runtime_error("Mock missing expected call.");
	}
}

extern void mock_begin_expect(const char* call_str, const char* file_name, size_t line)
{
	if (g_mock_state == MOCK_STATE_RECORD_DONE)
		mock_set_state(MOCK_STATE_IDLE);
	if (g_mock_state == MOCK_STATE_RECORD_DONE_WAITING_RETURN)
	{
		FAIL("Mock expected call '%s' missing _AND_RETURN or _AND_THROW %s:%zd", g_expect_call_str, g_expect_filename, g_expect_line);
		throw std::runtime_error("Mock expected call missing _AND_RETURN or _AND_THROW.");
	}
	if (g_mock_state != MOCK_STATE_IDLE)
	{
		FAIL("Mock internal error: state error (mock_begin_expect %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	mock_set_state(MOCK_STATE_RECORD_BEGIN);
	g_expect_call_str = call_str;
	g_expect_filename = file_name;
	g_expect_line = line;
}

extern void mock_end_expect(const char* call_str)
{
	if (g_mock_state == MOCK_STATE_RECORD_BEGIN)
	{
		FAIL("Mock of a non-mocked method '%s'.", call_str);
		throw std::runtime_error("Mock of a non-mocked method.");
	}
	if (g_mock_state != MOCK_STATE_RECORD_CALLED)
	{
		FAIL("Mock internal error: state error (mock_end_expect %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	if (std::strcmp(call_str, g_expect_call_str) != 0)
	{
		FAIL("Mock internal error: mismatched expect.");
		throw std::runtime_error("Mock internal error: mismatched expect.");
	}
	if (g_expected_calls.empty())
	{
		FAIL("Mock internal error: empty call queue.");
		throw std::runtime_error("Mock internal error: empty call queue.");
	}
	auto& expected = g_expected_calls.back();
	if (expected.has_return_type())
		mock_set_state(MOCK_STATE_RECORD_DONE_WAITING_RETURN);
	else
		mock_set_state(MOCK_STATE_RECORD_DONE);
}

extern void mock_add_callback(std::function<void()> callback)
{
	if (g_mock_state != MOCK_STATE_RECORD_DONE_WAITING_RETURN && g_mock_state != MOCK_STATE_RECORD_DONE)
	{
		FAIL("Mock internal error: state error (mock_add_callback %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	if (g_expected_calls.empty())
	{
		FAIL("Mock internal error: empty call queue.");
		throw std::runtime_error("Mock internal error: empty call queue.");
	}
	auto& expected = g_expected_calls.back();
	if (expected.has_callback())
	{
		FAIL("Mock only supports one do action per method.");
		throw std::runtime_error("Mock only supports one do action per method.");
	}
	expected.set_callback(callback);
}

extern void mock_add_return(const std::shared_ptr<mock_value_wrapper>& value, const char* value_str)
{
	if (g_mock_state == MOCK_STATE_RECORD_DONE)
	{
		FAIL("Mock '%s' does not expect a return. %s:%zd", g_expect_call_str, g_expect_filename, g_expect_line);
		throw std::runtime_error("Mock has no return.");
	}
	if (g_mock_state != MOCK_STATE_RECORD_DONE_WAITING_RETURN)
	{
		FAIL("Mock internal error: state error (mock_add_return %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	if (g_expected_calls.empty())
	{
		FAIL("Mock internal error: empty call queue.");
		throw std::runtime_error("Mock internal error: empty call queue.");
	}
	auto& expected = g_expected_calls.back();
	if (expected.get_return_type() != value->get_type())
	{
		const char* expected_type_name = expected.get_return_type().name();
		const char* actual_type_name = value->get_type().name();
		FAIL("Mock '%s' expects return type %s, but got %s with %s. %s:%zd", g_expect_call_str, expected_type_name, actual_type_name, value_str, g_expect_filename, g_expect_line);
		throw std::runtime_error("Mock return type mismatch");
	}
	expected.set_return_value(value);
	mock_set_state(MOCK_STATE_IDLE);
}

extern void mock_add_exception(const std::shared_ptr<mock_value_wrapper>& exception)
{
	if (g_mock_state != MOCK_STATE_RECORD_DONE_WAITING_RETURN && g_mock_state != MOCK_STATE_RECORD_DONE)
	{
		FAIL("Mock internal error: state error (mock_add_exception %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	if (g_expected_calls.empty())
	{
		FAIL("Mock internal error: empty call queue.");
		throw std::runtime_error("Mock internal error: empty call queue.");
	}
	auto& expected = g_expected_calls.back();
	expected.set_exception(exception);
	mock_set_state(MOCK_STATE_IDLE);
}

extern void mock_call(const std::vector<std::shared_ptr<mock_value_wrapper>>& params, const char* function_name_str)
{
	if (g_mock_state == MOCK_STATE_RECORD_DONE)
		mock_set_state(MOCK_STATE_IDLE);
	if (g_mock_state == MOCK_STATE_RECORD_BEGIN)
	{
		g_expected_calls.emplace(function_name_str, params, g_expect_call_str, g_expect_filename, g_expect_line);
		mock_set_state(MOCK_STATE_RECORD_CALLED);
		return;
	}
	MockFunctionCall call(function_name_str, params);
	if (g_mock_state == MOCK_STATE_RECORD_CALLED)
	{
		FAIL("Mock '%s' calls multiple mocked methods. %s:%zd", g_expect_call_str, g_expect_filename, g_expect_line);
		throw std::runtime_error("Mock calls multiple mocked methods.");
	}
	if (g_mock_state != MOCK_STATE_IDLE)
	{
		FAIL("Mock internal error: state error (mock_call %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	if (g_expected_calls.empty())
	{
		FAIL("Mock unexpected call %s.", call.to_string().c_str());
		throw std::runtime_error("Mock unexpected call.");
	}
	auto& expected = g_expected_calls.front();
	if (!expected.match(call))
	{
		FAIL("Mock expected %s actual %s.", expected.to_string().c_str(), call.to_string().c_str());
		throw std::runtime_error("Mock mismatched call.");
	}
	if (expected.has_exception())
	{
		auto exception = expected.get_exception();
		g_expected_calls.pop();
		exception->throw_exception();
		FAIL("Mock throw failed.");
		throw std::runtime_error("Mock throw failed.");
	}
	if (expected.has_output())
	{
		mock_set_state(MOCK_STATE_PLAY_WAITING_OUTPUT);
	}
	else if (expected.has_return_value())
	{
		mock_set_state(MOCK_STATE_PLAY_WAITING_RETURN);
	}
	else
	{
		auto callback = expected.get_callback();
		g_expected_calls.pop();
		mock_set_state(MOCK_STATE_IDLE);
		if (callback)
			callback();
	}
}

extern void mock_output(const std::shared_ptr<mock_value_wrapper>& output)
{
	if (g_mock_state == MOCK_STATE_RECORD_CALLED)
	{
		ASSERT(!g_expected_calls.empty());
		auto& expected = g_expected_calls.back();
		if (expected.has_output())
		{
			FAIL("Mock method only currently supports a single output.");
			throw std::runtime_error("Mock method only currently supports a single output.");
		}
		expected.set_output(output);
		mock_set_state(MOCK_STATE_RECORD_CALLED);
		return;
	}
	if (g_mock_state != MOCK_STATE_PLAY_WAITING_OUTPUT)
	{
		FAIL("Mock internal error: state error (mock_output %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	ASSERT(!g_expected_calls.empty());
	auto& expected = g_expected_calls.front();
	ASSERT(expected.has_output());
	auto saved_output = expected.get_output();
	ASSERT(saved_output->get_type() == output->get_type());
	output->set(saved_output);
	if (expected.has_return_value())
	{
		mock_set_state(MOCK_STATE_PLAY_WAITING_RETURN);
	}
	else
	{
		auto callback = expected.get_callback();
		g_expected_calls.pop();
		mock_set_state(MOCK_STATE_IDLE);
		if (callback)
			callback();
	}
}

extern void mock_return(mock_value_wrapper* result, const char* function_name_str)
{
	if (g_mock_state == MOCK_STATE_RECORD_CALLED)
	{
		ASSERT(!g_expected_calls.empty());
		auto& expected = g_expected_calls.back();
		if (expected.has_return_type())
		{
			FAIL("Mock method has two returns.");
			throw std::runtime_error("Mock method has two returns.");
		}
		expected.set_return_type(result->get_type());
		mock_set_state(MOCK_STATE_RECORD_CALLED);
		return;
	}
	if (g_mock_state != MOCK_STATE_PLAY_WAITING_RETURN)
	{
		FAIL("Mock internal error: state error (mock_return %s).", to_string(g_mock_state));
		throw std::runtime_error("Mock internal error: state error.");
	}
	ASSERT(!g_expected_calls.empty());
	auto& expected = g_expected_calls.front();
	ASSERT(expected.has_return_value());
	ASSERT(expected.get_return_type() == result->get_type());
	result->set(expected.get_return_value());
	auto callback = expected.get_callback();
	g_expected_calls.pop();
	mock_set_state(MOCK_STATE_IDLE);
	if (callback)
		callback();
}

TEST_START(MOCK_START)
{
	mock_reset();
}

TEST_FINISH(MOCK_FINISH)
{
	mock_verify();
}

TEST_TEARDOWN(MOCK_TEARDOWN)
{
	mock_reset();
}
