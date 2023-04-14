/***
 * MIT License
 * Copyright 2023 Mountcloud mountcloud@outlook.com
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "bytestream.h"

#include <cstdlib>
#include <cstring>

mc::ByteStream::ByteStream(mc::BS_DSIZE bucketSize){
    this->m_bucket_size = bucketSize;
    //init once bucket
    _init_once_bucket();
}
mc::ByteStream::~ByteStream(){
    //delete all bucket
    for(int i=0;i<m_bucket_count;i++){
        delete m_buckets[i];
    }
    //delete m_buckets
    delete m_buckets;
}

void mc::ByteStream::_init_once_bucket(){
    char* bucket = (char*)calloc(m_bucket_size, sizeof(char));
    m_buckets = (char**)calloc(1, sizeof(char*));
    m_buckets[0] = bucket;
    m_bucket_count = 1;
}

void mc::ByteStream::_rebucketInfoByWrite(mc::BS_DSIZE write_size){
    //判断是否需要新增bucket
    mc::BS_DSIZE last_bucket_volume = m_bucket_size - m_write_pos_in_last_bucket;
    if(last_bucket_volume >= write_size){
        //不需要新增bucket
        m_write_pos_in_last_bucket = m_write_pos_in_last_bucket + write_size;
        return;
    }

    //需要新增bucket
    //是否需要再加一个，第一种：余数不是0，第二种：写入的数据加上当前的位置，刚好沾满了最后一个bucket时需要再加一个bucket，重新设置pos时会设置为0
    bool need_add_one = (write_size - last_bucket_volume) % m_bucket_size > 0 || (write_size + m_write_pos_in_last_bucket) % m_bucket_count == 0;
    mc::BS_DSIZE need_new_bucket_count = (write_size - last_bucket_volume) / m_bucket_size + (need_add_one ? 1 : 0);

    mc::BS_DSIZE new_bucket_count = m_bucket_count + need_new_bucket_count;

    char** new_bucket = (char**)calloc(new_bucket_count, sizeof(char*));
    memcpy((char*)new_bucket, (char*)m_buckets, m_bucket_count * sizeof(char*));

    delete m_buckets;
    m_buckets = new_bucket;

    //申请新bucket
    for(int i=0;i<need_new_bucket_count;i++){
        char* new_bucket = (char*)calloc(m_bucket_size, sizeof(char));
        m_buckets[m_bucket_count] = new_bucket;
        m_bucket_count++;
    }
    
    //重新设置pos
    mc::BS_DSIZE new_write_pos = (write_size - (m_bucket_size - m_write_pos_in_last_bucket)) % m_bucket_size;
    m_write_pos_in_last_bucket = new_write_pos;
}

void mc::ByteStream::_rebucketInfoByRead(mc::BS_DSIZE read_size){
    //需要删除bucket
    mc::BS_DSIZE remove_bucket_count = (read_size + m_read_pos_in_first_bucket) / m_bucket_size;

    for(int i=0;i<remove_bucket_count;i++){
        delete m_buckets[i];
    }

    if(remove_bucket_count>0 && remove_bucket_count<m_bucket_count){
        //重新重构新bucket
        char** new_bucket = (char**)calloc(m_bucket_count - remove_bucket_count, sizeof(char*));
        memcpy(new_bucket, ((char*)m_buckets) + (remove_bucket_count*sizeof(char*)), (m_bucket_count - remove_bucket_count) * sizeof(char*));

        delete m_buckets;
        m_buckets = new_bucket;

        m_bucket_count = m_bucket_count - remove_bucket_count;
        
        mc::BS_DSIZE new_read_pos = (read_size + m_read_pos_in_first_bucket) % m_bucket_size;
        m_read_pos_in_first_bucket = new_read_pos;

    }else if(remove_bucket_count == m_bucket_count){
        //删除干净了
        delete m_buckets;
        _init_once_bucket();
    }else{
        mc::BS_DSIZE new_read_pos = (read_size + m_read_pos_in_first_bucket) % m_bucket_size;
        m_read_pos_in_first_bucket = new_read_pos;
    }

    
    //如果就剩下了1个bucket，并且读取的数据刚好把这个bucket读完了，那么就把这个bucket初始化，pos也初始化
    if(m_bucket_count == 1&& m_read_pos_in_first_bucket == m_write_pos_in_last_bucket && m_write_pos_in_last_bucket > 0){
        memset(m_buckets[0], 0, m_write_pos_in_last_bucket);
        m_read_pos_in_first_bucket = 0;
        m_write_pos_in_last_bucket = 0;
    }
}

//写入数据
void mc::ByteStream::write(const char* data, mc::BS_DSIZE size){
    char* tempdata = const_cast<char*>(data);
    this->write(tempdata, size);
}

void mc::ByteStream::write(char* data, mc::BS_DSIZE size){
    if(size == 0){
        return;
    }
    
    //计算写入起始写入bucket的位置
    mc::BS_DSIZE write_bucket_index = m_bucket_count - 1;
    //计算写入数据在bucket中的位置
    mc::BS_DSIZE write_pos_in_bucket = m_write_pos_in_last_bucket;

    //重新计算并分配bucket信息，write时rebucket需要放前面，因为write时需要计算是否需要新增bucket
    this->_rebucketInfoByWrite(size);

    //计算需要写入的bucket数量
    mc::BS_DSIZE size_and_pos = size + write_pos_in_bucket;
    mc::BS_DSIZE write_bucket_count = size_and_pos / m_bucket_size + (size_and_pos % m_bucket_size > 0 ? 1 : 0);
    //写入数据
    for(int i=0;i<write_bucket_count;i++){
        if(i==0){
            //写入第一个bucket，此时有两种情况，第一种是写入的数据小于bucket的大小，第二种是写入的数据大于bucket的大小
            //第一种情况
            if(write_bucket_count == 1){
                mc::BS_DSIZE write_size = size;
                memcpy(m_buckets[write_bucket_index+i] + write_pos_in_bucket, data, write_size);
            }else{
                //第二种情况就是横跨bucket，第一个bucket是要写满的
                mc::BS_DSIZE write_size = m_bucket_size - write_pos_in_bucket;
                memcpy(m_buckets[write_bucket_index+i] + write_pos_in_bucket, data, write_size);
            }
        }else if(i==write_bucket_count-1){
            //写入最后一个bucket
            //第一个已经写入的大小
            mc::BS_DSIZE first_write_size = m_bucket_size - write_pos_in_bucket;
            mc::BS_DSIZE skip_mid_bucket_size =  (write_bucket_count - 2) * m_bucket_size;
            mc::BS_DSIZE write_size = size - first_write_size - skip_mid_bucket_size;
            memcpy(m_buckets[write_bucket_index+i], data + (first_write_size + skip_mid_bucket_size), write_size);
        }else{
            //写入中间bucket
            mc::BS_DSIZE first_write_size = m_bucket_size - write_pos_in_bucket;
            mc::BS_DSIZE skip_mid_bucket_size =  (i - 1) * m_bucket_size;
            mc::BS_DSIZE write_size = m_bucket_size;
            memcpy(m_buckets[write_bucket_index+i], data + (first_write_size + skip_mid_bucket_size), write_size);
        }
    }
}

//读取数据
mc::BS_DSIZE mc::ByteStream::read(char* data, mc::BS_DSIZE size){

    if(size == 0){
        return size;
    }
    
    mc::BS_DSIZE readsize = size;
    mc::BS_DSIZE bssize = this->size();
    if(bssize==0){
        return 0;
    }

    if(size>bssize){
        readsize = bssize;
    }
    
    //计算需要读取的bucket数量
    mc::BS_DSIZE size_and_pos = readsize + m_read_pos_in_first_bucket;
    mc::BS_DSIZE read_bucket_count = size_and_pos / m_bucket_size + (size_and_pos % m_bucket_size > 0 ? 1 : 0);
    //读取数据
    for(int i=0;i<read_bucket_count;i++){
        if(i==0){
            //读取第一个bucket，此时有两种情况，第一种是读取的数据小于bucket的大小，第二种是读取的数据大于bucket的大小
            //第一种情况
            if(read_bucket_count == 1){
                mc::BS_DSIZE read_size = readsize;
                memcpy(data, m_buckets[i] + m_read_pos_in_first_bucket, read_size);
            }else{
                //第二种情况就是横跨bucket，第一个bucket是要读满的
                mc::BS_DSIZE read_size = m_bucket_size - m_read_pos_in_first_bucket;
                memcpy(data, m_buckets[i] + m_read_pos_in_first_bucket, read_size);
            }
        }else if(i==read_bucket_count-1){
            //读取最后一个bucket
            //第一个已经读取的大小
            mc::BS_DSIZE first_read_size = m_bucket_size - m_read_pos_in_first_bucket;
            mc::BS_DSIZE skip_mid_bucket_size =  (read_bucket_count - 2) * m_bucket_size;
            mc::BS_DSIZE read_size = readsize - first_read_size - skip_mid_bucket_size;
            memcpy(data + (first_read_size + skip_mid_bucket_size), m_buckets[i], read_size);
        }else{
            //读取中间bucket
            mc::BS_DSIZE first_read_size = m_bucket_size - m_read_pos_in_first_bucket;
            mc::BS_DSIZE skip_mid_bucket_size =  (i - 1) * m_bucket_size;
            mc::BS_DSIZE read_size = m_bucket_size;
            memcpy(data + (first_read_size + skip_mid_bucket_size), m_buckets[i], read_size);
        }
    }

    //重新计算bucket信息，read时rebucket需要放后面，因为read时需要计算是否需要删除bucket
    this->_rebucketInfoByRead(readsize);

    return readsize;
}

//获取数据大小
mc::BS_DSIZE mc::ByteStream::size(){
    mc::BS_DSIZE datasize = (this->m_bucket_count - 1) * this->m_bucket_size - this->m_read_pos_in_first_bucket + this->m_write_pos_in_last_bucket;
    return datasize;
}