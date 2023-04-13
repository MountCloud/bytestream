# bytestream

c++内存字节流。

C++ bytestream in memory.

# license

是大家都喜欢的MIT

It's a MIT that everyone likes

# 使用简单 Easy to use

main.cpp

```
#include "bytestream.h"

//每个bucket的大小8字节，测试bucket分片工作是否正常所以设置的数值小，正常情况下bucket的大小应该根据业务场景设置
mc::ByteStream* bs = new mc::ByteStream(8);

//write
std::string testText = "12345678abcdefgh!@#$%^&*";
bs->write(testText.c_str(), testText.size());

//read
char* data = (char*)calloc(1024, sizeof(char));
mc::BS_DSIZE readsize = bs->read(data, 1024);

std::string readText = data;
std::cout << readText << std::endl;

delete data;

//delete
delete bs;
```