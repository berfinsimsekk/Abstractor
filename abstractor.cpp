/*
*@author Berfin Şimşek
*
*This project creates a multithreaded scientific search engine
*that queries the paper abstracts and summarizes the most relevant ones.
*This program uses the Jaccard Similarity metric to determine the
*similarity between two texts.
*
*/





#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <queue>
#include <bits/stdc++.h>
#include <algorithm>
#include <cstring>
#include <pthread.h>
#include <map>
#include <unistd.h>
#include <string.h>

using namespace std;


vector< pair <string,bool> > absname2; //stores abstract files' names and whether they are calculated
ofstream outfile; //outfile
unordered_set<string> queryset; //stores the words to query
vector<  pair<string, pair<bool, string> >  > result; //stores abstract name, whether it is calculated or not and the sentences that have common words in the file
multimap<float,string, greater<float> > names; //stores the score and the file name of the abstract


pthread_mutex_t mutex1;

struct thread_data { //to send the thread id to the function as argument
   int  thread_id;
};


/*
* This function takes file name as parameter then it splits the content of the file 
* to sentences. It checks every sentence to find a common word with the query. If there is, it stores the sentences
*in the global result vector. 
*/

void sentence(string filename){

  
  ifstream myfile;
  string path = "../abstracts/" + filename;

  myfile.open(path);
  string s="";
  string line;

  while(getline(myfile,line)){

    stringstream ss(line); 
      string text;
   
      while (getline(ss, text, '\n')){ //splits the file with respect to new lines 
        s = s+ text;
      } 

  }
  string line2;
  istringstream iss(s);
 

  int fileindex=-1;
  while (getline(iss, line2, '.')) { //splits the lines with respect to dots
  
    if (!line2.empty()){
      
      line2=line2+". ";
      bool st=false;
      unordered_set<string> :: iterator itr3;

       
       for (itr3 = queryset.begin(); itr3 != queryset.end(); itr3++){ //iterates through the query set
          if(st==true){
            break;
          }
          string key= " " + (*itr3)+ " "; 
          size_t found = line2.find(key);
          if(found!= string::npos){  //if key is in the sentence

              for(int i =0; i<result.size(); i++){

                      if(result[i].first == filename && result[i].second.first == false){ //if the file is not calculated
                        fileindex=i;
                        if(line2[0]==' '){
                         line2=line2.substr(1);
                        }

                        result[i].second.second = result[i].second.second + line2; 
                        st=true;
                        break;
                      }
              
                
              }


          }
        }

      }
    }
    if(fileindex!=-1){ 
      result[fileindex].second.first = true; //file is calculated
    }
  }


/*
* This function tokenize the files' content, finds the intersection and union with the query and calculates the 
* the score with respect to Jaccard Similarity.
*/
  

void jaccard(string filename){

  
  unordered_set<string> tokens; //words are stored in this set
  
  ifstream myfile;
  string path = "../abstracts/" + filename;
  myfile.open(path);
  string line;
    while( getline(myfile, line) )
   {
      stringstream ss(line); 
      string token;
   
      while (getline(ss, token, ' ')){ //tokenizes the sentences with delimeter white space
         tokens.insert(token);  
      }   
     
   }
   

  queue<string> intersection; //intersecting words of query and file
  queue<string> uni; //union of query and file

  unordered_set<string> :: iterator itr2;
  for (itr2 = tokens.begin(); itr2 != tokens.end(); itr2++){
      uni.push((*itr2));
    }

  unordered_set<string> :: iterator itr;
    for (itr = queryset.begin(); itr != queryset.end(); itr++){

      string key=(*itr);
      if (tokens.find(key) != tokens.end()){ //whether the word is common in two sets
        intersection.push(key);
      }else{
        uni.push(key);
      }
       
    }
    pthread_mutex_lock(&mutex1); 
   float result2 = (float)intersection.size()/uni.size(); //result is calculated
   result.push_back(make_pair(filename, make_pair(false, ""))); 
   sentence(filename);
   names.insert(pair<float, string>(result2, filename));
 
   pthread_mutex_unlock(&mutex1);


}

/*
* This function is called by the threads. It takes the thread id as parameter and calls the jaccard
*function in order to calculate score.
*/

void* forthreads(void *threadarg){

  struct thread_data *my_data;
  my_data = (struct thread_data *) threadarg;

  for(int i=0; i<absname2.size(); i++){
    if(absname2[i].second==false){ //if the file is not calculated
      absname2[i].second=true;
      string filename;
  
  pthread_mutex_lock(&mutex1);
  filename =absname2[i].first;
  outfile<<"Thread "<<(char)(my_data->thread_id+65)<<" is calculating "<<filename<<endl;
  pthread_mutex_unlock(&mutex1);
  
  
  jaccard(filename);
    }
  
  
  }
  

}



int main(int argc, char const *argv[]) {
  
  ifstream fin;
  fin.open(argv[1]);

  string firstline;
  getline(fin, firstline);
  vector <string> tokens; 
  stringstream check1(firstline);
  string intermediate;
  
  while(getline(check1, intermediate, ' ')) //tokenizing the first line with respect to space
    {
        tokens.push_back(intermediate);
    }
  int thrnum= stoi(tokens[0]); // Number of threads
  int absnum= stoi(tokens[1]); // Number of abstracts that will be scanned by the threads
  int retnum= stoi(tokens[2]); // Number of abstracts that will be returned and summarized
  
  struct thread_data td[thrnum];
  
  pthread_mutex_init(&mutex1, NULL);
  
  string query;
  getline(fin,query);
  string l;
  stringstream aa(query); 
   string temp_str;

   while(getline(aa, temp_str, ' ')){ //tokenizes the query with respect to space and stores it in a set
      queryset.insert(temp_str);
   }
  

  for(int i=0; i<absnum;i++){
    getline(fin,l);
    absname2.push_back(make_pair(l,false)); //stores the abstract files' names and whether it is calculated or not in a set
  }
    fin.close();
  
    outfile.open(argv[2]);

    pthread_t th[thrnum];
    int i;
    
      for (i = 0; i < thrnum; i++) {
       
        
          td[i].thread_id = i;
          
           if (pthread_create(th + i, NULL, &forthreads, (void *)&td[i]) != 0) { //creating threads
            perror("Failed to create thread");
            return 1;
           }
       }
    
    
      for (i = 0; i < thrnum; i++) { //joining threads
          pthread_join(th[i], NULL);
      }   
  multimap<float,string>::iterator itr;
  int stop=0;
  
  int num=1;
    outfile<<"###"<<endl;

    /*
    * Iterates through the names map and prints the scores, file names and summary into output file
    */
   for( itr = names.begin(); itr != names.end(); ++itr){
      if(stop==retnum){
        break;
      }
    
      outfile<<"Result "<<num<<":"<<endl;
      outfile<<"File: " <<itr->second<<endl;
      outfile<<"Score: " << fixed<<setprecision(4)<<itr->first << endl;

      for(int i =0; i<result.size(); i++){

        if(result[i].first == itr->second){
          outfile<<"Summary: " << result[i].second.second << endl;  
          break;
        }
              
                
      }

          stop++;
          num++;

      outfile<<"###"<<endl;
      } 

    
        
  
 pthread_exit(NULL);




  return 0;
}