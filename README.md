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


