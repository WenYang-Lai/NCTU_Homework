#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;
struct LIST;
struct PAGE;

struct PAGE
{
  char addr[6];
  PAGE* next;
  PAGE* previous;

  PAGE(){next=NULL; previous=NULL;}
};

struct LIST
{
  int size;
  PAGE *head;
  PAGE *end;
  LIST(){size = 0; head = NULL; end = NULL;}
  void erase(PAGE* page)
  { 
    if (page != head)
      page->previous->next = page->next;
    else
      head = page->next;
    if (page != end)
      page->next->previous = page->previous;
    else
      end = page->previous;
    size--;
  }
  PAGE* begin()
  {
    return head;
  }
  void push_back(PAGE* page)
  {
    if (size == 0)
    { 
      end = page;
      head = page;
    }
    else
    {
      end->next = page;
      page->previous = end;
      page->next = NULL;
      end = page;
    }
    size++;
  }
};



int main()
{
  /* FIFO part */
  cout << "FIFO---" << endl;
  cout << "size\tmiss\thit\tpage fault ratio" << endl;
  for(int frame = 64;frame<=512; frame*=2)
  { 
    ifstream file = ifstream("trace.txt", fstream::in);
    string s, str;
    LIST list;
    int miss = 0, hit = 0;
    map<string, PAGE* > table;
    while (file >> s >> str)
    {
      str = str.substr(0, 5);
      if (table.find(str) == table.end())
      { 
        miss += 1;
        if(list.size == frame)
        { 
           PAGE *it = list.begin();
           table.erase(string(it->addr));
           list.erase(it);
           delete it;
        }
        PAGE *node = new PAGE;
        strcpy(node->addr, str.c_str());
        list.push_back(node);
        table[str] = node; 
      }
      else
      {
        hit += 1;
      }
    }
    cout << frame << "\t" << miss << "\t" << hit << "\t" << (double)miss/(miss+hit) << endl;
    file.close();
  }

  /* LRU part */
  cout << "LRU---" << endl;
  cout << "size\tmiss\thit\tpage fault ratio" << endl;
  for(int frame = 64;frame<=512; frame*=2)
  { 
    ifstream file = ifstream("trace.txt", fstream::in);
    string s, str;
    LIST list;
    int miss = 0, hit = 0;
    map<string, PAGE* > table;
    while (file >> s >> str)
    {
      str = str.substr(0, 5);
      if (table.find(str) == table.end())
      { 
        miss += 1;
        if(list.size == frame)
        { 
           PAGE *it = list.begin();
           table.erase(string(it->addr));
           list.erase(it);
           delete it;
        }
        PAGE *node = new PAGE;
        strcpy(node->addr, str.c_str());
        list.push_back(node);
        table[str] = node; 
      }
      else
      { 
        PAGE* it = table[str];
        list.erase(it);
        list.push_back(it);
        hit += 1;
      }
    }
    cout << frame << "\t" << miss << "\t" << hit << "\t" << (double)miss/(miss+hit) << endl;
    file.close();
  }
}
