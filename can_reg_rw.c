/*
    提供配置AXI_CAN 寄存器的接口
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "can_reg.h"


/*function: DevWrite
    description: 向寄存器中写入值
    parameter:
        dev_fd: 设备实例
        addr： 写目标地址
        buffer： 写入数据的缓存地址
        size: 写入数据大小 
    return：
        int 0=success， -1=failed
*/
int DevWrite(int dev_fd, uint64_t addr, void *buffer, uint64_t size){
    if( addr != lseek(dev_fd,addr,SEEK_SET))
        return -1;
    if (size != write(dev_fd,buffer,size))
        return -1;
    return 0;
}


// function：DevRead
// description：读取寄存器中的值
// paramater：
//      dev_fd: 设备
//      addr： 读取数据地址
//      buffer： 读取后数据的指针
//      size： 读取数据的大小
//  return：
//      int 0=success， -1=failed
int DevRead(int dev_fd, uint64_t addr, void *buffer, uint64_t size){
    if( addr != lseek(dev_fd,addr,SEEK_SET))
        return -1;
    if (size != read(dev_fd,buffer,size))
        return -1;
    return 0;
}

/*读取每32bit的寄存器值*/
int DeRead32Bit(int dev_fd, uint64_t addr){
    uint32_t *buffer;
    buffer = (uint32_t *)malloc(4);
    DevRead(dev_fd,addr,buffer,4);
    return *buffer;
}

/*系统复位*/
int RstStream(int dev_fd, uint64_t addr)
//paramater:
//  dev_fd: 设备
//  addr： 设备的基地址
{
    uint64_t addr_offs = SRR;
    void *buffer;
    addr = addr + addr_offs;
    int data = 0b1; 
    buffer = &data;
    DevWrite(dev_fd,addr,buffer,1);

    return 0;
}

//配置寄存器使能
int _CenBit(int dev_fd,uint64_t addr,int state)
//paramater：
//  dev_fd: 设备
//  addr： 设备的基地址
//  state: 使能状态
{
    uint64_t addr_offs = SRR;
    void *buffer;
    addr = addr+addr_offs;
    int8_t data;
    buffer = &data;
    if (state==1)
        data = 0b00000000;
    else
        data = 0b00000010;
    DevWrite(dev_fd,addr,buffer,1);
}



int SelectMode(int dev_fd,uint64_t addr,int mode)
/*选择传输模式*/
//parameter:
//  dev_fd: 设备
//  addr： 设备基地址
//  mode： 设备传输模式{"NORM","SLEEP","LBACK","CONF"}
{
    uint64_t addr_offs = MSR;
    uint64_t addr_reg;
    void *buffer;
    addr_reg = addr + addr_offs;
    uint8_t data;
    buffer = &data;

    // enum MODE {"NORM","LBACK","SLEEP","CONF"};
    // enum MODE cmd = mode;
    switch(mode){
        case 0:
            _CenBit(dev_fd,addr,0);
            data = 0b00000000;
            break;
        case 1:
            _CenBit(dev_fd,addr,0);
            data = 0b00000010;
            break;
        case 2:
            _CenBit(dev_fd,addr,0);
            data = 0b00000001;
            break;
        case 3:
            _CenBit(dev_fd,addr,1);
            return 0;
            break;
        default:
            return -1;
            break;
    }
    DevWrite(dev_fd,addr_reg,buffer,1);
    return 0;
}



int _SetBrpr(int dev_fd,uint64_t addr,uint8_t brpr)
// 设置CAN的BRPR分频比
//parameter:
//  dev_fd: 设备
//  addr: 设备基地址
//  brpr： 分频率
{
    uint64_t addr_offs,addr_reg;
    addr_offs = BRPR;
    addr_reg = addr + addr_offs;
    void *buffer;
    uint8_t data = brpr;
    buffer = &data;
    DevWrite(dev_fd,addr_reg,buffer,1);
    return 0;
}

int _SetBitTiming(int dev_fd,uint64_t addr,int sjw, int ts2,int ts1)
//设置位时间寄存器
//parameter:
//  dev_fd: 设备
//  addr：设备基地址
//  sjw: 同步补偿宽度
//  ts2: 相位缓冲段1
//  ts1: 相位缓冲段2
{
    uint64_t addr_offs,addr_reg;
    addr_offs = BTR;
    addr_reg = addr + addr_offs;
    void *buffer;
    uint16_t data;
    buffer = &data;

    if ((sjw>3)||(ts2>7)||(ts1>15))
        return -1;
    data = ts1 + ts2<<4 + sjw<<7;
    DevWrite(dev_fd,addr_reg,buffer,2);
    
    return 0;
}

/*查询位时间*/
int QueryBitTiming(){
    return 0;
}



/*设置波特率*/
int SetBaudrate(int dev_fd,uint64_t addr,int clk,int baudrate)
//function: 设置波特率  
//parameter:
//  dev_fd: 设备
//  addr： 设备基地址
//  clk：系统时钟
//  baudrate： 设备的波特率
//波特率的计算公式： tq = tosc*(BRP+1) tq：time Quantum; tosc:系统时钟周期
// 经过计算 baud = tosc/((BRP+1)*(ts1+1+ts2+1+sjw+1))
{
    
    return 0;
}

/*查询波特率*/
int QueryBaudrate(){
    return 0;
}




/*查询错误计数*/
uint8_t QueryErrorCount(int dev_fd,uint64_t addr,int trec)
//func: 查询错误状态计数
//parameter：
//  dev_fd: 设备
//  addr: 设备基地址
//  trec： 错误类型 REC|TEC
//Return: 返回错误计数
{
    uint64_t addr_offs,addr_reg;
    addr_offs = ECR;
    addr_reg = addr+addr_offs;

    if(trec ==1)
        addr_reg = addr_reg + 1;
        
    uint8_t *buffer;
    buffer = (uint8_t *)malloc(1);
    
    DevRead(dev_fd,addr_reg,buffer,1);
    return *buffer;
}


/*设置错误状态*/
int SetErrorState(){
    return 0;
}

/*查询错误状态*/
int QueryErrorState(){
    return 0;
}


/*查询传输模式*/
int QueryMode(int dev_fd,uint64_t addr)
// Function:查询传输模式
// Parameter:
//      dev_fd: 设备
//      addr： 设备的基地址
// Return： 0——CONF模式；1——LBACK模式；2——SLEEP模式；3——NORM模式；
{
    uint64_t addr_offs,addr_reg;
    addr_offs = SR;
    addr_reg = addr + addr_offs;
    
    int32_t reg = DeRead32Bit(dev_fd,addr_reg);
    switch(reg&0xf){
        case 1: return 0; //配置模式
        case 2: return 1; //LBACK模式
        case 4: return 2; //SLEEP模式
        case 8: return 3; //NORM模式
        default: return -1;
    }
}

/*查询Tx发送缓存是否存满*/
int QueryTxFull(int dev_fd,uint64_t addr,int tx)
//Function: 查询Tx缓存是否存满
//Parameter：
//      dev_fd: 设备
//      addr: 设备的基地址
//      tx： FIFO|HPB
//Return: 0——为存满； 1——存满
{
    uint64_t addr_offs,addr_reg;
    addr_offs = SR;
    addr_reg = addr + addr_offs;
    int32_t reg = DeRead32Bit(dev_fd,addr_reg);
    int state;
    if(tx==1)
        state = (reg&0x20)>>5;
    else
        state = (reg&0x10)>>4;
    return state;
}




/*查询中断状态*/
int QueryInterruptState(){
    return 0;
}

/*设置中断使能*/
int SetInterruptEnable(){
    return 0;
}

/*中断清除*/
int SetInterruptClear(){
    return 0;
}


/*写FIFO IDR*/
int SetTxFifoId(int dev_fd,uint64_t addr,uint32_t id,int extend,int remote)
// 设置发送FIFO的ID段寄存器
//parameter:
//  dev_fd: 设备
//  addr： 基地址
//  id: 发送id
//  extend：是否是扩展帧
//  remote：是否是远程帧
{
    uint64_t addr_offs,addr_reg;
    addr_offs = TX_FIFO_ID;
    addr_reg = addr + addr_offs;

    void *buffer;
    uint32_t data;
    buffer = &data;

    int ide,bit11,bit31;
    
    if(!extend&&!remote){//标准数据帧
        bit11 = 0;
        ide = 0;
        bit31 = 0;
    }
    else if(!extend&&remote){//标准遥控帧
        bit11 = 1;
        ide = 0;
        bit31 = 0;
    }
    else if(extend&&!remote){//扩展数据帧
        bit11 = 0;
        ide = 1;
        bit31 = 0;
    }
    else{                 //扩展遥控帧
        bit11 = 1;
        ide = 1;
        bit31 = 1;
    }
    
    uint32_t id_0_10,id_13_30;

    if(!extend){        //标准帧的id配置
        id_0_10 = id;
        id_13_30 = 0;
    }
    else{               //扩展帧的id配置
        id_0_10 = id>>18;
        id_13_30 = id&0xff;
    }
    //将该32bit的寄存器整体配置
    data = id_0_10<<21 + bit11<<20 +ide<<19 + id_13_30<<1 + bit31;

    DevWrite(dev_fd,addr_reg,buffer,4);

    return 0;
}

/*写FIFO DLC*/
int SetTxFifoDlc(int dev_fd,uint64_t addr,int dlc)
//function: 写入TX Message长度
// parameter:
//  dev_fd: 设备
//  addr: 设备基地址
//  dlc: 数据长度
{
    uint64_t addr_offs,addr_reg;
    addr_offs = TX_FIFO_DLC;
    addr_reg = addr+addr_offs;
    void *buffer;
    uint32_t data;
    buffer = &data;

    if(dlc>15)
        return -1;
    
    data = dlc<<28;

    DevWrite(dev_fd,addr_reg,buffer,4);

    return 0;
}

 
/*写FIFO的第一个双字*/
int SetTxFifoDword1(int dev_fd,uint64_t addr,uint32_t data)
//function: 写入第一个双字数据
//parameter：
//  dev_fd: 设备
//  addr：设备基地址
//  data: 写入的数据
{
    uint64_t addr_offs,addr_reg;
    addr_offs = TX_FIFO_DW1;
    addr_reg = addr + addr_offs;
    void *buffer;
    DevWrite(dev_fd,addr_reg,buffer,4);
    return 0;
}

/*写FIFO的第二个双字*/
int SetTxFifoDword2(int dev_fd,uint64_t addr,uint32_t data)
//function: 写入第二个双字数据
//parameter：
//  dev_fd: 设备
//  addr：设备基地址
//  data: 写入的数据
{
    uint64_t addr_offs,addr_reg;
    addr_offs = TX_FIFO_DW2;
    addr_reg = addr + addr_offs;
    void *buffer;
    DevWrite(dev_fd,addr_reg,buffer,4);
    return 0;
}

//


/*写HPB的ID*/
int SetTxHpbId(){
    return 0;
}

/*写Hpb的DLC*/
int SetTxHpbDlc(){
    return 0;
}

/*写HPB的第一个双字*/
int SetTxHpbDword1(){
    return 0;
}

/*写HPB的第二个双字*/
int SetTxHpbDword2(){
    return 0;
}

/*发送消息*/
int SetTxMessage(){
    return 0;
}

/*读Rx FIFO消息ID*/
int QueryRxFifoId()
{
    return 0;
}

/*读Rx Fifo消息的DLC*/
int QueryRxFifoDlc(){
    return 0;
}

/*读Rx Fifo消息的第一个双字*/
int QueryRxFifoDword1(){
    return 0;
}

/*读Rx FIFO消息的第二个双字节*/
int QueryRxFifoDword2(){
    return 0;
}

/*接收Rx的Message*/
int QueryRxMessage(){
    return 0;
}

/*设置过滤器的使能状态*/
int SetAcceptFilterBitState(int dev_fd,uint64_t addr, int fstate)
//func: 设置过滤使能
//parameter:
//  dev_fd: 设备
//  addr： 设备基地址
//  fstate： 过滤器的使能状态
{   
    uint64_t addr_offs,addr_reg;
    addr_offs = AFR;
    addr_reg = addr + addr_offs;
    void *buffer;
    uint8_t data;
    buffer = &data;
    data = fstate;

    if(fstate>15)
        return -1;
    
    DevWrite(dev_fd,addr_reg,buffer,1);

    return 0;
}

/*查询过滤器的使能状态*/
int QueryAcceptFilterBitState(){
    return 0;
}

/*设置过滤器的mask*/
int SetAcceptFilterMask(int dev_fd,uint64_t addr, int fnum)
{
    uint64_t addr_offs,addr_reg;
    switch (fnum)
    {
    case 1:
        addr_offs = AFIR1;
        break;
    case 2:
        addr_offs = AFIR2;
        break;
    case 3:
        addr_offs = AFIR3;
        break;
    case 4:
        addr_offs = AFIR4;
    default:
        return -1;
    }
    addr_reg = addr + addr_offs;



    return 0;
}

/*查询过滤器的mask*/
int QueryAcceptFi了terMask(){
    return 0;
}

/*设置过滤ID*/
int SetAcceptFilterId(){
    return 0;
}

/*查询过滤ID*/
int QueryAcceptFilterId(){
    return 0;
}

int main(void){
    char dev_write[] = "/dev/xdma0_h2c_0";
    char dev_read[] = "/dev/xdma0_c2h_0";


    int fd_write = open(dev_write,O_RDWR);
    SelectMode(fd_write,0x0,3);
    close(fd_write);
    int fd_read = open(dev_read,O_RDWR);
    int mode = QueryMode(fd_read,0);
    printf("mode = %d\n",mode);
    return 0;
}