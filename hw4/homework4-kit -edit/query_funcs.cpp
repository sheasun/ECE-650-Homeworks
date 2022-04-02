#include "query_funcs.h"
#include <iomanip>

void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
    stringstream sqlStr;
    work W(*C);
    sqlStr << "INSERT INTO PLAYER (TEAM_ID,UNIFORM_NUM,FIRST_NAME,LAST_NAME,MPG,PPG,RPG,APG,SPG,BPG) VALUES(";
    sqlStr << team_id << ", ";
    sqlStr << jersey_num << ", ";
    sqlStr << W.quote(first_name) << ", ";
    sqlStr << W.quote(last_name) << ", ";
    sqlStr << mpg << ", " << ppg << ", " << rpg << ", " << apg << ", " << spg << ", " << bpg << ");";
    W.exec(sqlStr.str());
    W.commit();
}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
    stringstream sqlStr;
    work W(*C);
    sqlStr << "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) VALUES (";
    sqlStr << W.quote(name) << ", ";
    sqlStr << state_id << ", ";
    sqlStr << color_id << ", ";
    sqlStr << wins << ", ";
    sqlStr << losses << ");";
    W.exec(sqlStr.str());
    W.commit();
}


void add_state(connection *C, string name)
{
    work W(*C);
    string sqlStr = "INSERT INTO STATE (NAME) VALUES (" + W.quote(name) + ");";
    W.exec(sqlStr);
    W.commit();
}


void add_color(connection *C, string name)
{
    work W(*C);
    string sqlStr = "INSERT INTO COLOR (NAME) VALUES(" + W.quote(name) + ");";
    W.exec(sqlStr);
    W.commit();
}


void print_query1(result R) {
    cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG" << endl;
    result::const_iterator iter = R.begin();
    while(iter != R.end()) {
        cout << iter[0] << " ";
        cout << iter[1] << " ";
        cout << iter[2] << " ";
        cout << iter[3] << " ";
        cout << iter[4] << " ";
        cout << iter[5] << " ";
        cout << iter[6] << " ";
        cout << iter[7] << " ";
        cout << iter[8] << " ";
        cout << fixed << setprecision(1) << iter[9].as<double>() << " ";
        cout << fixed << setprecision(1) << iter[10].as<double>() << endl;
        ++iter;
    }
}


void query1(connection *C,
	    int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
    stringstream query;
    query << "SELECT * FROM PLAYER";
    int use[6] = {use_mpg, use_ppg, use_rpg, use_apg, use_spg, use_bpg};
    string name[6] = {"MPG", "PPG", "RPG", "APG", "SPG", "BPG"};
    int min[4] = {min_mpg, min_ppg, min_rpg, min_apg}; //min_spg, min_bpg};
    int max[4] = {max_mpg, max_ppg, max_rpg, max_apg}; //max_spg, max_bpg};
    double min_double[2] = {min_spg, min_bpg};
    double max_double[2] = {max_spg, max_bpg};
    bool first = true;
    for(int i = 0; i < 4; i++) {
        if(use[i]) {
            if(first == true) {
                query << " WHERE ";
                first = false;
            }
            else {
                query << " AND ";
            }
            query << "(" << name[i] << " BETWEEN " << min[i] << " AND " << max[i] << ")";
        }
    }
    for(int i = 0; i < 2; i++) {
        if(use[i+4]) {
            if(first == true) {
                query << " WHERE ";
                first = false;
            }
            else {
                query << " AND ";
            }
            query << "(" << name[i+4] << " BETWEEN " << min_double[i] << " AND " << max_double[i] << ")";
        }
    }
    query << ";";
    nontransaction N(*C);
    string str = query.str();
    result R(N.exec(str));
    print_query1(R);
}


void query2(connection *C, string team_color)
{
    stringstream query;
    work W(*C);
    query << "SELECT TEAM.NAME FROM TEAM, COLOR WHERE COLOR.COLOR_ID = TEAM.COLOR_ID AND COLOR.NAME = ";
    query << W.quote(team_color) << ";";
    W.commit();
    nontransaction N(*C);
    result R(N.exec(query.str()));
    cout << "NAME" << endl;
    result::const_iterator iter = R.begin();
    while(iter != R.end()) {
        cout << iter[0].as<string>() << endl;
        ++iter;
    }
}


void query3(connection *C, string team_name)
{
    stringstream query;
    work W(*C);
    query << "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM WHERE TEAM.TEAM_ID = PLAYER.TEAM_ID AND TEAM.NAME = ";
    query << W.quote(team_name) << " ORDER BY PPG DESC;";
    W.commit();
    nontransaction N(*C);
    result R(N.exec(query.str()));
    cout << "FIRST_NAME LAST_NAME" << endl;
    result::const_iterator iter = R.begin();
    while(iter != R.end()) {
        cout << iter[0].as<string>() << " " << iter[1].as<string>() << endl;
        ++iter;
    }
}


void query4(connection *C, string team_state, string team_color)
{
    stringstream query;
    work W(*C);
    query << "SELECT PLAYER.UNIFORM_NUM, PLAYER.FIRST_NAME, PLAYER.LAST_NAME FROM PLAYER, STATE, COLOR, TEAM WHERE ";
    query << "PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.COLOR_ID = COLOR.COLOR_ID AND TEAM.STATE_ID = STATE.STATE_ID AND ";
    query << "STATE.NAME = " << W.quote(team_state) << " AND " << "COLOR.NAME = " << W.quote(team_color) << ";";
    W.commit();
    nontransaction N(*C);
    result R(N.exec(query.str()));
    cout << "UNIFORM_NUM FIRST_NAME LAST_NAME" << endl;
    result::const_iterator iter = R.begin();
    while(iter != R.end()) {
        cout << iter[0].as<int>() << " " << iter[1].as<string>() << " " << iter[2].as<string>() << endl;
        ++iter;
    }
}


void query5(connection *C, int num_wins)
{
    stringstream query;
    //work W(*C);
    query << "SELECT FIRST_NAME, LAST_NAME, TEAM.NAME, WINS FROM PLAYER, TEAM WHERE ";
    query << "PLAYER.TEAM_ID = TEAM.TEAM_ID AND WINS >" << num_wins << ";";
    nontransaction N(*C);
    result R(N.exec(query.str()));
    cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
    result::const_iterator iter = R.begin();
    while(iter != R.end()) {
        cout << iter[0].as<string>() << " " << iter[1].as<string>() << " " << iter[2].as<string>() << " " << iter[3].as<int>() << endl;
        ++iter;
    }
}
