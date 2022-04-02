#include <iostream>
#include <pqxx/pqxx>
#include <fstream>
#include <sstream>

#include "exerciser.h"

using namespace std;
using namespace pqxx;


void drop_tables(connection *C) {
    string query1 = "DROP TABLE IF EXISTS PLAYER CASCADE;\n";
    string query2 = "DROP TABLE IF EXISTS TEAM CASCADE;\n";
    string query3 = "DROP TABLE IF EXISTS STATE CASCADE;\n";
    string query4 = "DROP TABLE IF EXISTS COLOR CASCADE;\n";
    string query = query1 + query2 + query3 + query4;
    work W(*C);
    W.exec(query);
    W.commit();
}

void create_tables(connection *C) {
    string filename = "create_tables.txt";
    ifstream file(filename.c_str());
    string query, line;
    if(file.is_open()) {
        while(getline(file, line)) {
            query.append(line);
        }
        file.close();
    }
    //cout << "create tables successfully" << endl;
    work W(*C);
    W.exec(query);
    W.commit();
}


void add_from_player(connection *C) {
  string filename = "player.txt";
  ifstream file(filename.c_str());
  if(file.fail()) {
    cout << "fail to open player.txt" << endl;
    return;
  }
  string line;
  int player_id, team_id, jersey_num, mpg, ppg, rpg, apg;
  double spg, bpg;
  string first_name, last_name;
  while(getline(file, line)) {
    stringstream query(line);
    query >> player_id >> team_id >> jersey_num >> first_name >> last_name >> mpg >> ppg >> rpg >> apg >> spg >> bpg;
    add_player(C, team_id, jersey_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
  }
  file.close();
}


void add_from_team(connection *C) {
  string filename = "team.txt";
  ifstream file (filename.c_str());
  if(file.fail()) {
    cout << "fail to open team.txt" << endl;
    return;
  }
  string line;
  string name;
  int team_id, state_id, color_id, wins, losses;
  double spg, bpg;
  while(getline(file, line)) {
    stringstream query(line);
    query >> team_id >> name >> state_id >> color_id >> wins >> losses;
    add_team(C, name, state_id, color_id, wins, losses);
  }
  file.close();
}


void add_from_state(connection *C) {
  string filename = "state.txt";
  ifstream file(filename.c_str());
  if(file.fail()) {
    cout << "fail to open state.txt" << endl;
    return;
  }
  string line, name;
  int state_id;
  while(getline(file, line)) {
    stringstream query(line);
    query >> state_id >> name;
    add_state(C, name);
  }
  file.close();
}


void add_from_color(connection *C) {
  string filename = "color.txt";
  ifstream file(filename.c_str());
  if(file.fail()) {
    cout << "fail to open color.txt" << endl;
    return;
  }
  string line, name;
  int color_id;
  while(getline(file, line)) {
    stringstream query(line);
    query >> color_id >> name;
    add_color(C, name);
  }
  file.close();
}


int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }


  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files
  drop_tables(C);
  create_tables(C);
  add_from_state(C);
  add_from_color(C);
  add_from_team(C);
  add_from_player(C); 

  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


