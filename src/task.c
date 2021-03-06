// 任务列表

#include "lora.h"

#define LISTSIZE 30
int8_t head = 0;
int8_t tail = 0;
int8_t listLen = 0;

TaskType TaskList[LISTSIZE];

void InitTask()
{
    head = 0;
    tail = 0;
    listLen = 0;
}

void PushTask(TaskType t)
{
    if (listLen >= LISTSIZE)
    {
        //Reset;
        return;
    }

    TaskList[tail] = t;
    tail++;
    listLen++;

    if (tail >= LISTSIZE && listLen <= LISTSIZE)
    {
        tail = 0;
    }
}

TaskType popTask()
{
    if (0 >= listLen)
    {
        return TaskEmpty;
    }
    TaskType t = TaskList[head];
    head++;
    if (head >= LISTSIZE)
    {
        head = 0;
    }
    listLen--;
    return t;
}
bool IsEmptyTaskList()
{
    if (0 >= listLen)
    {
        return TRUE;
    }
    return FALSE;
}

void HandleTask()
{
    TaskType task;
    //char str[50] = "second=0000,tick=00\r\n";
    while (1)
    {
        while (IsEmptyTaskList()) //队列不为空
            ;
        task = popTask();
        //Debug("popTask:%u", task);
        switch (task)
        {
        case TICK:
            // if (TickNum == 1 && Second % 10 == 0)
            // {
            //     str[7] = '0' + Second / 1000 % 10;
            //     str[8] = '0' + Second / 100 % 10;
            //     str[9] = '0' + Second / 10 % 10;
            //     str[10] = '0' + Second % 10;
            //     str[17] = '0' + TickNum / 10;
            //     str[18] = '0' + TickNum % 10;
            //     SendDevice((uint8_t *)str, (uint8_t)strlen(str));
            // }
            ResetKeyHandler();                      //处理复位按键
            if (Second % 300 == Conf.HeartBeatTime) //5分钟内的某一秒发送
            {
                SendHeartBeat();
            }
            for (uint8_t i = 0; i < CMD_NUMBER; i++)
            {
                if (Conf.cmdLen[i] != 0)
                {
                    if (Second % Conf.cmdPeriod[i] == 0)
                    {
                        SendCmdToDevice(i);
                    }
                }
            }
            break;

        case LORA_RECV_DATA:
            switch (LoraStatus)
            {
            case LORA_TRANSFER:
            case LORA_REGISTER:
            case GET_LORA_DEV_PARAM:
                HandleLoraData();
                break;
            case ENTER_LORA_AT_CMD:
                HandLoraATModel();
            }

            break;
        case DEV_RECV_DATA:
            switch (DevStatus)
            {
            case DEV_DATA_TRANSFER:
            case GW_REGISTER:
                HandleDevData();
                break;
            case ENTER_GPRS_AT_CMD:
                HandGPRSATModel();
                break;
            }
            //PushTask(LORA_DATA_SEND);
            break;
        case DEV_SEND_COMPLETE:
            while (USART_GetFlagStatus(DevCom, USART_FLAG_TC) != SET) //DMA 完成不等于串口发送完成，要等待串口发送完成，不然丢数据
                ;
            SetRS485CTL(RESET);
            break;
        case LORA_SEND_COMPLETE:
            while (USART_GetFlagStatus(LoraCom, USART_FLAG_TC) != SET)
                ; //DMA 完成不等于串口发送完成，要等待串口发送完成，不然丢数据
            break;
        case LORA_DATA_SEND:
            //SetWakeState(SET);
            //HandSendLoarData();
            break;
        case START_DELAY_TASK:
            StartDelayTask();
            break;
        case ATCMD_RESTARTEND:
            ATCMD_ResartEnd();
            break;
        case ENTER_ATMODLE_TIMEOUT:
            EnterAtModelTimeout();
            break;
        case ENTER_GPRS_AT:
            EnterGPRS_AT();
            break;
        case ENTER_GPRS_AT_TIMEOUT:
            Debug("ENTER_GPRS_AT_TIMEOUT");
            if (EnterGPRS_AtTryTimes > 3 && !isGPRS_GW) //尝试3次仍然没有GPRS模块相应则认为是Lora节点
            {
                AbortGPRSAtCmd();
                PushTask(GW_Register_Task);
                Debug("PushTask(GW_Register_Task)");
            }
            else
            {
                EnterGPRS_AT_TIMEOUT();
            }
            EnterGPRS_AtTryTimes++;
            break;
        case LoraRegister_Task:
            LoraRegister();
            break;
        case GetLoraDevParam_Task:
            GetLoraDevConf();
            break;
        case CONF_LORA_PARM:
            InitLoraConf();
            break;
        case HeartBeat_Task:
            SendHeartBeat();
            break;
        case SyncTime_Task:
            SendSyncTime();
            break;
        case GW_Register_Task:
            GwRegister();
            break;
        }
    }
}