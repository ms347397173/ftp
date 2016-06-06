//ftp server
#define __DEBUG__
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
#include<unistd.h>
#include<stdio.h>
#ifndef BUFSIZ
#define BUFSIZ 512
#endif

#ifndef SOCKET
typedef int SOCKET;
#endif



using namespace std;

bool sendFileList(SOCKET fd,vector<string> & file_list);
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

bool GetFileList(const string & file_path,vector<string> & v)
{
    string path= "ls " + file_path+ " | grep \".*\" > ./filelist.txt ";

	//get file list from file system
	system(path.c_str());
	__TRACE__("File List:\n");
	ReadFileList(string("./filelist.txt"),v);
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
	int sin_size;
	int fd;

	while(1)
	{
		sin_size=sizeof(struct sockaddr_in);
		fd=accept(sock,(struct sockaddr *)&remote_addr,&sin_size);
		if(fd<0)
		{
			cerr<<"accept error"<<endl;
			return ;
		}

		cout<<"accepted client "<<inet_ntoa(remote_addr.sin_addr)<<endl;
	
		//add code for child process

		pid_t pid=fork();
		if(pid==-1)
			return ;
		else if(pid==0)  //child process execute download
		{
	
			GetFileList(path,fileList);

			sendFileList(fd,fileList);

			int number=inputNumber(fd);
			__TRACE__("file number:%d",number);

			string filepath=path+"/"+fileList[number];

			if(Download(fd,filepath))
			{
				cout<<"download failed!!\n";
				//cout<<"download failed!!"<<endl;
			}
			else
			{
				cout<<"download success!!\n";
				//cout<<"download success!!"<<endl;
			}
			return ;
		}
		else  //father 
		{
			continue; //father process continue
		}

	}
}

bool sendFileList(SOCKET fd,vector<string> & fileList)
{
	if(fd<0)
		return false;

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
	return true;
}

bool Download(SOCKET s,string path)
{
	FILE * fp=fopen(path.c_str(),"rb");
	char sendBuf[BUFSIZ+1]={0};

	__TRACE__("file path: %s \n  fp=%p\n",path.c_str(),fp);

	if(!fp)
		return false;
	while(1)
	{
		int len=fread(sendBuf,1,BUFSIZ,fp);
		if(len<BUFSIZ)
		{
			send(s,sendBuf,len,0);
	//		__TRACE__("len:%d\n",len);
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
	   	__TRACE__("len:%d\n",len);
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

	__TRACE__("receive num: %d\n",num);

	return num;
	
}

int main(int argc,char * argv[])   
{
	vector<string> fileList;
	int r=Init(8000);  //listen over
	if(r<0)
	{
		cout<<"init error"<<endl;
    }
	Accept(r,string("/home/m/ftpFile"),fileList);  //add code for child  process
    return 0;
}
