// Community Clinic demonstration server: C++17 + SQLite3 + OpenSSL.
// Local development only. Do not deploy with demo credentials or without HTTPS.
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <regex>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <cstring>
#include <sqlite3.h>
#include <curl/curl.h>
#include <cstdlib>
#include <openssl/evp.h>
#include <openssl/rand.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
using socket_t=SOCKET;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using socket_t=int;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif
using namespace std;
namespace fs=std::filesystem;
sqlite3* db=nullptr;
string trim(string s){auto a=s.find_first_not_of(" \t\r\n");if(a==string::npos)return "";auto b=s.find_last_not_of(" \t\r\n");return s.substr(a,b-a+1);}
string escapeJson(const string& s){string o;for(unsigned char c:s){if(c=='"')o+="\\\"";else if(c=='\\')o+="\\\\";else if(c=='\n')o+="\\n";else if(c=='\r')o+="\\r";else if(c=='\t')o+="\\t";else if(c<32)o+=' ';else o+=c;}return o;}
string q(const string& s){return "\""+escapeJson(s)+"\"";}
// Flat JSON string values and integers; frontend sends only flat objects.
map<string,string> parseJson(const string& s){map<string,string> m;size_t i=0;auto ws=[&](){while(i<s.size()&&isspace((unsigned char)s[i]))i++;};auto str=[&](){string v;if(i>=s.size()||s[i]!='"')return v;i++;while(i<s.size()){char c=s[i++];if(c=='"')break;if(c=='\\'&&i<s.size()){char e=s[i++];if(e=='n')v+='\n';else if(e=='r')v+='\r';else if(e=='t')v+='\t';else if(e=='u'){if(i+4<=s.size())i+=4;v+='?';}else v+=e;}else v+=c;}return v;};ws();if(i>=s.size()||s[i++]!='{')return m;while(i<s.size()){ws();if(s[i]=='}')break;string k=str();ws();if(i>=s.size()||s[i++]!=':')break;ws();string v;if(i<s.size()&&s[i]=='"')v=str();else {size_t a=i;while(i<s.size()&&s[i]!=','&&s[i]!='}')i++;v=trim(s.substr(a,i-a));}m[k]=v;ws();if(i<s.size()&&s[i]==',')i++;else break;}return m;}
string field(const map<string,string>& m,const string& k){auto it=m.find(k);return it==m.end()?"":it->second;}
string randhex(int n){vector<unsigned char>b(n);if(RAND_bytes(b.data(),n)!=1)throw runtime_error("Random generation failed");ostringstream s;for(auto x:b)s<<hex<<setw(2)<<setfill('0')<<(int)x;return s.str();}
string passwordHash(const string& password,const string& salt){unsigned char out[32];if(!PKCS5_PBKDF2_HMAC(password.c_str(),(int)password.size(),(const unsigned char*)salt.data(),(int)salt.size(),150000,EVP_sha256(),32,out))throw runtime_error("Hash failed");ostringstream ss;for(auto c:out)ss<<hex<<setw(2)<<setfill('0')<<(int)c;return ss.str();}
void sql(const char* s){char* err=nullptr;if(sqlite3_exec(db,s,nullptr,nullptr,&err)!=SQLITE_OK){string msg=err?err:"SQL error";sqlite3_free(err);throw runtime_error(msg);}}
struct Stmt{sqlite3_stmt* s=nullptr;Stmt(const string& sql){if(sqlite3_prepare_v2(db,sql.c_str(),-1,&s,nullptr)!=SQLITE_OK)throw runtime_error(sqlite3_errmsg(db));}~Stmt(){sqlite3_finalize(s);}void bind(int n,const string& v){sqlite3_bind_text(s,n,v.c_str(),-1,SQLITE_TRANSIENT);}void bind(int n,int v){sqlite3_bind_int(s,n,v);}void nullBind(int n){sqlite3_bind_null(s,n);}bool next(){return sqlite3_step(s)==SQLITE_ROW;}bool done(){return sqlite3_step(s)==SQLITE_DONE;}string get(int n){auto x=sqlite3_column_text(s,n);return x?(const char*)x:"";}int num(int n){return sqlite3_column_int(s,n);}};
void createUser(const string& name,const string& surname,const string& email,const string& pass,const string& role,const string& id="",const string& phone="",bool smsOptIn=false){string salt=randhex(16);Stmt s("INSERT INTO users(name,surname,email,password_hash,salt,role,id_number,phone,sms_opt_in) VALUES(?,?,?,?,?,?,?,?,?)");s.bind(1,name);s.bind(2,surname);s.bind(3,email);s.bind(4,passwordHash(pass,salt));s.bind(5,salt);s.bind(6,role);if(id.empty())s.nullBind(7);else s.bind(7,id);if(phone.empty())s.nullBind(8);else s.bind(8,phone);s.bind(9,smsOptIn?1:0);if(!s.done())throw runtime_error(sqlite3_errmsg(db));}
void init(){if(sqlite3_open("clinic.db",&db)!=SQLITE_OK)throw runtime_error("Cannot open clinic.db");sql("PRAGMA foreign_keys=ON;");sql("CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY,name TEXT NOT NULL,surname TEXT NOT NULL,email TEXT NOT NULL UNIQUE,password_hash TEXT NOT NULL,salt TEXT NOT NULL,role TEXT NOT NULL CHECK(role IN ('patient','doctor','nurse','admin')),id_number TEXT UNIQUE,phone TEXT,sms_opt_in INTEGER NOT NULL DEFAULT 0);");if(sqlite3_exec(db,"ALTER TABLE users ADD COLUMN sms_opt_in INTEGER NOT NULL DEFAULT 0",nullptr,nullptr,nullptr)!=SQLITE_OK){/* Existing databases may already contain this column. */}
sql("CREATE TABLE IF NOT EXISTS notifications(id INTEGER PRIMARY KEY AUTOINCREMENT,patient_id INTEGER NOT NULL REFERENCES users(id),appointment_id INTEGER NOT NULL REFERENCES appointments(id),message TEXT NOT NULL,created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,UNIQUE(appointment_id));");sql("CREATE TABLE IF NOT EXISTS sessions(token TEXT PRIMARY KEY,user_id INTEGER NOT NULL REFERENCES users(id),expires_at INTEGER NOT NULL);");sql("CREATE TABLE IF NOT EXISTS slots(id INTEGER PRIMARY KEY,doctor_id INTEGER NOT NULL REFERENCES users(id),date TEXT NOT NULL,time TEXT NOT NULL,UNIQUE(doctor_id,date,time));");sql("CREATE TABLE IF NOT EXISTS appointments(id INTEGER PRIMARY KEY,patient_id INTEGER NOT NULL REFERENCES users(id),slot_id INTEGER NOT NULL UNIQUE REFERENCES slots(id),symptoms TEXT NOT NULL,status TEXT NOT NULL DEFAULT 'Pending' CHECK(status IN ('Pending','Approved','Cancelled','Checked In','Completed')),created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP);");Stmt s("SELECT COUNT(*) FROM users");if(s.next()&&s.num(0)==0){createUser("Sipho","Mthembu","doctor@clinic.test","Doctor123!","doctor");createUser("Thandiwe","Ncube","nurse@clinic.test","Nurse123!","nurse");createUser("Clinic","Administrator","admin@clinic.test","Admin123!","admin");cout<<"Created demonstration staff accounts (see README).\n";}}
string userJson(int id){Stmt s("SELECT id,name,surname,email,role FROM users WHERE id=?");s.bind(1,id);if(!s.next())return "null";return "{\"id\":"+to_string(s.num(0))+",\"name\":"+q(s.get(1)) +",\"surname\":"+q(s.get(2)) +",\"email\":"+q(s.get(3)) +",\"role\":"+q(s.get(4))+"}";}
struct Auth{int id=0;string role;};
Auth auth(const map<string,string>& headers){auto it=headers.find("authorization");if(it==headers.end()||it->second.rfind("Bearer ",0)!=0)return {};Stmt s("SELECT u.id,u.role FROM sessions se JOIN users u ON u.id=se.user_id WHERE se.token=? AND se.expires_at>?");s.bind(1,it->second.substr(7));s.bind(2,(int)time(nullptr));if(s.next())return {s.num(0),s.get(1)};return {};}
string err(const string& e){return "{\"error\":"+q(e)+"}";}
string jsonRows(Stmt& s,const vector<string>& cols){string out="[";bool first=true;while(s.next()){if(!first)out+=",";first=false;out+="{";for(size_t i=0;i<cols.size();i++){if(i)out+=",";out+=q(cols[i])+":"+q(s.get((int)i));}out+="}";}return out+"]";}

// Twilio is configured exclusively through environment variables; never expose credentials to the browser.
string envv(const char* name){const char* x=getenv(name);return x?string(x):"";}
string southAfricaE164(string phone){
 string d;for(char c:phone){if(isdigit((unsigned char)c))d+=c;else if(c!='+'&&c!=' '&&c!='-'&&c!='('&&c!=')')return "";}
 if(d.size()==10&&d[0]=='0')d="27"+d.substr(1);
 if(d.size()==11&&d.rfind("27",0)==0)return "+"+d;
 return "";
}
size_t discardCurl(char* p,size_t size,size_t nmemb,void*){return size*nmemb;}
bool sendTwilioSms(const string& phone,const string& message){
 const string sid=envv("TWILIO_ACCOUNT_SID"),secret=envv("TWILIO_AUTH_TOKEN"),from=envv("TWILIO_FROM_NUMBER");
 if(sid.empty()||secret.empty()||from.empty()){cerr<<"SMS skipped: Twilio environment variables not configured.\n";return false;}
 string to=southAfricaE164(phone);if(to.empty()){cerr<<"SMS skipped: invalid South African phone format.\n";return false;}
 CURL* c=curl_easy_init();if(!c)return false;
 auto esc=[&](const string& v){char* e=curl_easy_escape(c,v.c_str(),(int)v.size());string r=e?e:"";if(e)curl_free(e);return r;};
 string post="To="+esc(to)+"&From="+esc(from)+"&Body="+esc(message);
 string url="https://api.twilio.com/2010-04-01/Accounts/"+sid+"/Messages.json";
 curl_easy_setopt(c,CURLOPT_URL,url.c_str());curl_easy_setopt(c,CURLOPT_USERNAME,sid.c_str());curl_easy_setopt(c,CURLOPT_PASSWORD,secret.c_str());
 curl_easy_setopt(c,CURLOPT_POSTFIELDS,post.c_str());curl_easy_setopt(c,CURLOPT_TIMEOUT,12L);
 curl_easy_setopt(c,CURLOPT_WRITEFUNCTION,discardCurl);curl_easy_setopt(c,CURLOPT_SSL_VERIFYPEER,1L);curl_easy_setopt(c,CURLOPT_SSL_VERIFYHOST,2L);
 CURLcode r=curl_easy_perform(c);long code=0;curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&code);curl_easy_cleanup(c);
 if(r!=CURLE_OK||code<200||code>=300){cerr<<"Twilio SMS request failed (HTTP "<<code<<", curl "<<(int)r<<").\n";return false;}
 cout<<"Twilio accepted an appointment approval SMS request.\n";return true;
}

struct Response{int status=200;string body="{}";string type="application/json";};
Response api(const string& method,const string& path,const string& body,const map<string,string>& headers){auto m=parseJson(body);auto need=[&](string k){return field(m,k);};if(path=="/api/register"&&method=="POST"){
 string name=need("name"),surname=need("surname"),email=need("email"),pass=need("password"),id=need("id_number"),phone=need("phone");
 if(name.empty()||surname.empty()||email.find('@')==string::npos||pass.size()<8||id.size()!=13||!all_of(id.begin(),id.end(),::isdigit)||phone.empty())return {400,err("Check all required fields, including a 13-digit ID and password of at least 8 characters.")};
 if(!regex_search(pass,regex("[A-Za-z]"))||!regex_search(pass,regex("[0-9]"))||!regex_search(pass,regex("[^A-Za-z0-9]")))return {400,err("Password needs a letter, number and special character.")};
 // Self-registration is patient-only; never trust a submitted staff role.
 try{createUser(name,surname,email,pass,"patient",id,phone,need("sms_opt_in")=="on");}catch(...){return {409,err("Email or ID number already registered.")};}return {201,"{\"message\":\"Patient account created\"}"};}
 if(path=="/api/login"&&method=="POST"){
 Stmt s("SELECT id,password_hash,salt,role FROM users WHERE email=?");s.bind(1,need("email"));if(!s.next())return {401,err("Invalid credentials")};int id=s.num(0);string hash=s.get(1),salt=s.get(2),role=s.get(3);if(passwordHash(need("password"),salt)!=hash||(!need("role").empty()&&need("role")!=role))return {401,err("Invalid credentials or user type")};string token=randhex(32);Stmt ins("INSERT INTO sessions(token,user_id,expires_at) VALUES(?,?,?)");ins.bind(1,token);ins.bind(2,id);ins.bind(3,(int)time(nullptr)+86400);if(!ins.done())throw runtime_error("Session error");return {200,"{\"token\":"+q(token)+",\"user\":"+userJson(id)+"}"};}
 Auth a=auth(headers);if(!a.id)return {401,err("Please sign in")};
 if(path=="/api/me"&&method=="GET")return {200,userJson(a.id)};
 if(path=="/api/logout"&&method=="POST"){auto it=headers.find("authorization");if(it!=headers.end()){Stmt s("DELETE FROM sessions WHERE token=?");s.bind(1,it->second.substr(7));s.done();}return {200,"{}"};}
 if(path=="/api/slots"&&method=="GET"){
 // Create demo availability for next 21 days (08:00-16:30, every 30 minutes).
 Stmt d("SELECT id FROM users WHERE role='doctor' LIMIT 1");if(!d.next())return {200,"[]"};int doctor=d.num(0);time_t now=time(nullptr);for(int day=0;day<21;day++){time_t t=now+day*86400;tm* v=localtime(&t);if(v->tm_wday==0||v->tm_wday==6)continue;char date[16];strftime(date,sizeof(date),"%Y-%m-%d",v);for(int half=16;half<=33;half++){char hh[8];snprintf(hh,sizeof(hh),"%02d:%02d",half/2,(half%2)*30);Stmt ins("INSERT OR IGNORE INTO slots(doctor_id,date,time) VALUES(?,?,?)");ins.bind(1,doctor);ins.bind(2,string(date));ins.bind(3,string(hh));ins.done();}}
 Stmt s("SELECT s.id,s.date,s.time,u.name||' '||u.surname FROM slots s JOIN users u ON u.id=s.doctor_id LEFT JOIN appointments a ON a.slot_id=s.id AND a.status!='Cancelled' WHERE datetime(s.date||' '||s.time)>datetime('now','localtime') AND a.id IS NULL ORDER BY s.date,s.time LIMIT 500");return {200,jsonRows(s,{"id","date","time","doctor"})};}
 if(path=="/api/notifications"&&method=="GET"){if(a.role!="patient")return {403,err("Patient only")};Stmt n("SELECT id,message,created_at FROM notifications WHERE patient_id=? ORDER BY id DESC LIMIT 30");n.bind(1,a.id);return {200,jsonRows(n,{"id","message","created_at"})};}
 if(path=="/api/appointments"&&method=="GET"){
 string query="SELECT a.id,s.date,s.time,p.name||' '||p.surname,d.name||' '||d.surname,a.symptoms,a.status FROM appointments a JOIN slots s ON s.id=a.slot_id JOIN users p ON p.id=a.patient_id JOIN users d ON d.id=s.doctor_id";
 if(a.role=="patient")query+=" WHERE a.patient_id=?";else if(a.role=="doctor")query+=" WHERE s.doctor_id=?";
 query+=" ORDER BY s.date DESC,s.time DESC";Stmt s(query);if(a.role=="patient"||a.role=="doctor")s.bind(1,a.id);return {200,jsonRows(s,{"id","date","time","patient","doctor","symptoms","status"})};}
 if(path=="/api/appointments"&&method=="POST"){
 if(a.role!="patient")return {403,err("Patient account required")};string symptoms=trim(need("symptoms"));if(symptoms.empty()||symptoms.size()>500)return {400,err("Enter symptoms (1-500 characters)")};int slot=0;try{slot=stoi(need("slot_id"));}catch(...){return {400,err("Select a slot")};}
 Stmt s("INSERT INTO appointments(patient_id,slot_id,symptoms) SELECT ?,s.id,? FROM slots s WHERE s.id=? AND datetime(s.date||' '||s.time)>datetime('now','localtime') AND NOT EXISTS(SELECT 1 FROM appointments a WHERE a.slot_id=s.id)");s.bind(1,a.id);s.bind(2,symptoms);s.bind(3,slot);if(!s.done()||sqlite3_changes(db)!=1)return {409,err("Slot unavailable; refresh and choose another")};return {201,"{\"message\":\"Appointment requested\"}"};}
 if(path=="/api/status"&&method=="POST"){
 string status=need("status");if(a.role=="doctor"&&status!="Approved"&&status!="Cancelled"&&status!="Completed")return {403,err("Doctor may approve, cancel or complete")};if(a.role=="nurse"&&status!="Checked In")return {403,err("Nurse may check in patients")};if(a.role!="doctor"&&a.role!="nurse")return {403,err("Staff only")};int id=0;try{id=stoi(need("id"));}catch(...){return {400,err("Invalid appointment")};}
 string query="UPDATE appointments SET status=? WHERE id=? AND status IN (";
 if(a.role=="doctor")query+=(status=="Completed"?"'Approved','Checked In'":"'Pending'");else query+="'Approved'";
 query+=")";
 if(a.role=="doctor")query+=" AND slot_id IN (SELECT id FROM slots WHERE doctor_id=?)";
 Stmt s(query);s.bind(1,status);s.bind(2,id);if(a.role=="doctor")s.bind(3,a.id);if(!s.done()||sqlite3_changes(db)!=1)return {409,err("Action not allowed for this appointment status")};
 // Store a simulated notification for the patient after doctor approval.
 // The UNIQUE(appointment_id) constraint prevents duplicate approval notifications.
 if(status=="Approved"){
   Stmt info("SELECT ap.patient_id,sl.date,sl.time FROM appointments ap JOIN slots sl ON sl.id=ap.slot_id WHERE ap.id=?");
   info.bind(1,id);
   if(info.next()){
     string message="Your appointment on "+info.get(1)+" at "+info.get(2)+" has been approved by the doctor.";
     Stmt n("INSERT OR IGNORE INTO notifications(patient_id,appointment_id,message) VALUES(?,?,?)");
     n.bind(1,info.num(0));n.bind(2,id);n.bind(3,message);
     if(!n.done())cerr<<"Notification storage failed: "<<sqlite3_errmsg(db)<<"\n";
   }
 }
 return {200,"{\"message\":\"Status updated\"}"};}
 if(path=="/api/users"&&method=="GET"&&a.role=="admin"){Stmt s("SELECT id,name,surname,email,role FROM users ORDER BY id");return {200,jsonRows(s,{"id","name","surname","email","role"})};}
 return {404,err("Not found")};}
string mime(string p){if(p.size()>=4&&p.substr(p.size()-4)==".css")return "text/css";if(p.size()>=3&&p.substr(p.size()-3)==".js")return "application/javascript";if(p.size()>=4&&p.substr(p.size()-4)==".svg")return "image/svg+xml";if(p.size()>=4&&p.substr(p.size()-4)==".png")return "image/png";return "text/html";}
void closeSocket(socket_t s){
#ifdef _WIN32
 closesocket(s);
#else
 close(s);
#endif
}
void serve(socket_t client){try{string request;char buf[8192];size_t headerEnd=string::npos;while(request.size()<100000&&(headerEnd=request.find("\r\n\r\n"))==string::npos){int n=recv(client,buf,sizeof(buf),0);if(n<=0)return;request.append(buf,n);}if(headerEnd==string::npos)return;string head=request.substr(0,headerEnd);istringstream hs(head);string method,path,version;hs>>method>>path>>version;map<string,string> headers;string line;getline(hs,line);while(getline(hs,line)){line=trim(line);auto pos=line.find(':');if(pos!=string::npos){string k=line.substr(0,pos);transform(k.begin(),k.end(),k.begin(),::tolower);headers[k]=trim(line.substr(pos+1));}}int len=0;if(headers.count("content-length"))len=stoi(headers["content-length"]);if(len>65536||len<0)return;string body=request.substr(headerEnd+4);while((int)body.size()<len){int n=recv(client,buf,min((int)sizeof(buf),len-(int)body.size()),0);if(n<=0)return;body.append(buf,n);}auto qm=path.find('?');if(qm!=string::npos)path=path.substr(0,qm);Response r;
 if(path.rfind("/api/",0)==0)r=api(method,path,body,headers);
 else {if(path=="/")path="/index.html";if(path.find("..")!=string::npos||path.find('\\')!=string::npos){r={403,"Forbidden","text/plain"};}else{fs::path f=fs::path("public")/path.substr(1);ifstream in(f,ios::binary);if(!in)r={404,"Not found","text/plain"};else{ostringstream out;out<<in.rdbuf();r={200,out.str(),mime(path)};}}}
 string status=r.status==200?"OK":r.status==201?"Created":r.status==400?"Bad Request":r.status==401?"Unauthorized":r.status==403?"Forbidden":r.status==409?"Conflict":"Not Found";string h="HTTP/1.1 "+to_string(r.status)+" "+status+"\r\nContent-Type: "+r.type+"; charset=utf-8\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nContent-Length: "+to_string(r.body.size())+"\r\nConnection: close\r\n\r\n";string out=h+r.body;size_t off=0;while(off<out.size()){int n=send(client,out.data()+off,(int)min((size_t)16000,out.size()-off),0);if(n<=0)break;off+=n;}}
 catch(const exception& e){cerr<<"Request error: "<<e.what()<<"\n";}}
int main(){try{curl_global_init(CURL_GLOBAL_DEFAULT);init();
#ifdef _WIN32
 WSADATA wsa;if(WSAStartup(MAKEWORD(2,2),&wsa))throw runtime_error("Winsock startup failed");
#endif
 socket_t server=socket(AF_INET,SOCK_STREAM,0);if(server==INVALID_SOCKET)throw runtime_error("Socket failed");sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(8765);addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);int yes=1;setsockopt(server,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes));if(bind(server,(sockaddr*)&addr,sizeof(addr))==SOCKET_ERROR)throw runtime_error("Port 8765 unavailable");if(listen(server,16)==SOCKET_ERROR)throw runtime_error("Listen failed");cout<<"Open http://127.0.0.1:8765 in your browser (Ctrl+C to stop)\n";while(true){socket_t c=accept(server,nullptr,nullptr);if(c!=INVALID_SOCKET){serve(c);closeSocket(c);}}}catch(const exception& e){cerr<<"Error: "<<e.what()<<"\n";return 1;}}
