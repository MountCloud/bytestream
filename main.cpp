
#include "bytestream.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

int main(){

    //每个bucket的大小8字节，测试bucket分片工作是否正常所以设置的数值小，正常情况下bucket的大小应该根据业务场景设置
    //也可以不设置bucket大小，默认1024

    //The size of each bucket is 8 bytes. Test whether the bucket sharding is working properly, so the set value is small. Normally, the size of the bucket should be set according to the business scenario
    //You can also choose not to set the bucket size, which defaults to 1024
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

    return 0;
}