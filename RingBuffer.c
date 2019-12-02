#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "RingBuffer.h"

static u8* pBufHead = NULL;//环形存储区的首地址
static u8* pBufTail = NULL;//环形存储区的结尾地址
static u8* pDataHead = NULL;//环形存储区的首地址
static u8* pDataTail = NULL;//环形存储区的结尾地址
static u8 validLen = 0;
static u8 freeLen = 0;

/*
 * 初始化环形缓冲区
 */
void initRingbuffer(u32 buf_size)
{
    if(pBufHead == NULL)
    {
        pBufHead = (u8*) malloc(buf_size);
    }
    pDataHead = pBufHead;
    pDataTail = pBufHead;
    pBufTail = pBufHead + buf_size;
    freeLen = pBufTail - pBufHead;
    validLen = 0;
}

/*
 *  * function:向缓冲区中写入数据
 *   * param:@buffer 写入的数据指针
 *    *       @addLen 写入的数据长度
 *     * return:-1:写入长度过大
 *      *        -2:缓冲区没有初始化
 *       * */
int wirteRingbuffer(u8* buffer,u32 addLen)
{
    if(pBufHead == NULL) {
	    return -1;
    }
    /*full*/
    if(addLen > freeLen )
    {
	    return -1;
    }
    assert(buffer);

    //将要存入的数据copy到pValidTail处
    if(pDataTail + addLen >= pBufTail)//需要分成两段copy
    {
	    int len1 = pBufTail - pDataTail;
	    int len2 = addLen - len1;
	    memcpy( pDataTail, buffer, len1);
	    memcpy( pBufHead, buffer + len1, len2);
            printf("addr = %lx,c1 = %s,%s\n",pDataTail,pDataTail,pBufHead);
	    pDataTail = pBufHead + len2;//新的有效数据区结尾指针
    }else
    {
	    memcpy( pDataTail, buffer, addLen);
            printf("addr = %lx,c2 = %s\n",pDataTail,pDataTail);
	    pDataTail += addLen;//新的有效数据区结尾指针
    }

    validLen += addLen;
    freeLen -= addLen;
    return 0;
}

/*
 * function:从缓冲区内取出数据
 * param   :@buffer:接受读取数据的buffer
 *          @len:将要读取的数据的长度
 * return  :-1:没有初始化
 *          >0:实际读取的长度
 * */
int readRingbuffer(u8* buffer,u32 len)
{
	if(pBufHead == NULL){ 
		return -1;
	}
	assert(buffer);
        if(validLen == 0)
		return -1;
        printf("freelen =%d,validlen = %d\n",freeLen,validLen);

	if( len > validLen){ 
		len = validLen;
	}
        printf("pDataHead = %lx, pDataTail = %lx,pBufTail = %lx,pBufHead = %lx\n",pDataHead,pDataTail,pBufTail,pBufHead);

	if(pDataHead + len > pBufTail)//需要分成两段copy
	{
		int len1 = pBufTail - pDataHead;
		int len2 = len - len1;
		memcpy( buffer, pDataHead, len1);//第一段
		memset(pDataHead,0,len1);
		memcpy( buffer+len1, pBufHead, len2);//第二段，绕到整个存储区的开头
		memset(pBufHead,0,len2);
		pDataHead = pBufHead + len2;//更新已使用缓冲区的起始
	}else
	{
		memcpy( buffer, pDataHead, len);
		memset(pDataHead,0,len);
		pDataHead = pDataHead + len;//更新已使用缓冲区的起始
	}
        freeLen += len;
	validLen -= len;
        printf("pDataHead = %lx, pDataTail = %lx,pBufTail = %lx,pBufHead = %lx\n",pDataHead,pDataTail,pBufTail,pBufHead);
        printf("freelen =%d,validlen = %d\n",freeLen,validLen);

	return len;
}

/*
 * function:获取已使用缓冲区的长度
 * return  :已使用的buffer长度
 * */
u32 getRingbufferValidLen(void)
{
	u32 validLen;

        if(pDataTail >= pDataHead)
	{
		validLen = pDataHead - pDataTail;
	}else{
		validLen = pBufTail - pDataHead + pDataTail - pBufHead;
	}
	return validLen;
}

/*
 * function:释放环形缓冲区
 * */
void releaseRingbuffer(void)
{
	if(pBufHead != NULL){ 
		free(pBufHead);
	}
	pBufHead = NULL;
}
// 主函数
int main()
{
	char c;
	char buf[10] = {'a','b','c','d','e','f','g','h'};
	int i;
	int loop;
	int readLen;
	u8 readBuffer[10];
//	setvbuf(stdout,NULL,_IONBF,0); //pinrtf、putchar不能立马输出，打开此注释
	initRingbuffer(50);

	printf("Please enter a line [blank line to terminate]> ");
#if 0
	do{
		c = getchar();
		putchar(c);
		switch(c)
		{
			case 'Q':
				goto exit;
				break;
			case 'R':
				readLen = readRingbuffer(readBuffer,10);
				printf("readRingbuffer len:%d\n",readLen);
				if(readLen > 0){
					printf("readRingbuffer:");
					for(i = 0; i < readLen;i++){
						printf("%c ",(char)readBuffer[i]);
					}
					printf("\n");
				}
				break;
			default :
				if( c != '\n') 
					wirteRingbuffer((u8*)&c,1);
				break;
		}
	}while (1);
#else

	wirteRingbuffer((u8*)&buf[0],4);
	readLen = readRingbuffer(readBuffer,10);
	printf("readRingbuffer len:%d\n",readLen);
	if(readLen > 0){
		printf("readRingbuffer:");
		for(i = 0; i < readLen;i++){
			printf("%c ",(char)readBuffer[i]);
		}
		printf("\n");
	}

	printf("******************\n");

for(loop = 0 ; loop < 1000; loop++ )
{
	wirteRingbuffer((u8*)&buf[3],3);
	readLen = readRingbuffer(readBuffer,10);
	printf("readRingbuffer len:%d\n",readLen);
	if(readLen > 0){
		printf("readRingbuffer:");
		for(i = 0; i < readLen;i++){
			printf("%c ",(char)readBuffer[i]);
		}
		printf("\n");
	}
}

#endif


exit:
	releaseRingbuffer();
	printf("exit.\n");
	return 0;
}
