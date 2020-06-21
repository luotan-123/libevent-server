#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>


int TestLinuxList()
{
	int pid, msqid;//后者为消息队列识别代号
	struct msgbuf
	{
		long mtype;//消息类型
		char mtext[20];//消息内容
	}send_buf, receive_buf;

	printf("-----%d\n", sizeof(send_buf.mtype));

	int mykey = ftok("/root/123456/", 0x22);
	printf("mykey = %d\n", mykey);
	if (mykey == -1)
	{
		printf("mykey = %s\n", strerror(errno));
	}

	if ((msqid = msgget(IPC_PRIVATE, 0700)) < 0)//建立消息队列
	{
		printf("msgget建立消息队列失败。\n");
		exit(1);
	}
	else
		printf("msgget建立消息队列成功，该消息队列识别代号为%d。\n", msqid);
	if ((pid = fork()) < 0)
	{
		printf("fork()函数调用失败！\n");
		exit(2);
	}
	else if (pid > 0)//父进程，发送消息到消息队列
	{
		send_buf.mtype = 1;
		strcpy(send_buf.mtext, "My test information123456789");
		printf("发送到消息队列的信息内容为：%s\n", send_buf.mtext);

		printf("send_buf = %ld",sizeof(send_buf));

		if (msgsnd(msqid, &send_buf, 2, IPC_NOWAIT) < 0)//发送send_buf中的信息到msqid 对应的消息队列
		{
			printf("msgsnd消息发送失败。\n");
			exit(3);
		}
		else
			printf("msgsnd消息发送成功。\n");
		sleep(2);
		exit(0);
	}
	else//子进程，从消息队列中接收消息]
	{
		sleep(2);//等待父进程发送消息完成
		int infolen;//读到的信息数据长度
		if ((infolen = msgrcv(msqid, &receive_buf, 20, 0, IPC_NOWAIT)) < 0)//自消息队列接收信息
		{
			printf("msgrcv读取信息错误。\n");
			exit(4);
		}
		else
			printf("msgrcv读取信息成功。\n");
		printf("自消息队列读取到的内容为%s，共读取%d个字节。\n", receive_buf.mtext, infolen);
		if ((msgctl(msqid, IPC_RMID, NULL)) < 0)//删除msqid对应的消息队列
		{
			printf("msgctl函数调用出现错误。\n");
			exit(5);
		}
		else
		{
			printf("识别代号为%d的消息队列已经被成功删除。\n", msqid);
			exit(0);
		}
	}
}



void TestLinuxShm()
{
    int pid, shmid;//后者为共享内存识别代号
    char* write_address;
    char* read_address;
    struct shmid_ds dsbuf;
    if ((shmid = shmget(IPC_PRIVATE, 64, 0)) < 0)//分配共享内存
    {
        printf("shmid共享内存分配出现错误。\n");
        exit(1);
    }
    else
        printf("shmid共享内存分配成功，共享内存识别代号为：%d。\n", shmid);
    if ((pid = fork()) < 0)
    {
        printf("fork函数调用出现错误！\n");
        exit(2);
    }
    else if (pid > 0)//父进程，向共享内存中写入数据
    {
        printf("父进程的ID是：%d\n", getpid());
        write_address = (char*)shmat(shmid, NULL, 0);//连接共享内存
        if ((long)write_address == -1)
        {
            printf("shmat连接共享内存错误。\n");
            exit(3);
        }
        else
        {
            printf("shmat连接共享内存成功。\n");
            strcpy(write_address, "我是写入共享内存的测试数据");//将数据写入共享内存
            printf("写入共享内存的信息为“%s”。\n", write_address);
            if ((shmdt((void*)write_address)) < 0)//断开与共享内存的连接
                printf("shmdt共享内存断开错误。\n");
            else
                printf("shmdt共享内存断开成功。\n");
            sleep(2);
            return;
        }
    }
    else//子进程，从共享内存中读取数据
    {
        sleep(2);//等待父进程写入共享内存完毕
        printf("子进程ID是：%d\n", getpid());
        if ((shmctl(shmid, IPC_STAT, &dsbuf)) < 0)
        {
            printf("shmctl获取共享内存数据结构出现错误。\n");
            exit(4);
        }
        else
        {
            printf("shmctl获取共享内存数据结构成功。\n建立这个共享内存的进程ID是：%d\n", dsbuf.shm_cpid);
            printf("该共享内存的大小为：%d\n", dsbuf.shm_segsz);
            if ((read_address = (char*)shmat(shmid, 0, 0)) < 0)//连接共享内存
            {
                printf("shmat连接共享内存出现错误。\n");
                exit(5);
            }
            else
            {
                printf("自共享内存中读取的信息为：“%s”。\n", read_address);
                printf("最后一个操作该共享内存的进程ID是：%d\n", dsbuf.shm_lpid);
                if ((shmdt((void*)read_address)) < 0)//断开与共享内存的连接
                {
                    printf("shmdt共享内存断开错误。\n");
                    exit(6);
                }
                else
                    printf("shmdt共享内存断开成功。\n");
                if (shmctl(shmid, IPC_RMID, NULL) < 0)//删除共享内存及其数据结构
                {
                    printf("shmctl删除共享内存及其数据结构出现错误。\n");
                    exit(7);
                }
                else
                    printf("shmctl删除共享内存及其数据结构成功。\n");
                exit(0);
            }
        }
    }
}