#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <string>
#include "../include/curl/curl.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <math.h>
#include <cmath>
#include <unistd.h>
//#include <map>
#include <vector>

using std::string;
using std::vector;
//using std::map;

class threadInfo{
public:
	threadInfo():data_begin(0),data_end(0),filepart_len(0),recvfilesize(0),retry_times(0),fileHandler(NULL),t_idx(-1),curl(NULL){}
	unsigned long data_begin;
	unsigned long data_end;
	unsigned long filepart_len;
	unsigned long recvfilesize;
	string  filepath;
	FILE *fileHandler;
	CURL *curl;
	pthread_t tid;
	int t_idx;
	int retry_times;
};


class HttpDownloadDomain{
private:	
	HttpDownloadDomain(const string&,const string&,const string&);
public:
	static HttpDownloadDomain* CreateDownloadObj(const string &url,const string &filefullpath,const string &filename){
		if(downloader_instance==NULL)
		downloader_instance=new HttpDownloadDomain(url,filefullpath,filename);
	return downloader_instance;
	}
	~HttpDownloadDomain();	
	unsigned long get_filesize()const{return m_downloadFileSize;}
	unsigned long get_recvsize()const{return m_recvFileSize;}
	const string& get_fileurl()const{return m_fileurl;}
	

	bool start();
protected:
	bool mergeFile();
	bool DownloadInit();
	bool curlInit(threadInfo *arg);
	bool curlRetryInit(threadInfo *arg);
	static void* DownloadFile(void *arg);
protected:
	bool singleTheadDownload();
	static size_t singleThreadWriteData(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam);
	static size_t HeaderInfo(char *ptr, size_t size, size_t nmemb, void *userdata);
	bool GetFileLen();
	bool IsSupportRangeDownload();
	static size_t DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam);
//	static int ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	bool print_curlerror(const string error_str,CURLcode code,CURL *curl,FILE *file);
private:
	unsigned long m_downloadFileSize;
	unsigned long m_recvFileSize;
	const string m_fileName;
	const string m_savePath;	
	string m_fileurl;
	bool m_supportRangeFlag;
	bool m_RedirectedFlag;
	vector<threadInfo*> threadInfo_vec;
	long m_lastHttpCode;
//	map<int,threadInfo*> threadInfo_map;
private:
	static const unsigned long MM_TIMEOUT=15L;
	static const unsigned int MAX_RETRY_TIMES=5;
	static const unsigned long MAX_BLOCK_SIZE=1024*1024*5;
	static const unsigned int MAX_THREAD_NUMBER=5;
	static HttpDownloadDomain  *downloader_instance;
};

#endif
