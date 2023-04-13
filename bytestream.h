/***
 * MIT License
 * Copyright 2023 Mountcloud mountcloud@outlook.com
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _MOUNTCLOUD_BYTESTREAM_H_
#define _MOUNTCLOUD_BYTESTREAM_H_

/***
 * https://github.com/MountCloud/bytestream
 */

namespace mc{
    //数据大小单位，就用unsigned long long吧，搞不好真有大数据呦
    typedef unsigned long long BS_DSIZE;

    class ByteStream{
        public:
            /// 构造函数
            /// @param bucketSize 每个bucket的大小，数据越大，值就越大越好，省得频繁申请内存
            ByteStream(BS_DSIZE bucketSize = 1024);
            ~ByteStream();

            //写入数据
            void write(const char* data, BS_DSIZE size);
            void write(char* data, BS_DSIZE size);
            //读取数据
            BS_DSIZE read(char* data, BS_DSIZE size);

            //获取数据大小
            BS_DSIZE size();
            
        private:
            BS_DSIZE m_bucket_size = 1024;

            //真是数据，分片的
            char** m_buckets = 0;
            //数据块的数量
            BS_DSIZE m_bucket_count = 0;

            //读取位置在第一个bucket的位置
            //bucket[0]=[0101010101010101]  //size 16
            //[pos=5]         ↑   
            //读取完一个完整bucket，则会释放该bucket
            BS_DSIZE m_read_pos_in_first_bucket = 0;
            //写入位置在最后一个bucket的位置
            //bucket[n]=[0101100000000000]  //size 16,data size 5
            //[pos=5]         ↑   
            //写入完一个完整bucket，则会创建一个新的bucket继续写入
            BS_DSIZE m_write_pos_in_last_bucket = 0;


            //初始化一个bucket
            void _init_once_bucket();

            //写入-重新分配bucket
            void _rebucketInfoByWrite(BS_DSIZE write_size);
            //读取-重新分配bucket
            void _rebucketInfoByRead(BS_DSIZE read_size);

    };
}


#endif