#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string>

#include "test_common.h"
#include "global.h"
#include "protocol.pb.h"

//reactor::Reactor g_reactor;
#define g_reactor (*(sGlobal->g_reactor_ptr))

const size_t kBufferSize = 10240;
char g_read_buffer[kBufferSize];
char g_write_buffer[kBufferSize];
class RequestHandler : public reactor::EventHandler
{
public:

    RequestHandler(reactor::handle_t handle) :
        EventHandler(),
        m_handle(handle)
    {}

    virtual reactor::handle_t GetHandle() const
    {
        return m_handle;
    }

    const char* generateDataInfor(_data_ data )
    {
        if(data.flag() == 0)
        {
            return NULL;
        }
        std::string output = data.SerializeAsString();

        return output.c_str();
    }

    virtual void HandleWrite()
    {
        _data_ data;
	std::string buffer = g_read_buffer;
	data.ParseFromString(buffer);
        _data_ ret; 
        switch(data.flag())
        { 
	    case 1:
		if(server_insert(data.mutable_infor()) == 0) 
		{
		    ret.set_ret("Success");	
		}
		else
		{
		    ret.set_ret("fail");	
		}
		ret.set_flag(1);
		break;
	    case 2:
		break;
	    default:
		break;
	}
	ret.mutable_msgbase()->set_opcode(1);
	int len = sprintf(g_write_buffer,"%s",generateDataInfor(ret));
        len = send(m_handle, g_write_buffer, len, 0);
        if (len > 0)
        {
            fprintf(stderr, "send response to client, fd=%d\n", (int)m_handle);
            g_reactor.RegisterHandler(this, reactor::kReadEvent);
        }
        else
        {
            ReportSocketError("send");
        }
    }

    virtual void HandleRead()
    {
        memset(g_read_buffer, 0, kBufferSize);
        int len = recv(m_handle, g_read_buffer, kBufferSize, 0);
        if (len > 0)
        {
	    BaseMessage baseMsg;
	    std::string buffer = g_read_buffer;
            baseMsg.ParseFromString(buffer);
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

    virtual void HandleError()
    {
        fprintf(stderr, "client %d closed\n", m_handle);
        close(m_handle);
        g_reactor.RemoveHandler(this);
        delete this;
    }


   int server_insert(_infor_ *c){
	FILE *fp;
	fp = fopen(SRC,"a+b");
	_infor_ tmp;
	fseek(fp,0,SEEK_SET);
	int offset;	
	while(1) {
		if(fread(&tmp,sizeof(tmp),1,fp)<=0)break;
		if(strcmp(tmp.id().c_str(),c->id().c_str()) == 0) {
			fclose(fp);
			return -1;
		}
	}
	fwrite(c,1,sizeof(_infor_),fp);
	fclose(fp);
	return 0;
    }

    int server_delete(char _id[]) {	
	FILE *fp;
        fp = fopen(SRC,"r+");
        _infor_ tmp;
        fseek(fp,0,SEEK_SET);
	int i;
	int read_num;
	int offset;
	i=0;
        while(1) {
	    if(read_num = fread(&tmp,sizeof(tmp),1,fp)<=0)break;
	    if(strcmp(tmp.id().c_str(),_id) == 0) {
		offset=sizeof(_infor_)*i;
		fseek(fp,offset,SEEK_SET);	
		tmp.set_id("aaaaaaaaa");
		fwrite(&tmp,1,sizeof(_infor_),fp);
		fclose(fp);
		return 0;
            }
	    i++;
	}
	fclose(fp);
	return -1;
    }

    int server_modify(_infor_ *c) {
	FILE *fp;
        fp = fopen(SRC,"r+");
        _infor_ tmp;
        fseek(fp,0,SEEK_SET);
        int read_num;
        int offset;
	int i = 0;
	while(1) {
	     if(read_num = fread(&tmp,sizeof(tmp),1,fp)<=0)break;
             if(strcmp(tmp.id().c_str(),c->id().c_str()) == 0) {
	         offset=sizeof(_infor_)*i;
		 i++;
		 fseek(fp,offset,SEEK_SET);
		 fwrite(c,1,sizeof(_infor_),fp);
		 fclose(fp);
		 return 0;
	     }
        }
        fclose(fp);
        return -1;
    }

    _infor_ *server_search(char id[]) {
	FILE *fp;
	fp = fopen(SRC,"r");
        _infor_ tmp;
	_infor_ *ret;
	ret=(_infor_*)malloc(sizeof(_infor_));
	fseek(fp,0,SEEK_SET);
        int read_num;
        while(1) {
            if(read_num = fread(&tmp,sizeof(_infor_),1,fp)<=0)break;
            if(strcmp(tmp.id().c_str(),id) == 0) {
	 	ret->set_id(tmp.id());
		ret->set_name(tmp.name());
		ret->set_sex(tmp.sex());
		ret->set_num(tmp.num());
		ret->set_date(tmp.date());
		//strcpy(ret->id(),tmp.id);
		//strcpy(ret->name,tmp.name);
		//strcpy(ret->sex,tmp. sex);
		//strcpy(ret->num,tmp.num);
		//strcpy(ret->date,tmp.date);
		return ret;
            }
        }
        fclose(fp);
	return NULL;
    }

private:
    const char* SRC = "Record.bin";

    reactor::handle_t m_handle;
};

class TimeServer : public reactor::EventHandler
{
public:

    TimeServer(const char * ip, unsigned short port) :
        EventHandler(),
        m_ip(ip),
        m_port(port)
    {}

    bool Start()
    {
        m_handle = socket(AF_INET, SOCK_STREAM, 0);
        if (!IsValidHandle(m_handle))
        {
            ReportSocketError("socket");
            return false;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(m_port);
        addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        if (bind(m_handle, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            ReportSocketError("bind");
            return false;
        }

        if (listen(m_handle, 10) < 0)
        {
            ReportSocketError("listen");
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
	printf("handleRead of TimerServer");
        struct sockaddr addr;
        socklen_t addrlen = sizeof(addr);
        reactor::handle_t handle = accept(m_handle, &addr, &addrlen);
        if (!IsValidHandle(handle))
        {
            ReportSocketError("accept");
        }
        else
        {
            RequestHandler * handler = new RequestHandler(handle);
            if (g_reactor.RegisterHandler(handler, reactor::kReadEvent) != 0)
            {
                fprintf(stderr, "error: register handler failed\n");
                delete handler;
            }
        }
    }

private:

    reactor::handle_t     m_handle;
    std::string           m_ip;
    unsigned short        m_port;
};

void printHelloworld(client_data* data)
{
    fprintf(stderr, "timertask : Hello world from timerTask!\n");
}

int main(int argc, char ** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s ip port\n", argv[0]);
        return EXIT_FAILURE;
    }

    TimeServer server(argv[1], atoi(argv[2]));
    if (!server.Start())
    {
        fprintf(stderr, "start server failed\n");
        return EXIT_FAILURE;
    }
    fprintf(stderr, "server started!\n");

    while (1)
    {
        g_reactor.RegisterHandler(&server, reactor::kReadEvent);
        g_reactor.HandleEvents();
    }
    return EXIT_SUCCESS;
}

