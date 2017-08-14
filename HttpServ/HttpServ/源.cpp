
#include <fstream>
#include <iostream>
#include <MString.h>
#include <string>
#include <vector>
#include <HPath_File.h>
#include <IAsioInterface.h>
#include <HVector.h>
#include <IFileSystem.h>
#include <IAsioInterface.h>
#include <IFileMap.h>


std::shared_ptr<IFileMap> pFileMap{ nullptr };
std::shared_ptr<IFileSystem>  pFileSystem{ nullptr };

void httpFunction(const char* msg, __int64 len, const char* clientid, std::shared_ptr<IServiceNet> __Serv){

	mj::MString str(msg,msg+len);
	std::vector<mj::MString> __Vstr;
	str.split("\n", __Vstr);
	std::cout << str << std::endl;
	if (__Vstr.empty())
		return;

	mj::MString __Accept;
	mj::MString __Head;
	for (auto s : __Vstr){
		if (s.istart_with("Accept:")){
			__Accept = s;
		}
		if (s.istart_with("get") || s.istart_with("post")){
			__Head = s;
		}
	}
	
	if (__Accept.empty() || __Head.empty())
		return;

	std::vector<mj::MString> __Get;
	unsigned pos = __Head.find_first("/");
	if (pos == -1)
		return;

	unsigned pos2 = __Head.find(" ", pos);
	if (pos2 == -1)
		return;
	mj::MString url = __Head.copy<int>(pos+1, pos2);
	url.trim();

	static mj::MString headStr;
	headStr = "HTTP/1.1 200 OK\r\nServer: Microsoft-IIS/7.5\r\n";
	headStr += file::getCurrentTimeStr();
	headStr += "\r\nConnection: Keep-Alive\r\nContent-Type: text/html\r\n";

	if (url.empty()){
		static mj::MString homeStr;
		static mj::MString strForC;
		homeStr.fromFile("home.html");
		strForC.fromFile("Resource/code/README.txt");
		homeStr.replace_all("ReplcaedC", strForC);
		strForC.fromFile("Resource/Other/README.txt");
		homeStr.replace_all("ReplcedXiaoShuode", strForC);
		headStr += "Content-Length:" + std::to_string(homeStr.length()) + "\r\n\r\n";
		headStr += homeStr;
		__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);
	}
	else{
		if (url.iend_with(".jpg") || url.iend_with(".bmp") || url.iend_with(".gif")){
			if (file::fileisexist(url)){
				file::_Path p(url.toStdString());
				mj::MString extension = p.extension();

				static mj::MString picStr;
				picStr.fromFile(url, 0);
				headStr = "HTTP/1.1 200 OK\r\nServer: Microsoft-IIS/7.5\r\n";
				headStr += file::getCurrentTimeStr();
				headStr += "\r\nConnection: Keep-Alive\r\nContent-Type: image/";
				headStr += extension + "\r\n";
				headStr += "Content-Length:" + std::to_string(picStr.length()) + "\r\n\r\n";
				__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);
				__Serv->PostSend(picStr.c_str(), picStr.length(), clientid);
			}
		}
		else if (url.iend_with(".rar")){
			if (pFileSystem->MFileIsExist(url)){
				file::_Path p(url.toStdString());
				mj::MString extension = p.extension();
				static mj::MString picStr;
				picStr.fromFile(url, 0);
				headStr = "HTTP/1.1 200 OK\r\nServer: Microsoft-IIS/7.5\r\n";
				headStr += file::getCurrentTimeStr();
				headStr += "\r\nConnection: Keep-Alive\r\nContent-Type: image/";
				headStr += extension + "\r\n";
				headStr += "Content-Length:" + std::to_string(picStr.length()) + "\r\n\r\n";
				__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);
				int len = picStr.length();
				if (len > 8 * 1024 * 1024){
					mj::MBuffer buffer;
					picStr.Buffer(buffer);
					int pos = 0;
					while (len < 8 * 1024 * 1024){
						__Serv->PostSend(buffer.mBuffer + pos, 8*1024*1024, clientid);
						len -= 8 * 1024 * 1024;
						pos += 8 * 1024 * 1024;
					}
					__Serv->PostSend(buffer.mBuffer + pos, len, clientid);
					mj::MString::FreeBuffer(buffer);
				}
				else{
					__Serv->PostSend(picStr.c_str(), picStr.length(), clientid);
				}
				
			}
			else{
				headStr = "HTTP/1.1 404 OK\r\nServer: Microsoft-IIS/7.5\r\n";
				headStr += file::getCurrentTimeStr();
				headStr += "\r\nConnection: Keep-Alive\r\nContent-Type: text/html\r\n";
				headStr += "Content-Length:0\r\n\r\n";
				__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);
			}
		}
		else if (url.iend_with(".iso")){
			if (pFileSystem->MFileIsExist(url)){
				__int64 len = pFileSystem->MFileSize(url);
				file::_Path p(url.toStdString());
				mj::MString extension = p.extension();
				headStr = "HTTP/1.1 200 OK\r\nServer: Microsoft-IIS/7.5\r\n";
				headStr += file::getCurrentTimeStr();
				headStr += "\r\nConnection: Keep-Alive\r\nContent-Type: image/";
				headStr += extension + "\r\n";
				headStr += "Content-Length:" + std::to_string(len) + "\r\n\r\n";
				__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);

				if (len > 8 * 1024 * 1024){
					__int64 pos = 0ll;
					while (len > 8 * 1024 * 1024){
						pFileMap->OpenFile(url.c_str());
						char* ch = nullptr;
						if (pFileMap->Map(ch, pos, 8 * 1024 * 1024) && ch){
							__Serv->PostSend(ch, 8 * 1024 * 1024, clientid);
							len -= 8 * 1024 * 1024;
							pos += 8 * 1024 * 1024;
						}
						pFileMap->UnMap();
					}
					if (len > 0){
						char* ch = nullptr;
						pFileMap->OpenFile(url.c_str());
						if (pFileMap->Map(ch, pos, len) && ch){
							__Serv->PostSend(ch, len, clientid);
							pFileMap->UnMap();
							return;
						}
						pFileMap->OpenFile(url.c_str());
					}
				}
			}
		}
		else if (url.iend_with(".lua") || url.iend_with(".txt") || url.iend_with(".cpp") || url.iend_with(".h")){
			if (!file::fileisexist(url)){
				headStr = "HTTP/1.1 404 OK\r\nServer: Microsoft-IIS/7.5\r\n";
				headStr += file::getCurrentTimeStr();
				headStr += "\r\nConnection: Keep-Alive\r\nContent-Type: text/html\r\n";
				headStr += "Content-Length:0\r\n\r\n";
				__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);
				return;
			}
				
			std::ifstream inFile("htmlHead.txt");
			std::string recall((std::istreambuf_iterator<char>(inFile)),
				std::istreambuf_iterator<char>());

			std::cout << recall << std::endl;
			std::ifstream inFile2(url.toStdString());
			std::string sendStr((std::istreambuf_iterator<char>(inFile2)),
				std::istreambuf_iterator<char>());
			mj::MString str;
			str(recall.c_str(), sendStr);
			headStr += "Content-Length:" + std::to_string(str.length()) + "\r\n\r\n";
			headStr += str;
			std::cout << headStr << std::endl;
			__Serv->PostSend(headStr.c_str(), headStr.length(),clientid);
		}
		else if (url.iend_with(".html")){
			static mj::MString htmlStr;
			std::ifstream inFile(url.toStdString());
			std::string str((std::istreambuf_iterator<char>(inFile)),
				std::istreambuf_iterator<char>());
			htmlStr = str;
			htmlStr.replace_all("ReplacedContex", "");
			headStr += "Content-Length:" + std::to_string(htmlStr.length()) + "\r\n\r\n";
			headStr += htmlStr;
			mj::MString utf8str = headStr;// .to_utf8();
			__Serv->PostSend(utf8str.c_str(), utf8str.length(), clientid);
		}
		else{
			std::string fileName = pFileSystem->MFileName(url);
			static mj::MString mFileName;
			mFileName = url;
			static mj::MString htmlStr;
			std::vector<std::string> vstr;
			pFileSystem->MFindFiles(url, "*.txt", vstr);
			if (vstr.empty()){
				headStr = "HTTP/1.1 404 OK\r\nServer: Microsoft-IIS/7.5\r\n";
				headStr += file::getCurrentTimeStr();
				headStr += "\r\nConnection: Keep-Alive\r\nContent-Type: text/html\r\n";
				headStr += "Content-Length:0\r\n\r\n";
				__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);
				return;
			}
			mFileName = vstr[0];
			if (pFileSystem->MFileIsExist(mFileName)){
				std::ifstream inFile(mFileName.toStdString());
				std::string str((std::istreambuf_iterator<char>(inFile)),
					std::istreambuf_iterator<char>());
				inFile.close();

				//
				// 读取index.html
				//
				fileName = pFileSystem->MParentPath(url);
				mFileName = fileName;
				mFileName << "/index.html";
				inFile.open(mFileName.toStdString());
				std::string str2((std::istreambuf_iterator<char>(inFile)),
					std::istreambuf_iterator<char>());
				inFile.close();
				htmlStr = str2;

				//
				// 解析code标签
				//
				static mj::MString sContext;
				sContext = str;
				vstr.clear();
				sContext.replace_all("\n", "*#*");
				sContext.reg_serch_all("<code>.*?</code>", vstr);
				if (!vstr.empty()){
					sContext.reg_replace("<code>.*?</code>", "###");
					sContext.replace_all("*#*", "</p><br><p>&nbsp&nbsp&nbsp&nbsp");
					sContext << "<br><br><br><br><hr><p>本章完</p><br><br><br><br><br><br><br><br>";
					for (auto s : vstr){
						static mj::MString __temps;
						static mj::MString __s;
						__temps = "==========================================================";
						__s = s;
						__s.replace_all("<code>", __temps);
						__s.replace_all("</code>", __temps);
						__s.replace_all("\t", "&nbsp&nbsp&nbsp&nbsp");
						__s.replace_all(" ", "&nbsp");
						__s.replace_all("<<", "&lt&lt");
						__s.replace_all(">>", "&gt&gt");
						__s.replace_all(">", "&gt");
						__s.replace_all("<", "&lt");
						__s.push_front("<br>");
						__s.replace_all("*#*", "<br>");
						sContext.replace_first("###", __s);
					}
				}
				else{
					sContext.replace_all("*#*", "</p><br><p>&nbsp&nbsp&nbsp&nbsp");
					sContext << "<br><br><br><br><hr><p>本章完</p><br><br><br><br><br><br><br><br>";
				}

				htmlStr.replace_all("ReplacedContex", sContext);
				headStr += "Content-Length:" + std::to_string(htmlStr.length()) + "\r\n\r\n";
				headStr += htmlStr;
				__Serv->PostSend(headStr.c_str(), headStr.length(), clientid);
			}
		}
	}
}

int main(){
	INITCOM;
	pFileSystem = SharedComPtr(CreateFileSystem());
	pFileMap = SharedComPtr(CreateFileMap());
	if (!pFileSystem || !pFileMap){
		return 0;
	}
	std::shared_ptr<IServiceNet> pServ = SharedComPtr(CreateServiceNet());
	if (!pServ)
		return 0;
	pServ->Run();
	pServ->BindMsgFun(std::bind(httpFunction,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3,
		pServ));
	pServ->SetListenPort(12000);
	pServ->StartListen();	
	while (1)
		Sleep(10000000);
	return 0;
}