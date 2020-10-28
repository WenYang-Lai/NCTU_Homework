#include <windows.h>
#include <winsock2.h>
#include <wingdi.h>
#include <bits/stdc++.h>

using namespace std;

#include "resource.h"

#define SERVER_PORT 5487

#define WM_SOCKET_SERVER (WM_USER + 1)
#define WM_SOCKET_CLIENT (WM_USER + 2)
#define WM_SOCKET_CGI    (WM_USER + 3)

#define BUFFER_SIZE 65536

#define PARSE_KEY 0
#define PARSE_VALUE 1

struct client_data{
    SOCKET cgi;
    string buffer;
    int server_num;
    FILE* batch_file;
    client_data(){}
};
map<SOCKET, client_data> sockets;

using namespace std;
LRESULT CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
void close_socket(const SOCKET& fd);
void parse_query(string& str, map<string, string> &env);
//=================================================================
//	Global Variables
//=================================================================


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainDlgProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_EXCLAMATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)BLACK_BRUSH;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "np winsock";
    RegisterClass(&wc);
    HWND hwnd = CreateWindow("np winsock", "", WS_SYSMENU, 300, 0, 600, 400, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL){
        MessageBox(hwnd, "create windows failed", "failed", MB_OK);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    WSADATA wsaData;
    WORD version = MAKEWORD(2,0);
    if (WSAStartup(version, &wsaData)!=0){
        MessageBox(NULL, "WSAStartup() Failed", "", 0);
        return 1;
    }
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET){
        MessageBox(NULL, "socket() Failed", "", 0);
        return 1;
    }
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(5487);
    sin.sin_addr.s_addr = INADDR_ANY;
    if (bind(s, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR){
        MessageBox(NULL, "bind() Failed", "", 0);
        return 1;
    }
    if (listen(s, 10) == SOCKET_ERROR){
        MessageBox(NULL, "listen() Failed", "", 0);
        return 1;
    }
	MessageBox(hwnd, "start listen", "", MB_OK);
    WSAAsyncSelect(s, hwnd, WM_SOCKET_SERVER, FD_ACCEPT | FD_CLOSE);
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    closesocket(s);
    WSACleanup();
    return msg.wParam;
}

LRESULT CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    if (Message == WM_SOCKET_SERVER)
    {
        SOCKET sockfd = wParam;
        if(WSAGETSELECTERROR(lParam))
        {
            closesocket(sockfd);
            return 0;
        }
        int event = WSAGETSELECTEVENT(lParam);
        if (event == FD_ACCEPT)
        {
            sockaddr_in client_addr;
            int len = sizeof(client_addr);
            SOCKET client_fd;
            client_fd = accept(sockfd, (sockaddr*)&client_addr, &len);
            if (client_fd == INVALID_SOCKET)
            {
                cout << "Accept failed!" << endl;
                return 0;
            }
            else
            {
                sockets[client_fd] = client_data();
                WSAAsyncSelect(client_fd, hwnd, WM_SOCKET_CGI, FD_READ | FD_CLOSE);
            }
            //WSAAsyncSelect(wParam, hwnd, WM_SOCKET_SERVER, FD_ACCEPT | FD_CLOSE);
        }
        else if (event == FD_CLOSE)
            shutdown(sockfd, SD_BOTH);
    }
    else if (Message == WM_SOCKET_CGI)
    {
        SOCKET sockfd = wParam;
        if(WSAGETSELECTERROR(lParam))
        {
            closesocket(sockfd);
            return 0;
        }
        int event = WSAGETSELECTEVENT(lParam);
        if (event == FD_READ)
        {
            char buffer[BUFFER_SIZE] = {};
            recv(sockfd, buffer, sizeof(buffer), 0);
            stringstream ss(buffer);
            string query_method, query_string, url, file_path, file_type;
            ss >> query_method >> url;
            if (url.size() > 1)
                file_path = url.substr(1, url.find_last_of('?')-1);
            if (url.find('?') != string::npos)
                query_string = url.substr(url.find_last_of('?')+1, url.size()-url.find_last_of('?'));
            while(query_string.find("%2F") != string::npos)
                query_string.replace(query_string.find("%2F"), 3, "/");
            if (file_path.find('.') != string::npos)
                file_type = file_path.substr(file_path.find_last_of('.')+1);
            sockets[sockfd].buffer += string("HTTP/1.1 200 OK\nContent-type: text/html\n\n");   // response status and header
            if (file_type == "cgi")
            {
                sockets[sockfd].buffer += string("<html>\n");
                sockets[sockfd].buffer += string("<head>\n");
                sockets[sockfd].buffer += string("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n");
                sockets[sockfd].buffer += string("<title>Network Programming Homework 3</title>\n");
                sockets[sockfd].buffer += string("</head>\n");
                sockets[sockfd].buffer += string("<body id='body' bgcolor=#336699>\n");
                sockets[sockfd].buffer += string("<font face=\"Courier New\" size=2 color=#FFFF99>\n");
                sockets[sockfd].buffer += string("<table width=\"800\" border=\"1\">\n");
                sockets[sockfd].buffer += string("<tr>\n");
                map<string, string> env;
                parse_query(query_string, env);  // parse query_string;
                for (int i=1;i<=5;i++)
                {
                    string host = "h"; host += ('0'+i);
                    if (env.find(host) != env.end())
                        sockets[sockfd].buffer += string(string("<td>") + env[string(host)] + string("</td>\n"));
                }
                sockets[sockfd].buffer += string("</tr>\n");
                sockets[sockfd].buffer += string("<tr>\n");
                for (int i=1;i<=5;i++)
                {
                    string host = "h"; host += ('0'+i);
                    string id =""; id += ('0'+i);
                    if (env.find(host) != env.end())
                        sockets[sockfd].buffer += string(string("<td valign=\"top\" id=\"m") + id + string("\"></td>\n"));
                }
                sockets[sockfd].buffer += string("</tr>\n");
                sockets[sockfd].buffer += string("</table>\n</body>\n");
                sockets[sockfd].buffer += string("</font>\n");
                sockets[sockfd].buffer += string("</html>\n");

                sockets[sockfd].server_num = 0;
                for (int i=1;i<=5;i++)
                {
                    string host = "h"; host += ('0'+i);
                    if(env.find(host) != env.end())
                    {
                        sockets[sockfd].server_num++;
                        string port = "p"; port += ('0'+i);
                        string file_name = "f"; file_name += ('0'+i);
                        SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
                        sockaddr_in client_sin;
                        client_sin.sin_family = AF_INET;
                        client_sin.sin_addr = *((in_addr*)gethostbyname(env[host].c_str())->h_addr);
                        client_sin.sin_port = htons(atoi(env[port].c_str()));
                        /* set non-blocking connect */
                        u_long val = 1;
                        ioctlsocket(client, FIONBIO, &val);
                        connect(client, (sockaddr*)&client_sin, sizeof(client_sin));

                        sockets[client] = client_data();
                        sockets[client].batch_file = fopen(env[file_name].c_str(), "r");
                        sockets[client].cgi = sockfd;
                        sockets[client].server_num = i;
                        WSAAsyncSelect(client, hwnd, WM_SOCKET_CLIENT, FD_CONNECT | FD_CLOSE);
                    }
                }
            }
            else
            {
                ifstream fin(file_path.c_str());
                string str;
                while (getline(fin, str))
                    sockets[sockfd].buffer += str;
                fin.close();
            }
            WSAAsyncSelect(wParam, hwnd, WM_SOCKET_CGI, FD_WRITE);
        }
        else if (event == FD_WRITE)
        {
            if (sockets[sockfd].buffer.size())
            {
                int n = send(sockfd, sockets[sockfd].buffer.c_str(), sockets[sockfd].buffer.size(), 0);
                if (n>=0)
                    sockets[sockfd].buffer = sockets[sockfd].buffer.substr(n, sockets[sockfd].buffer.size()-n);
            }
            if (sockets[sockfd].buffer.empty() && sockets[sockfd].server_num == 0)
            {
                WSAAsyncSelect(wParam, hwnd, WM_SOCKET_CGI, FD_CLOSE);
                PostMessage(hwnd, WM_SOCKET_CGI, wParam, FD_CLOSE);
            }
            else
                PostMessage(hwnd, WM_SOCKET_CGI, wParam, FD_WRITE);
        }
        else if (event == FD_CLOSE)
        {
            close_socket(sockfd);
        }
    }
    else if (Message == WM_SOCKET_CLIENT)
    {
        SOCKET sockfd = wParam;
        if(WSAGETSELECTERROR(lParam))
        {
            closesocket(sockfd);
            return 0;
        }
        int event = WSAGETSELECTEVENT(lParam);
        if (event == FD_READ)
        {
            char buffer[BUFFER_SIZE];
            int send_command = 0;
            int n = recv(sockfd, buffer , sizeof(buffer), 0);
            string &send_buffer = sockets[sockets[sockfd].cgi].buffer;
            int id = sockets[sockfd].server_num;
            send_buffer += string("<script>document.all['m");
            send_buffer += (id+'0');
            send_buffer += string("'].innerHTML += \"");
            for (int i=0;i<n;i++)
            {
                if(buffer[i]=='%') send_command = 1;
                if(buffer[i]=='\n') send_buffer += string("<br>");
                else if(buffer[i]=='\r') continue;
                else if(buffer[i] == '<') send_buffer += string("&lt;");
                else if(buffer[i] == '>') send_buffer += string("&gt;");
                else if(buffer[i] == '&') send_buffer += string("&amp;");
                else if(buffer[i] == '"') send_buffer += string("&quot;");
                else if(buffer[i] == ' ' || buffer[i] == '\t') send_buffer += string("&nbsp;");
                else send_buffer += buffer[i];
            }
            send_buffer += string("\"</script>\n");
            if (send_command)
            {
                WSAAsyncSelect(wParam, hwnd, WM_SOCKET_CLIENT, FD_WRITE | FD_CLOSE);
                fgets(buffer, BUFFER_SIZE, sockets[sockfd].batch_file );
                /* send message to server */
                sockets[sockfd].buffer += string(buffer);
                /* write to browser */
                send_buffer += string("<script>document.all['m");
                send_buffer += (id+'0');
                send_buffer += string("'].innerHTML += \"<b>");
                for (int i=0; buffer[i]!='\0';i++)
                {
                    if(buffer[i]=='%') send_command = 1;
                    if(buffer[i]=='\n') send_buffer += string("<br>");
                    else if(buffer[i]=='\r') continue;
                    else if(buffer[i] == '<') send_buffer += string("&lt;");
                    else if(buffer[i] == '>') send_buffer += string("&gt;");
                    else if(buffer[i] == '&') send_buffer += string("&amp;");
                    else if(buffer[i] == '"') send_buffer += string("&quot;");
                    else if(buffer[i] == ' ' || buffer[i] == '\t') send_buffer += string("&nbsp;");
                    else send_buffer += buffer[i];
                }
                send_buffer += string("</b>\"</script>\n");
            }
        }
        else if (event == FD_WRITE)
        {
            if (sockets[sockfd].buffer.size())
            {
                int n = send(sockfd, sockets[sockfd].buffer.c_str(), sockets[sockfd].buffer.size(), 0);
                if (n>=0)
                    sockets[sockfd].buffer = sockets[sockfd].buffer.substr(n, sockets[sockfd].buffer.size()-n);
                WSAAsyncSelect(wParam, hwnd, WM_SOCKET_CLIENT, FD_READ | FD_CLOSE);
            }
            else
                PostMessage(hwnd, WM_SOCKET_CLIENT, wParam, FD_CLOSE);

        }
        else if (event == FD_CONNECT)
        {
            cout << "Connect!" << endl;
            WSAAsyncSelect(wParam, hwnd, WM_SOCKET_CLIENT, FD_READ | FD_CLOSE);
        }
        else if (event == FD_CLOSE)
        {
            sockets[sockets[sockfd].cgi].server_num -- ;
            close_socket(sockfd);
        }
    }
    else if(Message == WM_CLOSE)
    {
        if (MessageBox(hwnd, "exit?", "message", MB_YESNO) == IDYES)
            DestroyWindow(hwnd);
    }
    else if (Message == WM_DESTROY)
        PostQuitMessage(0);
    else
        return DefWindowProc(hwnd, Message, wParam, lParam);

}

void close_socket(const SOCKET& fd)
{
    sockets.erase(fd);
    shutdown(fd, SD_BOTH);
}

void parse_query(string& str, map<string, string> &env)
{
    string key="", value="";
    int status = PARSE_KEY;
    for (int i=0;i<str.length();i++)
    {
        if(status == PARSE_KEY)  //parse key
        {
            if(str[i]=='=')
                status = PARSE_VALUE;
            else
                key += str[i];
        }
        else                    // parse value
        {
            if (str[i]=='&')
            {
                if (value != "")
                    env[key] = value;
                status = PARSE_KEY;
                key = "";
                value = "";
            }
            else
            {
                value += str[i];
                if (i+1 == str.length())
                    env[key] = value;
            }
        }
    }
}
