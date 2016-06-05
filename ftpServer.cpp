//ftp server
#include"Trace.h"
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<memory.h>
#include<stdlib.h>
#include<string>
#include<vector>
#include<fstream>
#include<stdio.h>
#ifndef BUFSIZ
#define BUFSIZ 512
#endif

#ifndef SOCKET
typedef int SOCKET;
#endif

#define __DEBUG__


using namespace std;

bool Download(SOCKET s,string path);
int inputNumber(SOCKET s);
void ReadFileList(string path,vector<string> & v);
int Init(unsigned short port)
{
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	int sin_size;
	char buf[BUFSIZ];

    memset(&my_addr,0,sizeof(my_addr));
   //set family & ip & port
	my_addr.sin_family=AF_INET;
	my_addr.sin_addr.s_addr=INADDR_ANY;
    my_addr.sin_port=htons(port);   //8000 port

	//create socket
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		cerr<<"create socket error "<<endl;
		return -1;
	}

	//bind
	
	if(bind(sock,(struct sockaddr*)&my_addr,sizeof(struct sockaddr) )<0 )
	{
		cerr<<"bind error"<<endl;
		return -2;
	}
    //listen
	listen(sock,5);
 
	//sock returned is used to Accept function
	return sock;
    //init over
	
}

//path is ftp main directory
void Accept(SOCKET sock,string path,vector<string>& fileList)
{
	struct sockaddr_in remote_addr;
    //memset(&remote_addr,0,sizeof(remote_addr));

	if(sock<0)
	{
		cerr<<"sock<0 Accept Error"<<endl;
		return ;
	}
	int sin_size=sizeof(struct sockaddr_in);
	int fd=accept(sock,(struct sockaddr *)&remote_addr,&sin_size);
	if(fd<0)
	{
		cerr<<"accept error"<<endl;
		return ;
	}

	cout<<"accepted client "<<inet_ntoa(remote_addr.sin_addr)<<endl;

	//get file list from file system
 
    string path2= "ls " + path+ " | grep \".*\" > /home/m/ftp/filelist.txt ";

	system(path2.c_str());

#ifdef __DEBUG__
	cout<<"File List:"<<endl;
#endif
	ReadFileList(string("/home/m/ftp/filelist.txt"),fileList);

	char sendBuf[BUFSIZ];
	memset(sendBuf,0,sizeof(char)*BUFSIZ);
	
   	char str[20];

	__TRACE__("file list:\n");

	for(int i=0;i<fileList.size();++i)
	{
	   __TRACE__("%s",fileList[i].c_str());
	   strcpy(sendBuf,fileList[i].c_str());
     	   send(fd,sendBuf,strlen(sendBuf)+1,0);
	}

	send(fd,"fileList\nend",13,0);

	int number=inputNumber(fd);
	__TRACE__("file number:%d",number);

	string filepath=path+"/"+fileList[number];

	if(Download(fd,filepath))
		__TRACE__("download failed!!\n");
		//cout<<"download failed!!"<<endl;
	else
		__TRACE__("download success!!\n");
		//cout<<"download success!!"<<endl;
		

}

bool Download(SOCKET s,string path)
{
	FILE * fp=fopen(path.c_str(),"rb");
	char sendBuf[BUFSIZ+1]={0};

#ifndef __DEBUG__
	cout<<"file path:"<<path<<endl;
#endif

	if(!fp)
		return false;
	while(1)
	{
		int len=fread(sendBuf,1,BUFSIZ,fp);
		if(len<BUFSIZ)
		{
			send(s,sendBuf,len,0);
			cout<<"len:"<<len<<endl;
			break;
		}
		if(len<1)
		{
			//send a empty packet
			memset(sendBuf,0,BUFSIZ);
			send(s,sendBuf,0,0);
			break;
		}
		send(s,sendBuf,len,0);
		cout<<"len:"<<len<<endl;
	}
	return true;
}

void ReadFileList(string path,vector<string> & v)
{
	ifstream ifs;
	ifs.open(path.c_str());
	string tmp;
	while(ifs.peek()!=EOF)
	{
		getline(ifs,tmp);
		v.push_back(tmp);
	}
}

//wait user input
int inputNumber(SOCKET s)
{
	char recBuf_num[20]={0};
	int len=recv(s,recBuf_num,20,0);
	int num=-1;
	if(len<=0)
		return -1;
	else
	{
		num=atoi(recBuf_num);
	}
#ifdef __DEBUG__
cout<<"receive num:"<<num<<endl;
#endif
	return num;
	
}

int main()
{
   while(1)
   {
		vector<string> fileList;
		int r=Init(8000);  //listen wait 
		if(r<0)
		{
		    cout<<"init error"<<endl;
	    }
		Accept(r,string("/home/m/ftpFile"),fileList);  //add code for child  process
   }
   return 0;
}
