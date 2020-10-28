#include "cgi.h"

void parse_query_string(char *str)
{ 
  string key="", value="";
  int status = PARSE_KEY;
  for (int i=0;str[i]!='\0';i++)
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
          parameters[key] = value;
        fprintf(stderr, "KEY: %s, VALUE: %s\n",key.c_str(), value.c_str());
        status = PARSE_KEY;
        key = "";
        value = "";
      }
      else
      { 
        value += str[i];
        if (str[i+1] == '\0')
          parameters[key] = value;
      }
    }
  }
}

/* connect to specified server */
void connect_server(int i)  
{ 
  fprintf(stderr, "Entering Connect to server\n");
  /* get parameters inder */
  string host = "h", port = "p", file = "f", s_host = "sh", s_port = "sp";
  host += (i+'0'); port += (i+'0'); file += (i+'0'); s_host += (i+'0'); s_port += (i+'0');
  /* set TCP connection */
  CLIENT *client = &(clients[i-1]);
  client->client_sin.sin_family = AF_INET;
  client->client_sin.sin_addr = *((struct in_addr *)(gethostbyname(parameters[s_host].c_str())->h_addr));
  client->client_sin.sin_port = htons(atoi(parameters[s_port].c_str()));
  client->client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (connect(client->client_fd, (struct sockaddr*)&(client->client_sin), sizeof(clients->client_sin)) < 0 )
  {
    fprintf(stderr, "Connecting!\n");
    if (errno != EINPROGRESS)
    {
      fprintf(stderr, "Can't connect to \"%s\"\n", parameters[host].c_str());
      fprintf(stderr, "error: %s\n", strerror(errno));
    }
    else
    {
      if (client->client_fd >= nfds) nfds = client->client_fd +1;
      client->status = S_CONNECTING;
      FD_SET(client->client_fd, &rfds);
      FD_SET(client->client_fd, &wfds);
      if ( (client->file = fopen(parameters[file].c_str(), "r")) < 0) 
        fprintf(stderr, "can't open file : %s\n", parameters[file].c_str());
      connect_num++;
    }
  }
  else
  {
    fprintf(stderr, "Connecting complete!\n");
    if (client->client_fd >= nfds) nfds = client->client_fd +1;
    FD_SET(client->client_fd, &rfds);
    FD_SET(client->client_fd, &wfds);
    client->status = S_PROXY;
    client->file = fopen(parameters[file].c_str(), "r");
    connect_num += 1;
    unsigned char request[128];
    in_addr client_addr = *((struct in_addr *)(gethostbyname(parameters[host].c_str())->h_addr));
    request[0] = 4;
    request[1] = 1;
    request[2] = atoi(parameters[port].c_str())/256;
    request[3] = atoi(parameters[port].c_str())%256;
    request[7] = (client_addr.s_addr >> 24)%256;
    request[6] = (client_addr.s_addr >> 16)%256;
    request[5] = (client_addr.s_addr >> 8)%256;
    request[4] = client_addr.s_addr%256 ;
    fprintf(stderr, "%d.%d.%d.%d\n", request[4], request[5], request[6], request[7]);
    request[8] = 0;
    write(client->client_fd, request, 9);
  }
}

void response_basic_information()
{ 
  //fprintf(stderr, "Debug_message\n");
  printf("Content-type: text/html\n\n");

  printf("<html>\n");
  printf("<head>\n");
  printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n");
  printf("<title>Network Programming Homework 3</title>\n");
  printf("</head>\n");
  printf("<body bgcolor=#336699>\n");
  printf("<font face=\"Courier New\" size=2 color=#FFFF99>\n");
  printf("<table width=\"800\" border=\"1\">\n");
  printf("<tr>\n"); 
  for (int i=0;i<5;i++)
    printf("<td>%s</td>", inet_ntoa(clients[i].client_sin.sin_addr));
  printf("</tr>\n");
  printf("<tr>\n"); 
  for (int i=0;i<5;i++)
    printf("<td valign=\"top\" id=\"m%d\">", i);
  printf("</tr>\n");
  printf("</table>");
  printf("</font>\n");
  printf("</body>\n");
  printf("</html>\n");
  fflush(stdout);
}

void string_shift_right(char *str, int s, int& e)
{
  for (int i=e;i>=s;i--)
    str[i+1] = str[i];
  e++;
}
void escaping_string(char *str)
{ 
  int len = strlen(str);
  for (int i=0;i<len;i++)
  {
    if (str[i] == ' ')
    {
      for (int j=0;j<5;j++)
        string_shift_right(str, i+1, len); 
      str[i] = '&';  str[i+1] = 'n';  str[i+2] = 'b';  str[i+3] = 's';  str[i+4] = 'p';  str[i+5] = ';';
    }
    else if (str[i]=='"')
    {
      for (int j=0;j<5;j++)
        string_shift_right(str, i+1, len);
      str[i] = '&';  str[i+1] = 'q';  str[i+2] = 'u';  str[i+3] = 'o';  str[i+4] = 't';  str[i+5] = ';';
    }
    else if(str[i] == '&')
    {
      for (int j=0;j<4;j++)
        string_shift_right(str, i+1, len);
      str[i] = '&';  str[i+1] = 'a';  str[i+2] = 'm';  str[i+3] = 'p';  str[i+4] = ';'; 
    }
    else if(str[i] == '<')
    {
      for (int j=0;j<3;j++)
        string_shift_right(str, i+1, len);
      str[i] = '&';  str[i+1] = 'l';  str[i+2] = 't';  str[i+3] = ';';  
    }
    else if (str[i] == '>')
    {
      for (int j=0;j<3;j++)
        string_shift_right(str, i+1, len);
      str[i] = '&';  str[i+1] = 'g';  str[i+2] = 't';  str[i+3] = ';';  
    }
    else if (str[i] == '\n')
    {
      for (int j=0;j<3;j++)
        string_shift_right(str, i+1, len);
      str[i] = '<';  str[i+1] = 'b';  str[i+2] = 'r';  str[i+3] = '>';  
      i = i+3;
    }
    else if (str[i] == '\r')
      str[i] = ' ';
  }
}
int main()
{ 
  memset(clients, 0, sizeof(CLIENT)*5);
  char *query_string = getenv("QUERY_STRING");
  parse_query_string(query_string);
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  
  /* check whether have input and connect */
  if (parameters.find("h1") != parameters.end())
    connect_server(1);
  if (parameters.find("h2") != parameters.end())
    connect_server(2);
  if (parameters.find("h3") != parameters.end())
    connect_server(3);
  if (parameters.find("h4") != parameters.end())
    connect_server(4);
  if (parameters.find("h5") != parameters.end())
    connect_server(5);
  
  response_basic_information();
  while (connect_num)
  {
    bcopy((char*)&rfds, (char*)&crfds, sizeof(rfds));
    bcopy((char*)&wfds, (char*)&cwfds, sizeof(wfds));
    
    if (select(nfds, &crfds, &cwfds, (fd_set*)0, (struct timeval*)0) < 0)
    {
      if (errno == EINTR)
        continue;
      perror("select error");
      exit(1);
    }
    else
    {
      for (int i=0;i<5;i++)
      {
        if(clients[i].status != S_NOUSE)
        { 
          int sockfd = clients[i].client_fd;
          if(clients[i].status == S_CONNECTING && (FD_ISSET(sockfd, &crfds) || FD_ISSET(sockfd, &cwfds)))
          { 
            int error; socklen_t n = sizeof(sockfd);
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &n) < 0 || error != 0)
            {
              fprintf(stderr, "Connect host %d failed", i);   //connection refuse
              FD_CLR(sockfd, &rfds);
              FD_CLR(sockfd, &wfds);
              connect_num--;
            }
            else
            { 
              clients[i].status = S_DONE;
              fprintf(stderr, "Delay Connecting Complete\n");
            }
          }
          else if (clients[i].status == S_PROXY && FD_ISSET(sockfd, &crfds))
          { 
            char buffer[65536];
            int len = read(sockfd, buffer, 8);
            if (len > 0)
            {
              if(buffer[1] == 90)
              {   
                  fprintf(stderr, "proxy accepy\n");
                  clients[i].status = S_DONE;
                  fcntl(clients[i].client_fd, F_SETFL, O_NONBLOCK);
              }
              else
              {
                  fprintf(stderr, "proxy refused\n");
                  FD_CLR(sockfd, &rfds);
                  FD_CLR(sockfd, &wfds);
                  connect_num--;
                  close(sockfd);
              }
            }
            else if (len == 0)
            {
              fprintf(stderr, "Connect to host : %d is closed by foreign\n", i);
              FD_CLR(sockfd, &rfds);
              FD_CLR(sockfd, &wfds);
              connect_num--;
              close(sockfd);
            }
          }
          else if (clients[i].status == S_DONE && FD_ISSET(sockfd, &crfds))
          {
            char buffer[65537];
            int len = read(sockfd, buffer, sizeof(buffer));
            if (len > 0)
            {
              for (int j=0;j<len;j++)
                if(buffer[j]== '%')
                  clients[i].send_message = 1;
              
              buffer[len] = 0;
              fprintf(stderr, "data: %s\n",buffer);  // check receive what data
              escaping_string(buffer);
              printf("<script>document.all['m%d'].innerHTML += \"%s\";</script>\n", i, buffer);
              fflush(stdout);
            }
            else if (len == 0)
            { 
              /*  connection interrupt */
              if (len == 0)
                fprintf(stderr, "Connect to host : %d is closed by foreign\n", i);
              else
                fprintf(stderr, "Connect to host : %d is closed unexpectly\n", i);
              FD_CLR(sockfd, &rfds);
              FD_CLR(sockfd, &wfds);
              connect_num--;
              close(sockfd);
            }
          }
          if (clients[i].send_message == 1 && FD_ISSET(sockfd, &cwfds))  // need to send message
          { 
            if (clients[i].sent_num == strlen(clients[i].buffer))
            {
              fgets(clients[i].buffer, sizeof(clients[i].buffer), clients[i].file);
              clients[i].sent_num = 0;
            }
            /* send to server */
            int n = write(clients[i].client_fd, clients[i].buffer + clients[i].sent_num, strlen(clients[i].buffer) - clients[i].sent_num);
            clients[i].sent_num += n;
            if (n < 0)  
            {
              fprintf(stderr, "Write to host : %d crashed!\n", i);
              FD_CLR(clients[i].client_fd, &rfds);
              FD_CLR(clients[i].client_fd, &wfds);
              close(clients[i].client_fd);
            }
            else if (clients[i].sent_num == strlen(clients[i].buffer)) // send to browser
            {
              char buffer[65536];
              strcpy(buffer, clients[i].buffer);
              escaping_string(buffer);
              printf("<script>document.all['m%d'].innerHTML += \"<b>%s</b>\";</script>\n", i, buffer);
              fflush(stdout);
              clients[i].send_message = 0;
            }
          }
        }
      }
    }
  }
  fprintf(stderr, "ENDCGI\n");
}
