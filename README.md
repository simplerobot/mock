# mock
A simple interface for building mocks C calls in C++.

For methods:
```
int FX(int x, int y);
void GX(int x, int y, int z);
```
Your test code defines the expected calls:
```
EXPECT(FX(1, 2))_AND_RETURN(5);
EXPECT(GX(2, 4, 6));
EXPECT(FX(3, 4))_AND_THROW(std::runtime_error("mock error"));
```
To support this, you need to overwrite the definitions of the mocked functions.  Assuming you have C++ guards in the .h file, you include it without changes.  You then define the methods as follows:
```
int FX(int x, int y)
{
    MOCK_CALL(x, y);
    MOCK_RETURN(int);
}
void GX(int x, int y, int z)
{
    MOCK_CALL(x, y, z);
}
```
If a parameter should not be checked, for instance if the y parameter to FX was a time, you could replace it to MOCK_CALL with an unchanging version.
```
int FX(int x, int y)
{
    MOCK_CALL(x, "<time>");
    MOCK_RETURN(int);
}
```
If the data pointed to by a parameter should be matched instead of the pointer itself, either convert to a std::vector or a std::string.  const char* and char[N] parameters are automatically converted from null terminated C-style strings to std::strings.
```
void HX(const uint8_t* data, size_t size)
{
	MOCK_CALL(std::vector<uint8_t>(data, data + size));
}
```
Interrupt callbacks can be added.  They will be called and expectations read during the previous call.  For example:
```
EXPECT(FX(1, 2))_AND_RETURN(5);
EXPECT_BEGIN_CALLBACK(JX(10));
EXPECT(WakeThread());
EXPECT_END_CALLBACK();
```
When the test code calls FX, execution will stop and JX will be called.  JX will be expected to call WakeThread and then return.  Once JX returns, FX will return 5 to its caller.

