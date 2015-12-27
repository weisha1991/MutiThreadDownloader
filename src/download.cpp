//#include "stdafx.h"

#include "download.h"
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::vector;
HttpDownloadDomain  *HttpDownloadDomain::downloader_instance=NULL;
std::string Log_Format(const char *format, ...)
{
		char szLog[1024]={0};
		va_list ap;
		va_start(ap, format);
		vsprintf(szLog, format, ap);
		va_end(ap);
		return std::string(szLog);
}


HttpDownloadDomain::HttpDownloadDomain(const string &url,const string &filefullpath,const string &filename):m_fileurl(url),m_savePath(filefullpath),
	m_fileName(filename),m_downloadFileSize(0),m_recvFileSize(0),m_supportRangeFlag(false),m_RedirectedFlag(false),m_lastHttpCode(0)
{}

/*
static HttpDownloadDomain* HttpDownloadDomain::CreateDownloadObj(const string &url,const string &filefullpath,const string &filename){
	if(downloader_instance==NULL)
		downloader_instance=new HttpDownloadDomain(url,filefullpath,filename);
	return downloader_instance;
}
*/
HttpDownloadDomain::~HttpDownloadDomain()
{
	auto iter=threadInfo_vec.begin();
	while(iter!=threadInfo_vec.end()){
		threadInfo *temp=*iter;
		if(temp)
			delete temp;
		++iter;
	}
}

size_t HttpDownloadDomain::DownloadCallback(void* pBuffer, size_t size, size_t nmemb, void* pParam)  
{  
	size_t writeLen=0;
   if(pParam){
   		int idx=*((int*)(pParam));
   		threadInfo *pinfo=downloader_instance->threadInfo_vec[idx];
   		if(pinfo)
   		{	   		
   			if (pinfo->recvfilesize < pinfo->filepart_len)
   			{
		   		if(fseek(pinfo->fileHandler,pinfo->recvfilesize,SEEK_SET)==0){
			   		unsigned long ulUnRecvSize = pinfo->filepart_len-pinfo->recvfilesize;
					size_t toWriteCount = (ulUnRecvSize > size * nmemb ? nmemb : ulUnRecvSize/size);
					writeLen = fwrite(pBuffer, size, toWriteCount, pinfo->fileHandler);
					pinfo->recvfilesize+= writeLen*size;
	   			}
	   			else{
		   				cout<<Log_Format("fseek error\n");   				
	   			}
   			}
   			else{
   				cout<<Log_Format("thread %d has full size\n",idx);
   			}
   		}
   }
   return size*nmemb; 
} 
 
size_t HttpDownloadDomain::singleThreadWriteData(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam){
	
	size_t writenNum=fwrite(pBuffer,nSize,nMemByte,(FILE*)pParam);
	if(writenNum!=nMemByte)
	{
		cout<<Log_Format("write bytes error\n");
	}
	downloader_instance->m_recvFileSize+=nSize*nMemByte;
	return nSize*nMemByte;
}
/*  
int HttpDownloadDomain::ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)  
{  
	HttpDownloadDomain* dd = (HttpDownloadDomain*)clientp;

    if ( dltotal > -0.1 && dltotal < 0.1 )  
    {
		return 0;
	}
    int nPos = (int) ( (dlnow/dltotal)*100 );  
    //通知进度条更新下载进度
    std::cout << "dltotal: " << (long)dltotal << " ---- dlnow:" << (long)dlnow << std::endl;

	if(dd->cancel_)
	{
		//1. 返回非0值就会终止 curl_easy_perform 执行
		return -2;
	}
     
    
    return 0; 
}
*/
size_t HttpDownloadDomain::HeaderInfo(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string* pHead = (std::string*)userdata;
	if (pHead)
	{
		pHead->append(std::string((char*)ptr, nmemb));
	}
	return size*nmemb;
}

bool HttpDownloadDomain::IsSupportRangeDownload()
{
	int retry_times = 0;
	string strUrl = m_fileurl;
	CURL* curl = curl_easy_init();
	while(retry_times<MAX_RETRY_TIMES){
		if(m_downloadFileSize>MAX_BLOCK_SIZE){		
			curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());  
			curl_easy_setopt(curl, CURLOPT_HEADER, 1); 
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
			// 通过头部信息判断是否支持断点续传
			std::string strHeader;
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &strHeader);
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &HttpDownloadDomain::HeaderInfo);
			curl_easy_setopt(curl, CURLOPT_RANGE, "0-1");
			// 整个请求超时控制
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, /*15L*/MM_TIMEOUT);
			// 优化性能，防止超时崩溃 
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			CURLcode code = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			curl = NULL;
			if(code==CURLE_OK){
				m_supportRangeFlag=(strHeader.find("Content-Range: bytes") != std::string::npos);
				if(m_supportRangeFlag)
				{
					cout<<"support RangeDownload"<<endl;
					curl_easy_cleanup(curl);
					return true;
				}
				else
				{
					cout<<"unspport RangeDownload"<<endl;
					curl_easy_cleanup(curl);
					return false;
					
				}
			}			
			else 
			{			
				// 重试时间：一秒，两秒，四秒 
				float iVal = pow(2.0, retry_times);
				sleep(iVal*1000);
				retry_times++;
				cout<<Log_Format("Downloader Obj Ptr: 0X%p, 判断是否支持多线程失败, CURLcode:%d, 尝试第%d次重试\n", this,code,retry_times);				
			}
			
		}
		else{
				m_supportRangeFlag=false;
				curl_easy_cleanup(curl);
				cout<<Log_Format("Download File's size is smaller than the default value,then shut off mutliThread mode\n");		
				return false;
			}
 	}
	cout<<Log_Format("retry times exceed\n");
	curl_easy_cleanup(curl);
	return false;
}
bool HttpDownloadDomain::GetFileLen()
{	
	CURL *curl=curl_easy_init();
	double fileLength=0;
	unsigned int retry_cnt=0;
	
	while(retry_cnt<MAX_RETRY_TIMES){
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");		
		curl_easy_setopt(curl, CURLOPT_URL, m_fileurl.c_str());
		curl_easy_setopt(curl, CURLOPT_HEADER, 1); 
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, /*15L*/MM_TIMEOUT);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		long lCode=0;
		CURLcode code = curl_easy_perform(curl);
	
		if(code==CURLE_OK){
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &lCode);
			m_lastHttpCode=lCode;
			if(lCode == 301 || lCode == 302){
				retry_cnt++;
				char *redirecturl = {0};
				curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirecturl);
				cout<<Log_Format("Downloader Obj Ptr: 0X%p,RedirectUrl: %s;retry number:%d\n",this, redirecturl,retry_cnt);
				m_fileurl = string(redirecturl);
				m_RedirectedFlag=true;
				continue;
			
			}
			else if (lCode >= 200 && lCode < 300)
			{
				if (lCode != 200)
				{
					cout<<Log_Format("Downloader Obj Ptr: 0X%p, HttpCode: %d\n", this, lCode);
				}
				curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fileLength);
			}
		}
		else{
			cout<<Log_Format("Get file size failed:curl_easy_perform error\n");
			curl_easy_cleanup(curl);
			return false;
		}
		
		if(fileLength<=0){
			if(lCode==404){
				curl_easy_cleanup(curl);
				return false;
				
			}
			float iVal = pow(2.0, retry_cnt);
			sleep(iVal*1000);
			retry_cnt++;
			cout<<Log_Format("Downloader Obj Ptr: 0X%p,the %dth retry",this,retry_cnt);	
		}	
		else{
			m_downloadFileSize=fileLength;
			cout<<Log_Format("File Size is:%ldMB\n",m_downloadFileSize/(1024*1024));
			curl_easy_cleanup(curl);
			return true;
		}
	}
	cout<<Log_Format("retry times exceed the default value\n");
	curl_easy_cleanup(curl);
	return false;
}


bool HttpDownloadDomain::DownloadInit(){
	bool initSucess=false;
	if(GetFileLen())
	{
		if(IsSupportRangeDownload())
			initSucess=true;
	}
	if(initSucess){
		cout<<"download init sucess"<<endl;
		bool flag=(m_downloadFileSize<1024*1024);
		cout<<Log_Format("File Size is %d Bytes\n",m_downloadFileSize);
		cout<<Log_Format("Support range download:%s\n",m_supportRangeFlag?("true"):("false"));
		return true;
	}
	else{
		cout<<"download init error"<<endl;
		return false;
	}
}

bool HttpDownloadDomain::singleTheadDownload(){
	CURL *curl=curl_easy_init();
	CURLcode code=curl_easy_setopt(curl,CURLOPT_URL,m_fileurl.c_str());
	string filename=m_savePath+m_fileName;
	FILE *file=fopen(filename.c_str(),"wb+");
	if(file==NULL){
		cout<<Log_Format("open file error");
		return false;
	}
	code=curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloader_instance->singleThreadWriteData);
	if(code!=CURLE_OK){
		cout<<Log_Format("set curlopt_writefunction error\n");
		return false;
	}
	code=curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	if(code!=CURLE_OK){
		cout<<Log_Format("set CURLOPT_WRITEDATA error\n");
		return false;
	}
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 
	if(code!=CURLE_OK){
		cout<<Log_Format("set CURLOPT_FOLLOWLOCATION error\n");
		return false;
	}
	code = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 
	if(code!=CURLE_OK){
		cout<<Log_Format("set CURLOPT_NOSIGNAL error\n");
		return false;
	}
	code = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);  
	if(code!=CURLE_OK){
		cout<<Log_Format("set CURLOPT_LOW_SPEED_LIMIT error\n");
		return false;
	}
	code = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, /*5L*/30L); 
	if(code!=CURLE_OK){
		cout<<Log_Format("set CURLOPT_LOW_SPEED_TIME error\n");
		return false;
	}
//	code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, /*15L*/downloader_instance->MM_TIMEOUT);
//	if(code!=CURLE_OK){
//		cout<<Log_Format("set CURLOPT_CONNECTTIMEOUT error\n");
//		return false;
//	}
	char error_buf[256];
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,error_buf); 
	code=curl_easy_perform(curl);
	if(code!=CURLE_OK){
		cout<<Log_Format("set curl_easy_perform error:");		
		cout<<Log_Format("%s\n",error_buf);
		return false;
	}
	curl_easy_cleanup(curl);
	fclose(file);
	return true;
}

bool HttpDownloadDomain::start(){
	
/*	if(!DownloadInit()){
		
		return false;	
	}			
*/
	if(!GetFileLen()||m_lastHttpCode>=400){
		return false;
	}
	if(!IsSupportRangeDownload()){
		if(m_supportRangeFlag==false){
			cout<<Log_Format("single thread mode\n");
			return singleTheadDownload();
		}
		else
			return false;
	}		
	int thread_cnt=m_downloadFileSize/MAX_BLOCK_SIZE;
	thread_cnt=m_supportRangeFlag?(std::min(thread_cnt,5)):1;
//	threadInfo *pinfo=new threadInfo[thread_cnt];
	unsigned long realBlocksize=m_downloadFileSize/thread_cnt;
	
	for(int i=0;i<thread_cnt;++i){
		
		threadInfo *pinfo=new threadInfo;	
		pinfo->t_idx=i;
		pinfo->data_begin=i*realBlocksize;		
		if(i==thread_cnt-1){
			pinfo->filepart_len=m_downloadFileSize-i*realBlocksize;
			pinfo->data_end=pinfo->data_begin+pinfo->filepart_len;
		}
		else{
			pinfo->filepart_len=realBlocksize;
			pinfo->data_end=pinfo->data_begin+realBlocksize;
		}
		char partname[2];
		sprintf(partname,"%d",i);
		pinfo->filepath=m_savePath+m_fileName+"part"+string(partname);
//		downloader_instance->threadInfo_map[i]=pinfo;
		downloader_instance->threadInfo_vec.push_back(pinfo);
		pthread_create(&(pinfo->tid),NULL,&DownloadFile,(void*)pinfo);
		cout<<Log_Format("create %dth thread sucess:",i);
		cout<<Log_Format("fileblockSize=%ld,dataRange:%ld--%ld\n",pinfo->filepart_len,pinfo->data_begin,pinfo->data_end-1);
	}
	
//	for(auto iter=downloader_instance->threadInfo_vec.begin();iter!=downloader_instance->threadInfo_vec.end();++iter){	
	for(int i=0;i<downloader_instance->threadInfo_vec.size();++i){
		if(pthread_join(downloader_instance->threadInfo_vec[i]->tid,nullptr)!=0)
		{
			cout<<Log_Format("join thread error\n");
			return false;
		}
		cout<<Log_Format("thread %d finish\n",i);
	}
	if(downloader_instance->mergeFile())	
	{	cout<<Log_Format("Merge file sucess\n");
		return true;
	}
	else{
		cout<<Log_Format("Merge file failed\n");
		return false;
	}
}

bool HttpDownloadDomain::print_curlerror(const string error_str,CURLcode code,CURL *curl,FILE *file){
	if(code!=CURLE_OK)
	{
		cout<<Log_Format("%s",error_str);
		curl_easy_cleanup(curl);
		curl=NULL;
		if(file)
			fclose(file);
		return false;
	}
	return true;
}

bool HttpDownloadDomain::curlInit(threadInfo *pinfo)
{
	
    CURLcode code;
    CURL *curl = curl_easy_init();  
    pinfo->curl=curl;
    code=curl_easy_setopt(curl, CURLOPT_URL, (downloader_instance->m_fileurl).c_str());
	print_curlerror("set url error",code,curl,NULL);	
    //设置接收数据的回调 
	pinfo->fileHandler=fopen(pinfo->filepath.c_str(), "wb+");
	if(pinfo->fileHandler==NULL){
		cout<<Log_Format("open file error\n");
		curl_easy_cleanup(curl);
		pinfo->curl=NULL;
		fclose(pinfo->fileHandler);
		return false;
	}
	
    code=curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloader_instance->DownloadCallback);
	if(print_curlerror("set CURLOPT_WRITEFUNCTION error",code,curl,pinfo->fileHandler)==false)
		return false;
	
	code=curl_easy_setopt(curl, CURLOPT_WRITEDATA,&pinfo->t_idx);
   	if(print_curlerror("set CURLOPT_WRITEDATA error",code,curl,pinfo->fileHandler)==false)
   		return false;
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 
	if(print_curlerror("set CURLOPT_FOLLOWLOCATION error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);  
	if(print_curlerror("set CURLOPT_NOSIGNAL error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);  
	if(print_curlerror("set CURLOPT_LOW_SPEED error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, /*5L*/30L); 
	if(print_curlerror("set CURLOPT_LOW_SPEED_TIME error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, /*15L*/downloader_instance->MM_TIMEOUT);
	if(print_curlerror("set CURLOPT_CONNECTTIMEOUT error",code,curl,pinfo->fileHandler)==false)
		return false;
	return true;
}
bool HttpDownloadDomain::curlRetryInit(threadInfo *pinfo){
	CURLcode code;
    CURL *curl = curl_easy_init();  
    pinfo->curl=curl;
    code=curl_easy_setopt(curl, CURLOPT_URL, (downloader_instance->m_fileurl).c_str());
	print_curlerror("set url error",code,curl,NULL);	
    //设置接收数据的回调 
    fclose(pinfo->fileHandler);
	pinfo->fileHandler=fopen(pinfo->filepath.c_str(), "ab");
	if(pinfo->fileHandler==NULL){
		cout<<Log_Format("open file error\n");
		curl_easy_cleanup(curl);
		pinfo->curl=NULL;
		fclose(pinfo->fileHandler);
		return false;
	}
	
    code=curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloader_instance->DownloadCallback);
	if(print_curlerror("set CURLOPT_WRITEFUNCTION error",code,curl,pinfo->fileHandler)==false)
		return false;
	
	code=curl_easy_setopt(curl, CURLOPT_WRITEDATA,&pinfo->t_idx);
   	if(print_curlerror("set CURLOPT_WRITEDATA error",code,curl,pinfo->fileHandler)==false)
   		return false;
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); 
	if(print_curlerror("set CURLOPT_FOLLOWLOCATION error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);  
	if(print_curlerror("set CURLOPT_NOSIGNAL error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);  
	if(print_curlerror("set CURLOPT_LOW_SPEED error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, /*5L*/30L); 
	if(print_curlerror("set CURLOPT_LOW_SPEED_TIME error",code,curl,pinfo->fileHandler)==false)
		return false;
	code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, /*15L*/downloader_instance->MM_TIMEOUT);
	if(print_curlerror("set CURLOPT_CONNECTTIMEOUT error",code,curl,pinfo->fileHandler)==false)
		return false;
	return true;

};

void* HttpDownloadDomain::DownloadFile(void *arg){
	threadInfo *pinfo=(threadInfo*)arg;
	bool bRet=true;
	if(downloader_instance->curlInit(pinfo)){
		unsigned long ulbegin=0,ulUnRecvSize=0,ulCurRecvSize=0;
		CURLcode code;
		
		long lCode=0;
//		int retry_times=0;
ReTry:		
		if(pinfo->recvfilesize<pinfo->filepart_len)
		{
			ulUnRecvSize=pinfo->filepart_len-pinfo->recvfilesize;
			ulCurRecvSize=ulUnRecvSize;
			ulbegin=pinfo->data_begin+pinfo->recvfilesize;
			bRet=false;
			if(downloader_instance->m_supportRangeFlag){
				char *range=new char[32];
				memset(range,'\0',sizeof(32));
				sprintf(range,"%lu-%lu",ulbegin,ulbegin+ulUnRecvSize);
				code=curl_easy_setopt(pinfo->curl,CURLOPT_RANGE,range);
//				free(range);
			}
			else
			{
			
				code==CURLE_OK;
			}
			if(code==CURLE_OK)
			{
				code=curl_easy_perform(pinfo->curl);
				if(code==CURLE_OK)
					bRet=(ulUnRecvSize>(pinfo->filepart_len-pinfo->recvfilesize));
				if(!bRet)
				{				
					curl_easy_getinfo(pinfo->curl, CURLINFO_RESPONSE_CODE, &lCode);
					cout<<Log_Format("server response code:%ld,thread %d retry\n",lCode,pinfo->t_idx);
					if(pinfo->retry_times<downloader_instance->MAX_RETRY_TIMES){
						int tryTimes=pinfo->retry_times;
						curl_easy_cleanup(pinfo->curl);
						pinfo->curl=NULL;
						if(downloader_instance->curlRetryInit(pinfo)){
							float iVal = pow(2.0, pinfo->retry_times);
							sleep(iVal*1000);
							pinfo->retry_times++;
							// 对于单线程下载，重试要从0开始 
							if (!downloader_instance->m_supportRangeFlag)
							{
								pinfo->recvfilesize = 0;
								downloader_instance->m_recvFileSize = 0;
							}
							float fPercent=100.0*(downloader_instance->m_recvFileSize)/(downloader_instance->m_downloadFileSize);
							cout<<Log_Format("Downloader Obj Ptr: 0X%p, 下载进度:%f%%, 线程%d下载数据失败，lastHttpCode: %d, CURLcode: %d, 尝试第%d次重试\n", downloader_instance, fPercent, pinfo->t_idx, lCode, code,pinfo->retry_times);
							
							goto ReTry;
						}
						else{
							cout<<Log_Format("Curl Init\n");							
						}
					}
					else{
						cout<<Log_Format("retry error\n");
					}				
				}
			}
			if(!bRet)
			{
				Log_Format("Downloader Obj Ptr: 0X%p, 线程%d接收数据失败，错误码：CURLCode=%d\n",downloader_instance,pinfo->t_idx, code);	
				pthread_exit(NULL);
			}
		}
		else{
			if(pinfo->retry_times)
			cout<<Log_Format("Downloader Obj Ptr: 0X%p, 线程%d接收数据完毕，重试次数：%d\n", downloader_instance,pinfo->t_idx, pinfo->retry_times);
	//		pthread_exit(NULL);
		}
		curl_easy_cleanup(pinfo->curl);
		pinfo->curl=NULL;
	}
	fclose(pinfo->fileHandler);
	
//	if(bRet)
//		cout<<Log_Format("Downloader Obj Ptr: 0X%p, 线程%d接收数据完毕，重试次数：%d\n", downloader_instance,pinfo->t_idx, pinfo->retry_times);
	pthread_exit(NULL);
}

bool HttpDownloadDomain::mergeFile(){
	cout<<"start merge file"<<endl;
	auto iter=downloader_instance->threadInfo_vec.begin();
	auto end_iter=downloader_instance->threadInfo_vec.end();
	FILE *head=fopen((*iter)->filepath.c_str(),"ab");
	int read_num=0,write_num=0;
	char buffer[4096];
	if(head==NULL){
		cout<<Log_Format("open file error\n");
		return false;
	}
	while((++iter)!=end_iter){
		FILE *present=fopen((*iter)->filepath.c_str(),"rb");
		if(present==NULL){
			cout<<Log_Format("open file error\n");
			return false;
		}
		while(!feof(present)){
			read_num=fread(buffer,sizeof(char),4096,present);
			write_num=fwrite(buffer,sizeof(char),read_num,head);
		}
		cout<<Log_Format("Has read %d part file\n",(*iter)->t_idx);
		fclose(present);
		remove((*iter)->filepath.c_str());
		(*iter)->fileHandler=NULL;
	}
	fclose(head);
	char tempname[512];
	sprintf(tempname,"%s%s",m_savePath.c_str(),m_fileName.c_str());
	
	if(rename((downloader_instance->threadInfo_vec[0])->filepath.c_str(),tempname)<0){
		cout<<Log_Format("rename error\n");
		return false;
	}
	return true;
}




