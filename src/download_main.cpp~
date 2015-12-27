#include <libgen.h>
#include "download.h"
#include <unistd.h>
#include <string>
#include <stdio.h>

using std::string;
using std::cout;
using std::endl;
int main(int argc, char* argv[])
{
 	if(argc<2){
 		cout<<"Usage:downloadmain filename"<<endl;
 		return -1;
 	}
    string url("http://jaist.dl.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.gz");
    string url1("http://60.21.164.30/ws.cdn.baidupcs.com/file/c43404ac4a1c9e3d1eee0d3b9c14843c?bkt=p3-1400c43404ac4a1c9e3d1eee0d3b9c14843cdc3368220000a0f49997&xcode=1237d7ada22d5423a2d58a3edba550713f86f439a077bf42ed03e924080ece4b&fid=2735878885-250528-983925317961809&time=1450872387&sign=FDTAXGERLBH-DCb740ccc5511e5e8fedcff06b081203-sHGfj8ovVmKEadOxPzYtLk4n2lk%3D&to=lc&fm=Nin,B,G,t&sta_dx=2575&sta_cs=22&sta_ft=mp4&sta_ct=5&fm2=Ningbo,B,G,t&newver=1&newfm=1&secfm=1&flow_ver=3&pkey=1400c43404ac4a1c9e3d1eee0d3b9c14843cdc3368220000a0f49997&sl=79102030&expires=8h&rt=pr&r=484495604&mlogid=8276076459853332120&vuk=2735878885&vbdid=393686169&fin=World%20War%20Z.BD1280%E8%B6%85%E6%B8%85%E4%B8%AD%E8%8B%B1%E5%8F%8C%E5%AD%97.mp4&fn=World%20War%20Z.BD1280%E8%B6%85%E6%B8%85%E4%B8%AD%E8%8B%B1%E5%8F%8C%E5%AD%97.mp4&slt=pm&uta=0&rtype=1&iv=0&isw=0&dp-logid=8276076459853332120&dp-callid=0.1.1&wshc_tag=0&wsts_tag=567a8e45&wsid_tag=dec5b496&wsiphost=ipdbm");
    string url2("http://download.virtualbox.org/virtualbox/5.0.12/virtualbox-5.0_5.0.12-104815~Ubuntu~wily_amd64.deb");
    string url3("http://60.21.164.31/ws.cdn.baidupcs.com/file/3309271b78b0d8ead79c1dd82992d978?bkt=p3-00000092b9480e57e8e14120dea3290080ed&xcode=1be45f8645ef0b4d7c488ff8c40f2da8313a940fcba8af07f77424e07ee197d9&fid=2735878885-250528-455816073096766&time=1451131599&sign=FDTAXGERLBH-DCb740ccc5511e5e8fedcff06b081203-HAEj2zpAY%2Fjg%2FXz62hKhas%2FCe0s%3D&to=lc&fm=Nin,B,G,t&sta_dx=813&sta_cs=317&sta_ft=mp4&sta_ct=5&fm2=Ningbo,B,G,t&newver=1&newfm=1&secfm=1&flow_ver=3&pkey=00000092b9480e57e8e14120dea3290080ed&sl=79364174&expires=8h&rt=sh&r=763822884&mlogid=8345658151333366235&vuk=2735878885&vbdid=393686169&fin=%5B%E4%B8%83%E6%B1%89%E5%BD%B1%E8%A7%86%5DThe.Walking.Dead.S06E01.HDTV.x264-FLEET.mp4&fn=%5B%E4%B8%83%E6%B1%89%E5%BD%B1%E8%A7%86%5DThe.Walking.Dead.S06E01.HDTV.x264-FLEET.mp4&slt=pm&uta=0&rtype=1&iv=0&isw=0&dp-logid=8345658151333366235&dp-callid=0.1.1&wshc_tag=0&wsts_tag=567e82d0&wsid_tag=dec5b496&wsiphost=ipdbm");
    string url4("http://lx.cdn.baidupcs.com/file/964fe0a41e0ec7cf4e7aa69919da5ff7?bkt=p3-0000eb4359408a35d6300639faf07aa0b1cb&xcode=5c259b7dd124e8e52827c3c9474b3ecaa1315ed92839835d837047dfb5e85c39&fid=2656480408-250528-201369040012699&time=1451132730&sign=FDTAXGERLBH-DCb740ccc5511e5e8fedcff06b081203-CNpAs8v3k9vHrdr2ek%2BeJAAL5x0%3D&to=lc&fm=Nan,B,G,t&sta_dx=612&sta_cs=81&sta_ft=mp4&sta_ct=5&fm2=Nanjing02,B,G,t&newver=1&newfm=1&secfm=1&flow_ver=3&pkey=0000eb4359408a35d6300639faf07aa0b1cb&sl=77725774&expires=8h&rt=sh&r=634592002&mlogid=8345961749850896646&vuk=2735878885&vbdid=393686169&fin=%5B%E4%B8%83%E6%B1%89%E5%BD%B1%E8%A7%86%5DThe.W.D.S06E01.%E4%B8%AD%E8%8B%B1%E5%AD%97%E5%B9%95.mp4&fn=%5B%E4%B8%83%E6%B1%89%E5%BD%B1%E8%A7%86%5DThe.W.D.S06E01.%E4%B8%AD%E8%8B%B1%E5%AD%97%E5%B9%95.mp4&slt=pm&uta=0&rtype=1&iv=0&isw=0&dp-logid=8345961749850896646&dp-callid=0.1.1");
    string file_path("/home/long/download/");
    string filename(argv[1]);
//  string user_url(argv[1]);
    HttpDownloadDomain *hdd=HttpDownloadDomain::CreateDownloadObj(url,file_path,filename);
   
// 	hdd->DownloadInit();
// 	cout<<hdd->get_filesize()<<endl;
 	if(hdd->start())
 		cout<<"download success"<<endl;
 	else
 		cout<<"download failed"<<endl;
/*	if(hdd->mergeFile())
		cout<<"merge file sucess"<<endl;
	else		
		cout<<"merge file faield"<<endl;
*/
// 	cout<<hdd.get_fileurl()<<endl;
	return 0;
}

