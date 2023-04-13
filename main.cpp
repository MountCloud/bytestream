
#include "bytestream.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

int main(){

    //每个bucket的大小8字节，测试bucket分片工作是否正常所以设置的数值小，正常情况下bucket的大小应该根据业务场景设置
    mc::ByteStream* bs = new mc::ByteStream(8);
    
    //write
    std::string testText = "12345678";
    bs->write(testText.c_str(), testText.size());

    //read
    char* data = (char*)calloc(1024, sizeof(char));
    mc::BS_DSIZE readsize = bs->read(data, 1024);

    std::string readText = data;
    std::cout << readText << std::endl;

    delete data;

    //delete
    delete bs;

    return 0;
}