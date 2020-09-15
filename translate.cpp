#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <pqxx/pqxx>

#define LL long long
#define p(a, b) (cout << a << ": " << b << endl)
#define rm(v, a) (v.erase(find(v.begin(), v.end(), a)))
#define ascend(v) (sort(v.begin(), v.end()))
#define descend(v, t) (sort(v.begin(), v.end(), greater<t>()))
#define MAXBUFFER 10000000

using namespace std;

int main(int argc, char *argv[])
{
  // User argument checks
  if (argc < 5)
  {
    printf("Call as %s <site_to_translate.html> <user> <pass> <pairs>\n", argv[0]);
    printf("\t\tsite_to_translate.html: The page containing acronyms to translate.\n");
    printf("\t\tuser : The username for PostgreSQL\n");
    printf("\t\tpass : The password for PostgreSQL\n");
    printf("\t\tpairs: Acronym pairs (e.g. \"API:ApplicationProgrammingInterface\"\n");
    return 1;
  }

  // Readying acronyms for PostgreSQL
  map<string, string> acr_pairs;
  for (int i = argc - 1; i > 3; --i)
  {
    char key[10];
    char val[100];
    char *pair = strtok(argv[i], ":");
    strcpy(key, pair);
    pair = strtok(NULL, "");
    strcpy(val, pair);
    acr_pairs[key] = val;
  }

  // Connecting to PostgreSQL
  try
  {
    char user[50];
    char pass[50];
    strcpy(user, argv[2]);
    strcpy(pass, argv[3]);
    char connect[200] = "dbname = template1 user = ";
    strcat(connect, user);
    strcat(connect, " password = ");
    strcat(connect, pass);
    strcat(connect, " hostaddr = 127.0.0.1 port = 5432");

    pqxx::connection C(connect);
    ostringstream sql_oss;
    string sql_oss_str;
    if (C.is_open()) 
    {
      cout << "Opened database successfully: " << C.dbname() << endl;
      char sql[100] = "CREATE TABLE IF NOT EXISTS ACRONYMS(" \
                      "ACRO   CHAR(10) NOT NULL," \
                      "EXPAND CHAR(100) NOT NULL);";
      pqxx::work W(C);
      W.exec(sql);
      W.commit();
      cout << "Table created successfully" << endl;

      map<string, string>::iterator it;
      for (it = acr_pairs.begin(); it != acr_pairs.end(); it++)
      {
        sql_oss << "INSERT INTO ACRONYMS (ACRO,EXPAND) (" << it->first << "," << it->second << ");";
        sql_oss_str = sql_oss.str();
        pqxx::work W(C);
        W.exec(sql);
        W.commit();
        cout << "Table updated successfully | " << it->first << " : " << it->second << endl;
      }
    } 
    else 
    {
      cout << "Can't open database" << endl;
      return 1;
    }
    //C.disconnect();
  } 
  catch (const exception &e) 
  {
      cerr << e.what() << endl;
      return 1;
  }

  // Reading from file into variable
  char *buffer = (char *)malloc(sizeof(char) * MAXBUFFER);
  char *change = (char *)malloc(sizeof(char) * MAXBUFFER);
  fstream file(argv[1]);
  file.read(buffer, MAXBUFFER);

  // Main replacement segment
  char c;
  char acro[10];
  int cnt = 0;
  LL buff_size = strlen(buffer);
  for(int i = 0, j = 0; i < buff_size; ++i, ++j)
  {
    c = buffer[i];
    if (c >= 'A' && c <= 'Z' && cnt != 10)
    {
      acro[cnt] = c;
      cnt += 1;
    }
    else
    {
      // More than 1 upper case letter in a row
      if (cnt > 1)
      {
        cout << "Acro is: " << acro << endl;
        if (acr_pairs.find(acro) != acr_pairs.end())
        {
          string val = acr_pairs[acro];
          j -= cnt;
          for (int k = 0; k < val.length(); ++k)
          {
            if (val[k] >= 'A' && val[k] <= 'Z' && k != 0)
            {
              change[j++] = ' ';
            }
            change[j++] = val[k];
          }
        }
        memset(acro, '\0', 10);
      }
      cnt = 0;
    }
    change[j] = c;
  }

  file.close();
  if (remove(argv[1]) != 0)
    perror("Error deleting file");
  else
    puts("File successfully deleted");

  ofstream new_file(argv[1]);
  new_file << change;
  new_file.close();

  printf("%s\n", change);

	return 0;
}
