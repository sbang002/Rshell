#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cstring>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "timer.h"
#include <fcntl.h>
#include <fstream>
#include <istream>
#include <ostream>
#include <cstdio>

using namespace std;

//function to check for parameters in loop
//checks for connectors and #
bool CHK( string Command_charicter, unsigned number)
{
    if(Command_charicter[number] == '|')
    { 
        if(number + 1 < Command_charicter.size())
        {
            if(Command_charicter[number+1] == '|')
            {
                return false;
            }
        }
    }
    if(Command_charicter[number] == '#')
    {
        return false;
    }
    if(Command_charicter[number] == '&')
    {
        if(number+1 < Command_charicter.size())
        {
            if(Command_charicter[number +1 ] == '&')
            {
                return false;
            }
        }
    }
    if(Command_charicter[number] == ';')
    {
        return false;
    }
    if(Command_charicter[number] == ')')
    {
        return false;
    }
    if(Command_charicter[number] == '(')
    {
        return false;
    }
    return true;
}
//checks for everything in CHK and space
bool CHKSp(string Command_charicter, int place)
{
    if(Command_charicter[place] == ' ')
    {
        return false;
    }
    return CHK(Command_charicter, place);
}


// groups the statements (ex: if we ran 1 || 2 & 3, it would run (1||2) && 3))
// execvpRun = current result of execvp, execGrouper 
//= boolean combination of last few execvps
void CrentAndNxtExecChk(string connectorCMD, bool execvpRun, bool &execGrouper)
{
    // execvpRun checks if current command passed or failed
    if (connectorCMD == ";")
    {
        execGrouper = true;
    }
    else if (connectorCMD == "&&")
    {
        execGrouper = execGrouper && execvpRun;
    }
    else if (connectorCMD == "||")
    {
        execGrouper = execGrouper || execvpRun;
    }
}

// checks whether or not next command SHOULD run or not (DoNotRunNextBool)
// CALL ONLY ON VECTOR OF CONNECTORS
void connect(string connectorCMD, bool execvpRun, bool &DoNotRunNextBool) 
{
    DoNotRunNextBool = false;
    
    // execvpRun checks if current command passed or failed
    if (connectorCMD == ";")
    {
        // do nothing (execvpRun == true)
    }
    else if (connectorCMD == "&&")
    {
        // check if current one passed (execvpRun == true)
            // if current one passed, goto next one 
            // if current one failed, do not run next one
        if (execvpRun == false)
        {
            // ignore rest
            DoNotRunNextBool = true;
        }
        // else
        // {
        //     // do nothing (execvpRun == true), lets it run the next command
        // }
    }
    else if (connectorCMD == "||")
    {
        // check if current one passed (execvpRun == true)
            // if current one passed, fail next one
            // if current one failed, check next one
                // if next one failed, do not run either
        if (execvpRun == true)
        {
            DoNotRunNextBool = true;
        }
        // else
        // {
        //     // check next one
        // }
    }
    // else
    // {
    //     // not valid connector
    // }
} 
// if DoNotRunNextBool == false, then that means you can run next. 
//If it is == true, then skip the next input


void swapVEC(unsigned i, unsigned j, vector< vector<unsigned> > &value)
{
    vector<unsigned> temp = value[i];
    value[i] = value[j];
    value[j] = temp;
    
}

//when you call it call it only if executable.at(i) == ls
//pass in argument.at(i); for string arg
// make sure to call only if executable.at(i) == "ls"
bool check_LS( string argum )
{
    string arg;
    for(unsigned i = 0; i < argum.size(); i++)
    {
        if(argum.at(i) == '\0')
        {
            i = argum.size();
        }
        else
        {
            arg = arg + argum.at(i);
        }
    }
    
    if(arg == "-e" || arg == "-E" || arg == "-j" || arg == "-J")
    {
        return false;
    }
    if(arg == "-M" || arg == "-O" || arg == "-P" || arg == "-V")
    {
        return false;
    }
    if(arg == "-W" || arg == "-y" || arg == "-Y" || arg == "-z")
    {
        return false;
    }
    return true;
}


//cp call we need to call this only if executable.at(i) == cp;need to update execvpRun according to cp
bool copy_function(string argument)
{
    string src; //= "";
    string dest; //= "";
    string third;// = "";
    unsigned i;
    //gets first argument
    for(i = 0; (i < argument.size()) && (argument.at(i) != ' '); i++)
    {
        src.push_back(argument.at(i));
    }
    //removes whitespace between arguments
    while((i < argument.size()) && (argument[i] == ' '))
    {
        i++;
    }
    //gets second argument
    while((i < argument.size()) && (argument.at(i) != ' '))
    {
        dest.push_back(argument.at(i));
        i++;
    }
    //removes whitespace between arguments
    while((i < argument.size()) && (argument[i] == ' '))
    {
        i++;
    }
    //gets third argument
    while((i < argument.size()) && (argument.at(i) != ' '))
    {
        third.push_back(argument.at(i));
        i++;
    }
    
    
    //A is first argument
    char *A = new char( src.length() +1);
    strcpy(A, src.c_str());
    if(*A == '\0')
    {
        A = NULL;
    }
    
    //B is second argument
    char *B = new char( dest.length() +1);
    strcpy(B, dest.c_str());
    if(*B == '\0') 
    {
        B = NULL;
    }
    
    //C is third argument
    char *C = new char( third.length() +1);
    strcpy(C, third.c_str());
    if(*C == '\0')
    {
        C = NULL;
    }
    
    //Too few arguments given
    if(A == NULL || B == NULL)
    {
        cout << "ERROR: Too few arguments given" << endl;
        delete C;
        delete B;
        delete A;
        return false;
    }
    
    struct stat buf;
   
    //Test if src exists
    stat(A, &buf);
    if (!(S_ISREG(buf.st_mode)))
    {
        cout << "ERROR: Source file is not a regular file" << endl;
        delete C;
        delete B;
        delete A;
        return false;
    }
   
    //Test if dest exists
    if (stat(B, &buf) == 0)
    {
        cout << "ERROR: Destination file already exists" << endl;
        delete C;
        delete B;
        delete A;
        return false;
    }
    
    //If no third argument
    if(C == NULL)
    {
        int fdsrc = open(A, O_RDONLY);
        int fddst = open(B, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR
            | S_IRGRP);
       
        if(fdsrc < 0)
        {
            cout << "ERROR: Source file did not open properly" << endl;
            return false;
        }
        if(fddst < 0)
        {
            cout << "ERROR: Destination file did not open properly" << endl;
            return false;
        }
       
        char cb[BUFSIZ];
    
        int numRead = read(fdsrc, (void*)&cb, BUFSIZ);
        write(fddst, (void*)&cb, numRead);
       
        close(fdsrc);
        close(fddst);
    }
    //If third argument is given
    else if(C != NULL)
    {
        //Use the std::istream::get and std::ostream::put C++ functions to
        //copy the input file to the output file one character at a time.
        cout << "Method 1 " << endl;
        Timer t1;
        double eTime1;
        t1.start();
       
        ifstream ifs(A);
        ofstream ofs(B);
       
        if(!(ifs.is_open()))
        {
            cout << "ERROR: Source file did not open properly" << endl;
            return false;
        }
        if(!(ofs.is_open()))
        {
            cout << "ERROR: Destination file did not open properly" << endl;
            return false;
        }
       
        char c;
        string s;
        while (ifs.get(c))
            s.push_back(c);
           
        char* cs = new char[s.size() + 1];
        strcpy(cs, s.c_str());
       
        ofs.write(cs, s.size());
       
        delete[] cs;
       
        ifs.close();
        ofs.close();
       
        t1.elapsedUserTime(eTime1);
        cout << "Time: " <<  eTime1 << endl;
       
        //Use the Unix system calls read() and write() to copy the input
        //file to the output file one character at a time.
        cout << "Method 2 " << endl;
        Timer t2;
        double eTime2;
        t2.start();
       
        int fdsrc = open(A, O_RDONLY);
        int fddst = open(B, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR
            | S_IRGRP);
       
        if(fdsrc < 0)
        {
            cout << "ERROR: Source file did not open properly" << endl;
            return false;
        }
        if(fddst < 0)
        {
            cout << "ERROR: Destination file did not open properly" << endl;
            return false;
        }
       
        while (read(fdsrc, (void*)&c, 1))
            write(fddst, (void*)&c, 1);
       
        close(fdsrc);
        close(fddst);
       
        t2.elapsedUserTime(eTime2);
        cout << "Time: " <<  eTime2 << endl;
       
       
        //Use the Unix system calls read() and write() to copy the input
        //file to the output file one buffer at a time. The buffer should
        //be of size BUFSIZ, which is declared in the stdio.h include file
        cout << "Method 3 " << endl;
        Timer t3;
        double eTime3;
        t3.start();
       
        fdsrc = open(A, O_RDONLY);
        fddst = open(B, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR
            | S_IRGRP);
       
        if(fdsrc < 0)
        {
            cout << "ERROR: Source file did not open properly" << endl;
            return false;
        }
        if(fddst < 0)
        {
            cout << "ERROR: Destination file did not open properly" << endl;
            return false;
        }
       
        char cb[BUFSIZ];
       
        int numRead = read(fdsrc, (void*)&cb, BUFSIZ);
        write(fddst, (void*)&cb, numRead);
       
        close(fdsrc);
        close(fddst);
       
        t3.elapsedUserTime(eTime3);
        cout << "Time: " <<  eTime3 << endl;
       
        return true;
    }
    delete C;
    delete B;
    delete A;
    
    return false;
}


void do_work(string command, unsigned &place)
{
    vector<string> executable; //vector to hold all executables passed in a line
    vector<string> argumentlist;//vector to hold all arguments passed in a line
    vector<string> connectorCMD;//vector to hold all connectors passed in a line
    
    vector< vector<unsigned> > parenGrouper; // vector to hold all parenthesis spot
    
    stack<unsigned> starting_place_Par; //used in while loop to hold starting places of ()
    // KENNY
    bool execvpRun = true; // set it to true so we know the result of execvp 
                          //(true if did not change, false if failed)
    bool DoNotRunNextBool = false; // need it to be false so it runs in the 
                                  //first case
    //
    //this bool keeps track of previous combination of functions were T/F
    bool execGrouper = true;
    
    bool currPrecBool; // stores the precedence bool of the ( ) we are currently on
    bool prevPrecBool = true; // stores the precedence bool of the last ( )
    
    unsigned parenGrouperCounter = 0; // this keeps track of which parenGrouper we are at
    
    while(place < command.size())
    {
        
        string Executable; // holds all executables
        string Argumentlist; // holds all arguments
        string ConnectorCMD; // holds all connectors
        
        bool Found_First_Par = false;
        bool Found_End_Par = false;
        

        //get rid of leading whitespace
        while((place < command.size()) && (command[place] == ' '))
        {
            place++;
        }
        
        
        // open parenthesis check if size of command and conector are same
        // if we find first par set Found_first_par to true
        if((place < command.size()) && (command.at(place) == '('))
        {
          Found_First_Par = true;
          place++;
        }
        if(Found_First_Par)
        {
            if(executable.size() == connectorCMD.size())
            {   
              //if Found_first_par is true add <currentexecutable#,currentcommandcounter> to stack
                starting_place_Par.push(executable.size());
            }
            else
            {
                cout << "ERROR improper input" << endl;
                return;
           }
          // set Found_first_par to false
          Found_First_Par = false;
        }
        
        // if we find second par set Found_second_par to true
        if((place < command.size()) && (command.at(place) == ')'))
        {
            
          Found_End_Par = true;
          place ++;
        }
        if(Found_End_Par)
        {
            // check if size of command -1 = conector size
            if((executable.size() - 1) != connectorCMD.size())
            {
                cout << "ERROR improper input 1" << endl;
                return;
            }
            // if Found_second_par == true 
            //     if stack != empty 
            if(!starting_place_Par.empty())
            {
            //       than we pop it so we will get back where this () start from
                unsigned i = starting_place_Par.top();
                starting_place_Par.pop();
            //       and we need to create a pair with <currentexecutable#,currentcommandcounter>
            //       so we know where we ended 
                vector<unsigned> j;
                j.push_back(i);
                j.push_back(executable.size());
                j.push_back(connectorCMD.size());
        
                //parenGrouper[i][1] = start , [2] = end place of executable, [3] = end place of connector
                parenGrouper.push_back(j);
              
            } 
            else
            {
                cout << "ERROR improper input 2 " << endl;
                return;
            }
            // set Found second par = false
            Found_End_Par = false;
        }
        
        
        //get rid of leading whitespace
        while((place < command.size()) && (command[place] == ' '))
        {
            place++;
        }
        
        
        //get executable
        if((place < command.size()) && command[place] != '#')
        {
            if(command.at(place) == '[')
            {
                Executable = "[";
                place ++;
            }
            else
            {
                for(unsigned i = place; i < command.size() && CHKSp(command, i); i++)
                {
                    Executable.push_back(command.at(i));
                    place++;
                }
            }
        }
        
        //goes over the extra space
        while((place < command.size()) && (command[place] == ' '))
        {
            place++;
        }
        
        //check if we are at end of command entered if not accept more
        //add into argument list
        if( place <  command.size())
        {
            if((command.at(place)!= ' ') && CHK(command, place))
            {
                if(Executable == "[")
                {
                    for(unsigned i = place; (i < command.size()) && (command.at(i) != ']'); i++)
                    {
     
                        place++;
                        //check if inside quotes ""  “ ”
                        if ((i < command.size()) && (command.at(i) ==  '\"') )
                        {
                            i++;
                            while((i < command.size()) && (command.at(i) != '\"'))
                            {
                                Argumentlist.append(command,i,1);
                                i++;
                                place++;
                            }
                            if(i < command.size())
                            {
                                // Argumentlist.append(command,i,1);
                                place ++;
                                i++;
                            }
                        }
                        if(i < command.size())
                        {
                            Argumentlist.push_back(command.at(i));
                        }
                        
                    }
                    
                    if(place > command.size())
                    {
                        //give error and ____ program either quit this run and wait for next or we just wait
                        cerr << "No ']' for the test function." << endl;
                        return;
                    }
                    else
                    {
                        place++;
                    }
                    
                }
                else
                {
                    for(unsigned i = place; (i < command.size()) && CHK(command, i); i++)
                    {
     
                        
                        place++;
    
                        
                        //check if inside quotes ""
                        if ((i < command.size()) && (command.at(i) ==  '\"'))
                        {
                            i++;
                            while((i < command.size()) && (command.at(i) != '\"'))
                            {
                                Argumentlist.append(command,i,1);
                                i++;
                                place++;
                            }
                            if(i < command.size())
                            {
                                // Argumentlist.append(command,i,1);
                                place ++;
                                i++;
                            }
                        }
                        if(i < command.size())
                        {
                            Argumentlist.push_back(command.at(i));
                        }
                        
                    }
                }
                
                if(!Argumentlist.empty())    
                {
                    if(Argumentlist.at(Argumentlist.size()-1) == ' ')
                    {
                        Argumentlist.at(Argumentlist.size()-1) = '\0';
                    }
                }
            }
        }
        //check if Executable is '[]' and if it is we swap it with "test"
        if( Executable == "[")
        {
            Executable = "test";
        }
        
        while((place < command.size()) && (command[place] == ' '))
        {
            place++;
        }
        
        //check if we are at end if not the last part is a connector cmd
        if( place < command.size())
        {
            if(command[place] == ';')
            {
                ConnectorCMD = ';';
                place++;
            }
            else if(command[place] == '|')
            {
                if(place +1 < command.size())
                {
                    if(command[place +1] == '|')
                    {
                        ConnectorCMD = "||";
                        place ++;
                        place ++;
                    }
                }
            }
            else if(command[place] == '&')
            {
                if(place+1 < command.size())    
                    if(command[place +1] == '&')
                    {
                        ConnectorCMD = "&&";
                        place ++;
                        place ++;
                    }
            }
        }
        
        if(place < command.size())
        {
            if(command[place] == '#')
            {
                place = command.size();
            }
        }
        
         // put the strings into the vector now as c string
        if(Executable != "\0")
        {
            executable.push_back(Executable);
            
            argumentlist.push_back(Argumentlist);
        }
        
        if(ConnectorCMD != "\0")
        {
            if(executable.size() - 1 == connectorCMD.size())
            {
                connectorCMD.push_back(ConnectorCMD);
            }
            else
            {
                cerr << "Syntax Error" << endl;
                return;
            }
        }
        
    }
    
    //check if starting_place_Par is not empty means we have extra '(' so return error and quit function
    if(!starting_place_Par.empty())
    {
        cout << "ERROR improper input" << endl;
        return;
    }
    
   
    //function to put the parenGrouper vector in order of ascending from [i][0] element
    //if there is tie then the [i][1] element is looked at and the higher is put in front
    for(unsigned j = 0; j < parenGrouper.size(); j++)
    {    
      for(unsigned i = j+1; i < parenGrouper.size(); i++)
      {
        if(parenGrouper[i][0] < parenGrouper[j][0])
        {
          swapVEC(i,j,parenGrouper);
        }
        else if(parenGrouper[i][0] == parenGrouper[j][0])
        {
          if(parenGrouper[j][1] < parenGrouper[i][1])
          {
            swapVEC(i,j,parenGrouper);
          }
        }
      }
    }


    // if there are no paranthesis or if there is only one paranthesis
    if (parenGrouper.empty() || connectorCMD.empty() || ((parenGrouper.size() == 1 && parenGrouper.at(0).at(0) == 0) && parenGrouper.at(0).at(1) == executable.size()))
    {
        bool RUN_AGAIN = true; // tells us whether or not next executable should run
        unsigned i = 0; // executable counter
        unsigned Conn_Counter = 0; // connecter counter
        //running executables
        while((i < executable.size()))
        {   
            
            // cout << "RUN_AGAIN " << RUN_AGAIN << " Executable "  << executable[i]  << " EXECGROUPER " << execGrouper  << ' ' << execvpRun << endl;
            if(RUN_AGAIN)
            {
                //now we got stuff we do work
                //make sure excp u insert c_str
                //make child process
                if(executable[i] == "exit")
                {
                    exit(1);
                }
                    
                
                //cp function
                if(executable[i] == "cp")
                {
                    execvpRun = copy_function(argumentlist.at(i));
                }
                else if(executable[i] != "test")
                {   
                    pid_t c_pid;//, pid;
                    int status;
                    //  char * args[2] = { "ls", NULL} ;
                    c_pid = fork();
                    if( c_pid < 0) // failed fork
                    {
                        perror("fork failed");
                        exit(1);
                    }
            
                    else if (c_pid == 0) // successful fork
                    {
                        
                        
                        char *A = new char( executable[i].length() +1);
                        strcpy(A, executable[i].c_str());
                        
                        char* D = NULL;
            
                        if(A[0] != '\0')
                        {
                            D = A;
                        }
                        //**********************************************
                        char *B = new char( argumentlist[i].length() +2);
                        strcpy(B, argumentlist[i].c_str());
            
                        //**********************************************
                        char* C = NULL;
            
                        if(B[0] != '\0')
                        {
                            C = B;
                        }
                        
                    
                        char* L[3] = {D, C, NULL};
            
                        
                        // printf("Child: executing \n");       
                        
                        execvp(L[0], L);
                
                        perror("execve failed");
            
                        // delete A;
                        // delete B;
                        
                        for(unsigned k = executable[i].length() +1; k >= 0; k--)
                        {
                            delete ( D + k);
                        }
                        for(unsigned k = argumentlist[i].length() +1; k >= 0; k--)
                        {
                            delete (C+ k);
                        }
                        
                    }
                    else if (c_pid > 0) // parent
                    {
                        wait(&status);
                        // pid = wait(&status); // FIXME ERROR change how we call wait
                                        
                        if (!(WIFEXITED(status))) // check if execvp failed through waitid
                        {
                            execvpRun = false;
                        }
                        
                        else
                        {
                            execvpRun = true;
                            
                        }
                        
                        if (executable.at(i) == "ls")
                        {
                            execvpRun = check_LS(argumentlist.at(i));
                        }
                        // printf("Parent: finished\n");
                    }
                }
                //test function have to figure out what to return
                else
                {
                    struct stat buf;
                    string test_arg = argumentlist[i];
                    if((test_arg[0] == '-') && (test_arg[1] == 'f'))
                    { 
                      test_arg.erase(0,3);
                      stat(test_arg.c_str(),&buf);
                      if(S_ISREG(buf.st_mode) != 0 )
                      {
                        execvpRun = true;
                        // cout << "file" << endl;
                      }
                      else
                      {
                        execvpRun = false;
                        // cout << "not file" << endl;
                      }
                    }
                    //-d check flag = checks if directory
                    else if((test_arg[0] == '-') && (test_arg[1] == 'd'))
                    {
                      test_arg.erase(0,3);
                      stat(test_arg.c_str(),&buf);
                      if(S_ISDIR(buf.st_mode))
                      {
                        execvpRun = true;
                        // cout << "folder" << endl;
                      }
                      else
                      {
                        execvpRun = false;
                        // cout << "not folder" << endl;
                      }
                    }
                    //-e checks for existence
                    else 
                    {
                      if((test_arg[0] == '-') && (test_arg[1] == 'e'))
                      {
                        test_arg.erase(0,3);
                      }
                      if(stat(test_arg.c_str(),&buf) == 0 )
                      {
                        execvpRun = true;
                        // cout << "exist" << endl;
                      }
                      else
                      {
                        execvpRun = false;
                        // cout << "doesn't exist" << endl;
                      }
                    }
                }
            }
            
            if(RUN_AGAIN)
            {
                if(executable[i] == "exit")
                {
                    exit(1);
                }
            }
            
            // checks whether or not next command SHOULD run or not (DoNotRunNextBool)
            RUN_AGAIN = true;
            if (!(connectorCMD.empty()))
            {
                // testing
                if(Conn_Counter < connectorCMD.size() && i == 0)
                {
                    connect(connectorCMD.at(Conn_Counter), execvpRun, DoNotRunNextBool);
                    RUN_AGAIN = !DoNotRunNextBool;
                    //call new function on execGrouper to change it DoNotRunNextBool 
                    // as parameter
                    execGrouper = execvpRun;
                }
                else if (Conn_Counter < connectorCMD.size() && i > 0)
                {
                  // new boolean combination INCLUDING the current 
                  //executable execvpRun
                    CrentAndNxtExecChk(connectorCMD.at(Conn_Counter), execvpRun, execGrouper);
                    execGrouper = execvpRun;
                    connect(connectorCMD.at(Conn_Counter), execGrouper, DoNotRunNextBool);
                    RUN_AGAIN = !DoNotRunNextBool;
                }
            }
            // cout << "BOT RUN_AGAIN "<< RUN_AGAIN  << " EXECGROUPER " << execGrouper << ' ' << execvpRun << endl;
            Conn_Counter++;
            i++;
        }
    }
    
    // if there are multiple paranthesis
    else if (parenGrouper.size() > 0)
    {
        bool RUN_AGAIN = true; // tells us whether or not next executable should run
        unsigned i = 0; // executable counter
        unsigned Conn_Counter = 0; // connecter counter
        //running executables (original while loop)
        while (i < executable.size()) // change while loop so it does not run until the end of all the executables, runs until the first '(' FIXMEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
        {
            // only goes in here if there is stuff after paranthesis
            if (parenGrouperCounter >= parenGrouper.size() - 1)
            {
                // this is the very last ()
                while((i < executable.size()))     
                {   
                    // cout << "RUN_AGAIN " << RUN_AGAIN << " Executable "  << executable[i]  << " EXECGROUPER " << execGrouper  << ' ' << execvpRun << endl;
                    if(RUN_AGAIN)
                    {
                        //now we got stuff we do work
                        //make sure excp u insert c_str
                        //make child process
                        if(executable[i] == "exit")
                        {
                            exit(1);
                        }
                        if(executable[i] == "cp")
                        {
                            execvpRun = copy_function(argumentlist.at(i));
                        }    
                        else if(executable[i] != "test")
                        {
                            pid_t c_pid;//, pid;
                            int status;
                            //  char * args[2] = { "ls", NULL} ;
                            c_pid = fork();
                            if( c_pid < 0) // failed fork
                            {
                                perror("fork failed");
                                exit(1);
                            }
                    
                            else if (c_pid == 0) // successful fork
                            {
                                
                                
                                char *A = new char( executable[i].length() +1);
                                strcpy(A, executable[i].c_str());
                                
                                char* D = NULL;
                    
                                if(A[0] != '\0')
                                {
                                    D = A;
                                }
                                //**********************************************
                                char *B = new char( argumentlist[i].length() +2);
                                strcpy(B, argumentlist[i].c_str());
                    
                                //**********************************************
                                char* C = NULL;
                    
                                if(B[0] != '\0')
                                {
                                    C = B;
                                }
                                
                            
                                char* L[3] = {D, C, NULL};
                    
                                
                                // printf("Child: executing \n");       
                                
                                execvp(L[0], L);
                        
                                perror("execve failed");
                    
                                // delete A;
                                // delete B;
                                
                                for(unsigned k = executable[i].length() +1; k >= 0; k--)
                                {
                                    delete ( D + k);
                                }
                                for(unsigned k = argumentlist[i].length() +1; k >= 0; k--)
                                {
                                    delete (C+ k);
                                }
                                
                            }
                            else if (c_pid > 0) // parent
                            {
                                wait(&status);
                                // pid = wait(&status); // FIXME ERROR change how we call wait
                                                
                                if (!(WIFEXITED(status))) // check if execvp failed through waitid
                                {
                                    execvpRun = false;
                                }
                                
                                
                                else
                                {
                                    execvpRun = true;
                                    
                                }
                                
                                if (executable.at(i) == "ls")
                                {
                                    execvpRun = check_LS(argumentlist.at(i));
                                }
                                
                                if (executable.at(i) == "ls")
                                {
                                    execvpRun = check_LS(argumentlist.at(i));
                                }
                
                                // printf("Parent: finished\n");
                            }
                        }
                        //test function have to figure out what to return
                        else
                        {
                            struct stat buf;
                            string test_arg = argumentlist[i];
                            if((test_arg[0] == '-') && (test_arg[1] == 'f'))
                            { 
                              test_arg.erase(0,3);
                              stat(test_arg.c_str(),&buf);
                              if(S_ISREG(buf.st_mode) != 0 )
                              {
                                execvpRun = true;
                                // cout << "file" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "not file" << endl;
                              }
                            }
                            //-d check flag = checks if directory
                            else if((test_arg[0] == '-') && (test_arg[1] == 'd'))
                            {
                              test_arg.erase(0,3);
                              stat(test_arg.c_str(),&buf);
                              if(S_ISDIR(buf.st_mode))
                              {
                                execvpRun = true;
                                // cout << "folder" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "not folder" << endl;
                              }
                            }
                            //-e checks for existence
                            else 
                            {
                              if((test_arg[0] == '-') && (test_arg[1] == 'e'))
                              {
                                test_arg.erase(0,3);
                              }
                              if(stat(test_arg.c_str(),&buf) == 0 )
                              {
                                execvpRun = true;
                                // cout << "exist" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "doesn't exist" << endl;
                              }
                            }
                        }
                    }
                    
                    if(RUN_AGAIN)
                    {
                        if(executable[i] == "exit")
                        {
                            exit(1);
                        }
                    }
                    
                    // checks whether or not next command SHOULD run or not (DoNotRunNextBool)
                    RUN_AGAIN = true;
                    if (!(connectorCMD.empty()))
                    {
                        // testing
                        if(Conn_Counter < connectorCMD.size() && i == 0)
                        {
                            connect(connectorCMD.at(Conn_Counter), execvpRun, DoNotRunNextBool);
                            RUN_AGAIN = !DoNotRunNextBool;
                            //call new function on execGrouper to change it DoNotRunNextBool 
                            // as parameter
                            execGrouper = execvpRun;
                        }
                        else if (Conn_Counter < connectorCMD.size() && i > 0)
                        {
                          // new boolean combination INCLUDING the current 
                          //executable execvpRun
                            CrentAndNxtExecChk(connectorCMD.at(Conn_Counter), execvpRun, execGrouper);
                            execGrouper = execvpRun;
                            connect(connectorCMD.at(Conn_Counter), execGrouper, DoNotRunNextBool);
                            RUN_AGAIN = !DoNotRunNextBool;
                        }
                    }
                    // cout << "BOT RUN_AGAIN "<< RUN_AGAIN  << " EXECGROUPER " << execGrouper << ' ' << execvpRun << endl;
                    Conn_Counter++;
                    i++;
                }
                goto stop;
            }
            
            // goes in here if there is still stuff before the last )
            else if (parenGrouperCounter < parenGrouper.size()) //-1 )
            {
                // cerr << "executable size: " << executable.size() << endl;
                // cerr << "executable counter: " << i << endl;
                // cout << "parenGrouper size: " << parenGrouper.size() << endl;
                // cout << "parenGrouperCounter: " <<  parenGrouperCounter << endl;
                
                // for (int j = 0; j < parenGrouper.size(); j++)
                // {
                //     cout << parenGrouper[j][0] << ' ' << parenGrouper[j][1] << ' ' << parenGrouper[j][2] << endl;
                // }
                
                // cerr << endl << endl << endl << "OUTSIDE" << endl << endl << endl;
                while ((i < parenGrouper.at(parenGrouperCounter).at(1)) && (i < parenGrouper.at(parenGrouperCounter + 1).at(0)))
                {
                    // while (parenGrouperCounter < parenGrouper.size())
                    // while (parenGrouper.at(parenGrouperCounter).at(0) > i)
        
                    if(RUN_AGAIN)
                    {
                        //now we got stuff we do work
                        //make sure excp u insert c_str
                        //make child process
                        if(executable[i] == "exit")
                        {
                            exit(1);
                        }
                        
                        if(executable[i] == "cp")
                        {
                            execvpRun = copy_function(argumentlist.at(i));
                        }    
                        else if(executable[i] != "test")
                        {
                            pid_t c_pid;//, pid;
                            int status;
                            //  char * args[2] = { "ls", NULL} ;
                            c_pid = fork();
                            if( c_pid < 0) // failed fork
                            {
                                perror("fork failed");
                                exit(1);
                            }
                    
                            else if (c_pid == 0) // successful fork
                            {
                                
                                
                                char *A = new char( executable[i].length() +1);
                                strcpy(A, executable[i].c_str());
                                
                                char* D = NULL;
                    
                                if(A[0] != '\0')
                                {
                                    D = A;
                                }
                                //**********************************************
                                char *B = new char( argumentlist[i].length() +2);
                                strcpy(B, argumentlist[i].c_str());
                    
                                //**********************************************
                                char* C = NULL;
                    
                                if(B[0] != '\0')
                                {
                                    C = B;
                                }
                                
                            
                                char* L[3] = {D, C, NULL};
                    
                                
                                // printf("Child: executing \n");       
                                
                                execvp(L[0], L);
                        
                                perror("execve failed");
                    
                                // delete A;
                                // delete B;
                                
                                for(unsigned k = executable[i].length() +1; k >= 0; k--)
                                {
                                    delete ( D + k);
                                }
                                for(unsigned k = argumentlist[i].length() +1; k >= 0; k--)
                                {
                                    delete (C+ k);
                                }
                                
                            }
                            else if (c_pid > 0) // parent
                            {
                                wait(&status);
                                // pid = wait(&status); // FIXME ERROR change how we call wait
                                                
                                if (!(WIFEXITED(status))) // check if execvp failed through waitid
                                {
                                    execvpRun = false;
                                }
                                
                                
                                else
                                {
                                    execvpRun = true;
                                    
                                }
                                
                                if (executable.at(i) == "ls")
                                {
                                    execvpRun = check_LS(argumentlist.at(i));
                                }
                
                                // printf("Parent: finished\n");
                            }
                        }
                        //test function have to figure out what to return
                        else
                        {
                            struct stat buf;
                            string test_arg = argumentlist[i];
                            if((test_arg[0] == '-') && (test_arg[1] == 'f'))
                            { 
                              test_arg.erase(0,3);
                              stat(test_arg.c_str(),&buf);
                              if(S_ISREG(buf.st_mode) != 0 )
                              {
                                execvpRun = true;
                                // cout << "file" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "not file" << endl;
                              }
                            }
                            //-d check flag = checks if directory
                            else if((test_arg[0] == '-') && (test_arg[1] == 'd'))
                            {
                              test_arg.erase(0,3);
                              stat(test_arg.c_str(),&buf);
                              if(S_ISDIR(buf.st_mode))
                              {
                                execvpRun = true;
                                // cout << "folder" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "not folder" << endl;
                              }
                            }
                            //-e checks for existence
                            else 
                            {
                              if((test_arg[0] == '-') && (test_arg[1] == 'e'))
                              {
                                test_arg.erase(0,3);
                              }
                              if(stat(test_arg.c_str(),&buf) == 0 )
                              {
                                execvpRun = true;
                                // cout << "exist" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "doesn't exist" << endl;
                              }
                            }
                        }
                    }
                    
                    if(RUN_AGAIN)
                    {
                        if(executable[i] == "exit")
                        {
                            exit(1);
                        }
                    }
                    
                    // checks whether or not next command SHOULD run or not (DoNotRunNextBool)
                    RUN_AGAIN = true;
                    if (!(connectorCMD.empty()))
                    {
                        // testing
                        if(Conn_Counter < connectorCMD.size() && i == 0)
                        {
                            connect(connectorCMD.at(Conn_Counter), execvpRun, DoNotRunNextBool);
                            RUN_AGAIN = !DoNotRunNextBool;
                            //call new function on execGrouper to change it DoNotRunNextBool 
                            // as parameter
                            execGrouper = execvpRun;
                        }
                        else if (Conn_Counter < connectorCMD.size() && i > 0)
                        {
                          // new boolean combination INCLUDING the current 
                          //executable execvpRun
                            CrentAndNxtExecChk(connectorCMD.at(Conn_Counter), execvpRun, execGrouper);
                            execGrouper = execvpRun;
                            connect(connectorCMD.at(Conn_Counter), execGrouper, DoNotRunNextBool);
                            RUN_AGAIN = !DoNotRunNextBool;
                        }
                    }
                    // cout << "BOT RUN_AGAIN "<< RUN_AGAIN  << " EXECGROUPER " << execGrouper << ' ' << execvpRun << endl;
                    Conn_Counter++; // COMMENTED OUT, FIXMEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
                    i++;
                    // // cerr << "i: " << i << endl;
                    // cerr << "Conn_Counter: " << Conn_Counter << endl;
                    // cerr << "ConnectorCMD size(): " << connectorCMD.size() << endl;
                    
                }
            }
            else if (parenGrouperCounter == parenGrouper.size())
            {   
                
                if(RUN_AGAIN)
                {
                    //now we got stuff we do work
                    //make sure excp u insert c_str
                    //make child process
                    if(executable[i] == "exit")
                    {
                        exit(1);
                    }
                    if(executable[i] == "cp")
                    {
                        execvpRun = copy_function(argumentlist.at(i));
                    }    
                    else if(executable[i] != "test")
                    {
                        pid_t c_pid;//, pid;
                        int status;
                        //  char * args[2] = { "ls", NULL} ;
                        c_pid = fork();
                        if( c_pid < 0) // failed fork
                        {
                            perror("fork failed");
                            exit(1);
                        }
                
                        else if (c_pid == 0) // successful fork
                        {
                            
                            
                            char *A = new char( executable[i].length() +1);
                            strcpy(A, executable[i].c_str());
                            
                            char* D = NULL;
                
                            if(A[0] != '\0')
                            {
                                D = A;
                            }
                            //**********************************************
                            char *B = new char( argumentlist[i].length() +2);
                            strcpy(B, argumentlist[i].c_str());
                
                            //**********************************************
                            char* C = NULL;
                
                            if(B[0] != '\0')
                            {
                                C = B;
                            }
                            
                        
                            char* L[3] = {D, C, NULL};
                
                            
                            // printf("Child: executing \n");       
                            
                            execvp(L[0], L);
                    
                            perror("execve failed");
                
                            // delete A;
                            // delete B;
                            
                            for(unsigned k = executable[i].length() +1; k >= 0; k--)
                            {
                                delete ( D + k);
                            }
                            for(unsigned k = argumentlist[i].length() +1; k >= 0; k--)
                            {
                                delete (C+ k);
                            }
                            
                        }
                        else if (c_pid > 0) // parent
                        {
                            wait(&status);
                            // pid = wait(&status); // FIXME ERROR change how we call wait
                                            
                            if (!(WIFEXITED(status))) // check if execvp failed through waitid
                            {
                                execvpRun = false;
                            }
                            
                            
                            else
                            {
                                execvpRun = true;
                                
                            }
                            if (executable.at(i) == "ls")
                            {
                                execvpRun = check_LS(argumentlist.at(i));
                            }
            
                            // printf("Parent: finished\n");
                        }
                    }
                    //test function have to figure out what to return
                    else
                    {
                        struct stat buf;
                        string test_arg = argumentlist[i];
                        if((test_arg[0] == '-') && (test_arg[1] == 'f'))
                        { 
                          test_arg.erase(0,3);
                          stat(test_arg.c_str(),&buf);
                          if(S_ISREG(buf.st_mode) != 0 )
                          {
                            execvpRun = true;
                            // cout << "file" << endl;
                          }
                          else
                          {
                            execvpRun = false;
                            // cout << "not file" << endl;
                          }
                        }
                        //-d check flag = checks if directory
                        else if((test_arg[0] == '-') && (test_arg[1] == 'd'))
                        {
                          test_arg.erase(0,3);
                          stat(test_arg.c_str(),&buf);
                          if(S_ISDIR(buf.st_mode))
                          {
                            execvpRun = true;
                            // cout << "folder" << endl;
                          }
                          else
                          {
                            execvpRun = false;
                            // cout << "not folder" << endl;
                          }
                        }
                        //-e checks for existence
                        else 
                        {
                          if((test_arg[0] == '-') && (test_arg[1] == 'e'))
                          {
                            test_arg.erase(0,3);
                          }
                          if(stat(test_arg.c_str(),&buf) == 0 )
                          {
                            execvpRun = true;
                            // cout << "exist" << endl;
                          }
                          else
                          {
                            execvpRun = false;
                            // cout << "doesn't exist" << endl;
                          }
                        }
                    }
                }
                
                if(RUN_AGAIN)
                {
                    if(executable[i] == "exit")
                    {
                        exit(1);
                    }
                }
                
                // checks whether or not next command SHOULD run or not (DoNotRunNextBool)
                RUN_AGAIN = true;
                if (!(connectorCMD.empty()))
                {
                    // testing
                    if(Conn_Counter < connectorCMD.size() && i == 0)
                    {
                        connect(connectorCMD.at(Conn_Counter), execvpRun, DoNotRunNextBool);
                        RUN_AGAIN = !DoNotRunNextBool;
                        //call new function on execGrouper to change it DoNotRunNextBool 
                        // as parameter
                        execGrouper = execvpRun;
                    }
                    else if (Conn_Counter < connectorCMD.size() && i > 0)
                    {
                      // new boolean combination INCLUDING the current 
                      //executable execvpRun
                        CrentAndNxtExecChk(connectorCMD.at(Conn_Counter), execvpRun, execGrouper);
                        execGrouper = execvpRun;
                        connect(connectorCMD.at(Conn_Counter), execGrouper, DoNotRunNextBool);
                        RUN_AGAIN = !DoNotRunNextBool;
                    }
                }
                // cout << "BOT RUN_AGAIN "<< RUN_AGAIN  << " EXECGROUPER " << execGrouper << ' ' << execvpRun << endl;
                Conn_Counter++;
                i++;
            }
            //for last parenthesis we need to make it run to the last executable of it
            
            // i and Conn_Counter are currently set at the NEXT implementation
            if(Conn_Counter < connectorCMD.size())
            {
                
                if (parenGrouperCounter == 0) // should go to everything before the first '('
                {
                    currPrecBool = execGrouper; // sets currPrecBool equal to ( ... ( , everything before the next paranthesis
                }
                
                if (parenGrouperCounter > 0) // make sure the command counter and the connector counter are the appropiate place
                {
                    CrentAndNxtExecChk(connectorCMD.at(Conn_Counter), prevPrecBool, currPrecBool); // this sets currPrecBool equal to all of the ( ) run so far, ex: if we ran (true) || (false) && (...), currPrecBool of this would be (true) && (...)
                    // precString should hold the next connectorCMD
                }
                
                // Conn_Counter - 1
                
                //test
                // cerr << "connectorCMD size: " << connectorCMD.size() << endl;
                // cerr << "Conn_Counter: " << Conn_Counter << endl;
                
                if(parenGrouperCounter == 0)
                {
                    if(parenGrouper.at(parenGrouperCounter).at(0) == parenGrouper.at(parenGrouperCounter + 1).at(0))
                    {
                        parenGrouperCounter ++;
                    }
                }
                else
                {
                    connect(connectorCMD.at(Conn_Counter - 1), currPrecBool, DoNotRunNextBool); // checks whether or not we should run the next ( )
                }
                RUN_AGAIN = !DoNotRunNextBool;
                 

                if (!DoNotRunNextBool)
                {
                    parenGrouperCounter++;
                }
                
                while (DoNotRunNextBool) // NEEDS TO SKIP UNTIL THE END OF THE RESPECTIVE '('
                {
                    // check if parenGrouper.at(parenGrouperCounter).at(0) == i
                        // if it is, the command we are trying to skip starts with a paranthesis
                        // if it isn't, there are commands before the paranthesis
                    if ( (parenGrouperCounter + 1 < parenGrouper.size()) && parenGrouper.at(parenGrouperCounter + 1).at(0) == i )
                    {
                        // command starts with paranthesis
                        // i = parenGrouper.at(parenGrouperCounter + 1).at(1);
                        // Conn_Counter = parenGrouper.at(parenGrouperCounter + 1).at(2);
                        // i and Conn_Counter are set at the proper place, we are trying to find what parenGrouperCounter should be set to
                       
                        
                        //check the vector ParenGrouper start element to see if there is anything after the one we need to skip and sets parenGrouperCounter to it
                        //if there is nothing after this parenethsis we need to skip Check_if_any_after_skip is false and parenGrouperCounter needs to be set to size of parenGrouper
                        bool Check_if_any_after_skip = false;
                        for( unsigned j = parenGrouperCounter + 1; j < parenGrouper.size(); j++)
                        {
                            if(parenGrouper.at(j).at(0) >= parenGrouper.at(parenGrouperCounter + 1).at(1)) // if the first command is >= the last command of the paranthesis we are not supposed to run
                            {
                                parenGrouperCounter = j;
                                Check_if_any_after_skip = true;
                                //we set j to this just so we break out once we find the first one
                                j = parenGrouper.size();
                            }
                            
                            i = parenGrouper.at(parenGrouperCounter).at(0); // first executable inside paranthesis
                            Conn_Counter = parenGrouper.at(parenGrouperCounter).at(0);
                        }
                        // very last paranthesis
                        if(!Check_if_any_after_skip)
                        {
                            unsigned temp = 1;
                            if(parenGrouper.size() > 2)
                            {
                                for(unsigned j = temp; j < parenGrouper.size(); j++)
                                {
                                    if(parenGrouper.at(parenGrouperCounter).at(1) == parenGrouper.at(j).at(1))
                                    {
                                        temp = j;
                                        j = parenGrouper.size();
                                    }
                                    else
                                    {
                                        temp = parenGrouper.size() - 1 ;
                                    }
                                }
                            }
                            
                            if (parenGrouper.at(parenGrouperCounter).at(1) ==  parenGrouper.at( temp ).at(1))
                            {   
                                i = parenGrouper.at(parenGrouperCounter).at(1);
                                Conn_Counter = parenGrouper.at(parenGrouperCounter).at(2);
                                parenGrouperCounter = parenGrouper.size();
                            }
                            else
                            {
                                i = parenGrouper.at(parenGrouperCounter).at(1) - 1;
                                Conn_Counter = parenGrouper.at(parenGrouperCounter).at(2) -1;
                                parenGrouperCounter = parenGrouper.size();
                            }
                            
                        }
                        // i = parenGrouper.at(parenGrouperCounter).at(0); // first executable inside paranthesis
                        if(i < executable.size())
                        {
                            connect(connectorCMD.at(Conn_Counter - 1), currPrecBool, DoNotRunNextBool);
                            RUN_AGAIN = !DoNotRunNextBool;
                        }
                        else
                        {
                            RUN_AGAIN = false;
                        }
                        DoNotRunNextBool = false; // TEST 2:53 pm
                        //if K is false then we never changed parenGroupCounter so that means there is no parenthesis after it however our parenthesis can be within one 
                        // ex: ( () ) if we run the function on this it will look until the end of the parenGrouper and since the inner most one
                        // will search the vector and not find any we know that it is the last parenthesis
                    }
                    
                    if (parenGrouperCounter < parenGrouper.size()-1)
                    {
                        DoNotRunNextBool = false;
                        // // TESTING
                        // // increment parenGroupCounter until it reaches the next parenGrouper, ex: if we had ( F && ( ( T ) ) || T), it should skip all the way until the || T
                        if (Conn_Counter < connectorCMD.size())
                        {
                            connect(connectorCMD.at(Conn_Counter), currPrecBool, DoNotRunNextBool); // GOOD
                            RUN_AGAIN = !DoNotRunNextBool;
                        }
                        else // this means that it is the very last command
                        {
                            if(RUN_AGAIN)
                            {
                                //now we got stuff we do work
                                //make sure excp u insert c_str
                                //make child process
                                if(executable[i] == "exit")
                                {
                                    exit(1);
                                }
                                if(executable[i] == "cp")
                                {
                                    execvpRun = copy_function(argumentlist.at(i));
                                }    
                                else if(executable[i] != "test")
                                {
                                    pid_t c_pid;//, pid;
                                    int status;
                                    //  char * args[2] = { "ls", NULL} ;
                                    c_pid = fork();
                                    if( c_pid < 0) // failed fork
                                    {
                                        perror("fork failed");
                                        exit(1);
                                    }
                            
                                    else if (c_pid == 0) // successful fork
                                    {
                                        
                                        
                                        char *A = new char( executable[i].length() +1);
                                        strcpy(A, executable[i].c_str());
                                        
                                        char* D = NULL;
                            
                                        if(A[0] != '\0')
                                        {
                                            D = A;
                                        }
                                        //**********************************************
                                        char *B = new char( argumentlist[i].length() +2);
                                        strcpy(B, argumentlist[i].c_str());
                            
                                        //**********************************************
                                        char* C = NULL;
                            
                                        if(B[0] != '\0')
                                        {
                                            C = B;
                                        }
                                        
                                    
                                        char* L[3] = {D, C, NULL};
                            
                                        
                                        // printf("Child: executing \n");       
                                        
                                        execvp(L[0], L);
                                
                                        perror("execve failed");
                            
                                        // delete A;
                                        // delete B;
                                        
                                        for(unsigned k = executable[i].length() +1; k >= 0; k--)
                                        {
                                            delete ( D + k);
                                        }
                                        for(unsigned k = argumentlist[i].length() +1; k >= 0; k--)
                                        {
                                            delete (C+ k);
                                        }
                                        
                                    }
                                    else if (c_pid > 0) // parent
                                    {
                                        wait(&status);
                                        // pid = wait(&status); // FIXME ERROR change how we call wait
                                                        
                                        if (!(WIFEXITED(status))) // check if execvp failed through waitid
                                        {
                                            execvpRun = false;
                                        }
                                        
                                        
                                        else
                                        {
                                            execvpRun = true;
                                            
                                        }
                                        
                                        if (executable.at(i) == "ls")
                                        {
                                            execvpRun = check_LS(argumentlist.at(i));
                                        }
                        
                                        // printf("Parent: finished\n");
                                    }
                                }
                                //test function have to figure out what to return
                                else
                                {
                                    struct stat buf;
                                    string test_arg = argumentlist[i];
                                    if((test_arg[0] == '-') && (test_arg[1] == 'f'))
                                    { 
                                      test_arg.erase(0,3);
                                      stat(test_arg.c_str(),&buf);
                                      if(S_ISREG(buf.st_mode) != 0 )
                                      {
                                        execvpRun = true;
                                        // cout << "file" << endl;
                                      }
                                      else
                                      {
                                        execvpRun = false;
                                        // cout << "not file" << endl;
                                      }
                                    }
                                    //-d check flag = checks if directory
                                    else if((test_arg[0] == '-') && (test_arg[1] == 'd'))
                                    {
                                      test_arg.erase(0,3);
                                      stat(test_arg.c_str(),&buf);
                                      if(S_ISDIR(buf.st_mode))
                                      {
                                        execvpRun = true;
                                        // cout << "folder" << endl;
                                      }
                                      else
                                      {
                                        execvpRun = false;
                                        // cout << "not folder" << endl;
                                      }
                                    }
                                    //-e checks for existence
                                    else 
                                    {
                                      if((test_arg[0] == '-') && (test_arg[1] == 'e'))
                                      {
                                        test_arg.erase(0,3);
                                      }
                                      if(stat(test_arg.c_str(),&buf) == 0 )
                                      {
                                        execvpRun = true;
                                        // cout << "exist" << endl;
                                      }
                                      else
                                      {
                                        execvpRun = false;
                                        // cout << "doesn't exist" << endl;
                                      }
                                    }
                                }
                            }
                            
                            if(RUN_AGAIN)
                            {
                                    if(executable[i] == "exit")
                                    {
                                        exit(1);
                                    }
                                }
                            return;
                    }
                    }
                }
            }
            else
            {
                if(RUN_AGAIN)
                {
                        //now we got stuff we do work
                        //make sure excp u insert c_str
                        //make child process
                        if(executable[i] == "exit")
                        {
                            exit(1);
                        }
                        else if(executable[i] == "cp")
                        {
                            execvpRun = copy_function(argumentlist.at(i));
                        }    
                        else if(executable[i] != "test")
                        {
                            pid_t c_pid;//, pid;
                            int status;
                            //  char * args[2] = { "ls", NULL} ;
                            c_pid = fork();
                            if( c_pid < 0) // failed fork
                            {
                                perror("fork failed");
                                exit(1);
                            }
                    
                            else if (c_pid == 0) // successful fork
                            {
                                
                                
                                char *A = new char( executable[i].length() +1);
                                strcpy(A, executable[i].c_str());
                                
                                char* D = NULL;
                    
                                if(A[0] != '\0')
                                {
                                    D = A;
                                }
                                //**********************************************
                                char *B = new char( argumentlist[i].length() +2);
                                strcpy(B, argumentlist[i].c_str());
                    
                                //**********************************************
                                char* C = NULL;
                    
                                if(B[0] != '\0')
                                {
                                    C = B;
                                }
                                
                            
                                char* L[3] = {D, C, NULL};
                    
                                
                                // printf("Child: executing \n");       
                                
                                execvp(L[0], L);
                        
                                perror("execve failed");
                    
                                // delete A;
                                // delete B;
                                
                                for(unsigned k = executable[i].length() +1; k >= 0; k--)
                                {
                                    delete ( D + k);
                                }
                                for(unsigned k = argumentlist[i].length() +1; k >= 0; k--)
                                {
                                    delete (C+ k);
                                }
                                
                            }
                            else if (c_pid > 0) // parent
                            {
                                wait(&status);
                                // pid = wait(&status); // FIXME ERROR change how we call wait
                                                
                                if (!(WIFEXITED(status))) // check if execvp failed through waitid
                                {
                                    execvpRun = false;
                                }
                                
                                
                                else
                                {
                                    execvpRun = true;
                                    
                                }
                                
                                if (executable.at(i) == "ls")
                                {
                                    execvpRun = check_LS(argumentlist.at(i));
                                }
                
                                // printf("Parent: finished\n");
                            }
                        }
                        //test function have to figure out what to return
                        else
                        {
                            struct stat buf;
                            string test_arg = argumentlist[i];
                            if((test_arg[0] == '-') && (test_arg[1] == 'f'))
                            { 
                              test_arg.erase(0,3);
                              stat(test_arg.c_str(),&buf);
                              if(S_ISREG(buf.st_mode) != 0 )
                              {
                                execvpRun = true;
                                // cout << "file" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "not file" << endl;
                              }
                            }
                            //-d check flag = checks if directory
                            else if((test_arg[0] == '-') && (test_arg[1] == 'd'))
                            {
                              test_arg.erase(0,3);
                              stat(test_arg.c_str(),&buf);
                              if(S_ISDIR(buf.st_mode))
                              {
                                execvpRun = true;
                                // cout << "folder" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "not folder" << endl;
                              }
                            }
                            //-e checks for existence
                            else 
                            {
                              if((test_arg[0] == '-') && (test_arg[1] == 'e'))
                              {
                                test_arg.erase(0,3);
                              }
                              if(stat(test_arg.c_str(),&buf) == 0 )
                              {
                                execvpRun = true;
                                // cout << "exist" << endl;
                              }
                              else
                              {
                                execvpRun = false;
                                // cout << "doesn't exist" << endl;
                              }
                            }
                        }
                    }
                    
                if(RUN_AGAIN)
                {
                        if(executable[i] == "exit")
                        {
                            exit(1);
                        }
                }
                return;
            }
        }
    }
    // skip here for the very last paranthesis
    stop:
    ;
}

void rshell()
{
  string command;
  while (command != "exit")
  {
        //1. Print a command prompt (e.g. $)
        cout << "$ ";
        
        //2. Read in a command on one line. Commands will have the form
        getline(cin, command);
        unsigned place = 0;
        
        do_work(command, place);
        
    }
}
   
int main()
{
    rshell();
    return 0;
}
