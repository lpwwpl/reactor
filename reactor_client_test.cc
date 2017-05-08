#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "test_common.h"
#include "protocol.pb.h"
reactor::Reactor g_reactor;

const size_t kBufferSize = 10240;
char g_read_buffer[kBufferSize];
char g_write_buffer[kBufferSize];

class TimeClient : public reactor::EventHandler
{
public:

    TimeClient() :
        EventHandler()
    {
        m_handle = socket(AF_INET, SOCK_STREAM, 0);
        assert(IsValidHandle(m_handle));
    }

    ~TimeClient()
    {
        close(m_handle);
    }
    /*
    void show(_infor_ infor)
    {
	printf("empno: %s           name:  %s           sex:   %s           num:  %s           date:   %s\n",infor.id(),infor.name(),infor.sex(),infor.num(),infor.date());
	printf("\n");
    }
    */
    const char* generateDataInfor(_data_ data )
    { 
	if(data.flag() == 0)
	{
	    return NULL;
	}
	std::string output= data.SerializeAsString(); 

	return output.c_str();
    }
    
    _data_ printmainmenu()
    {
	printf("*****************\n");	
	printf("1、增加员工信息\n");
	printf("2、删除员工信息\n");
	printf("3、查找员工信息\n");
	printf("4、修改员工信息\n");
	printf("5、退出系统\n");
	char opt;
	opt = getchar();
	getchar();
	_data_ data ;
	data.set_flag(1);
	switch(opt) {
	case '1':
	{
		char temp[255];
		_infor_* infor = data.mutable_infor();
		printf("********************\n");
		printf("请输入员工号(10位)：");
		if(NULL == gets(temp))
			perror("id");
		infor->set_id(temp);
		printf("请输入员工姓名(20位)：");
		if(NULL == gets(temp))
			perror("name");
		infor->set_name(temp);
		printf("请输入员工性别(4位)：");
		if(NULL == gets(temp))
			perror("sex");
		infor->set_sex(temp);
		printf("请输入员工手机号(11位)：");
		if(NULL == gets(temp))
			perror("num");
		infor->set_num(temp);
		printf("请输入员工入职时间(12位)：");
		if(NULL == gets(temp))
			perror("date");
		infor->set_date(temp);
		data.set_type(_data__Type_INFOR);
		//Send_Information(1,data);
		break;
	}
	case '2':
		printf("***********************\n");
		printf("请输入要删除的员工号：");
		//if(NULL == gets(data.infor.id))
		//	perror("id");
		//data.flag = FLAG_DEL;
		//send_information();
		//printf("%s\n",ret.ret());
		break;
	case '3':
		printf("***********************\n");
		printf("请输入要查询的员工号:");
		//if(NULL == gets(data.infor.id))
		//	perror("id");
		//data.flag = FLAG_SEARCH;
		//query(data.infor.id);
		break;
	case '4':
		printf("***********************\n");
		printf("请输入要修改的员工号:");
		/*
		gets(data.infor.id);
		data.flag = FLAG_SEARCH;
		if(1 == query(data.infor.id)) {
			printf("Please input name:");
			if(NULL == gets(data.infor.name))
				;//error_exit("name");
			printf("Please input sex:");
			if(NULL == gets(data.infor.sex))
				;//error_exit("sex");
			printf("Please input telephone num:");
			if(NULL == gets(data.infor.num))
				;//error_exit("tel");
			printf("Please input time of in company:");
			if(NULL == gets(data.infor.date))
				;//error_exit("date");
			data.flag=FLAG_MODIFY;
			send_information();				
		}
		printf("%s\n",ret.ret);
		*/
		break;
	case '5':
		/*
		data.flag=FLAG_LOGOUT;
		send_information();
		if(ret.flag=FLAG_ACK) {
			printf("%s\n",ret.ret);
			//close(sockfd);
			exit(EXIT_SUCCESS);
		} else {
					}
		*/
		break;
	default:
		break;
	}
	data.mutable_msgbase()->set_opcode(1);
	return data;
    }

    bool ConnectServer(const char * ip, unsigned short port)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);
        if (connect(m_handle, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            ReportSocketError("connect");
            return false;
        }
        return true;
    }

    virtual reactor::handle_t GetHandle() const
    {
        return m_handle;
    }

    virtual void HandleRead()
    {
        memset(g_read_buffer, 0, kBufferSize);
        int len = recv(m_handle, g_read_buffer, kBufferSize, 0);
        if (len > 0)
        {
            BaseMessage baseMsg;
            baseMsg.ParseFromString(g_read_buffer);
            int opcode = baseMsg.mutable_msgbase()->opcode();
            switch(opcode)
            {
                case 1:
                    g_reactor.RegisterHandler(this,reactor::kWriteEvent);
                    break;
                case 2:
                    //g_reactor.RegisterHandler(this,reactor::kWriteEvent);
                    break;
                //close(m_handle);
                //g_reactor.RemoveHandler(this);
                //delete this;
                //fprintf(stderr, "Invalid request: %s", g_read_buffer);
                default:
                    break;
	    }
        }
        else
        {
            ReportSocketError("recv");
        }
    }

    virtual void HandleWrite()
    {
	_data_ data = printmainmenu();
        memset(g_write_buffer, 0, kBufferSize);

        int len = sprintf(g_write_buffer, "%s",generateDataInfor(data));
        len = send(m_handle, g_write_buffer, len, 0);
        if (len > 0)
        {
            fprintf(stderr, "%s", g_write_buffer);
            g_reactor.RegisterHandler(this, reactor::kReadEvent);
        }
        else
        {
            ReportSocketError("send");
        }
    }

    virtual void HandleError()
    {
        fprintf(stderr, "server closed\n");
        close(m_handle);
        exit(0);
    }

private:

    reactor::handle_t  m_handle; 
};

int main(int argc, char ** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s ip port\n", argv[0]);
        return EXIT_FAILURE;
    }

    TimeClient client;
    if (!client.ConnectServer(argv[1], atoi(argv[2])))
    {
        fprintf(stderr, "connect remote server failed\n");
        return EXIT_FAILURE;
    }

    g_reactor.RegisterHandler(&client, reactor::kWriteEvent);
    while (1)
    {
        g_reactor.HandleEvents();
        sleep(1);
    }
    g_reactor.RemoveHandler(&client);
    return EXIT_SUCCESS;
}

