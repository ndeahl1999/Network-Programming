#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

int correct_characters(const string answer, const string guess){

  map<char, int> answer_chars = map<char, int>();

  // add the counts of all characters to the map
  for(int i=0;i<answer.length();i++){
    answer_chars[answer[i]]+=1;
  }

  int count=0;

  // for every character, if it's in the map
  // increment count, and remove 1 from the map
  for(int i=0;i<guess.length();i++){
    if(answer_chars[guess[i]] != 0){
      count++;
      answer_chars[guess[i]]--;
    }
  }
  /*

  INITIAL IMPLEMENTATION, WRONGkk
  set<char> guess_chars = set<char>();

  // add the chars of guess to a set, keeping only uniques
  for(int i=0;i<guess.length();i++){
    guess_chars.insert(guess[i]);
  }

  int count =0;
  // loop through set, and if add to the count
  // the value at key c
  for(char c: guess_chars){
    count+=answer_chars[c];
    cout<<"count is "<< count <<endl;
  a

*/

  return count;
}



int correct_placement(string answer, string guess){
  if(answer.length() != guess.length()){
    cout<<"not the same length"<<endl;
    return -1;
  }

  int count = 0;

  // count up the amount of same letters it has
  for(int i=0;i<answer.length();i++){
    if(answer[i] == guess[i]){
      count++;
    }
  }

  return count;
}


// wrapper function for the print statement
// about correct characters and placement
void new_guess(string answer, string guess){

  int correct_chars = correct_characters(answer, guess);

  int correct_place = correct_placement(answer, guess);

  cout<<"xxx guessed " << guess << ": "<< correct_chars
    <<" letter(s) were correct and " << correct_place<< " letter(s)"
    << " were correctly placed."<<endl;
 
}


void read_dictionary(){



}
int main(int argc, char** argv){

  // string answer = "guess";
  string guess = "cross";

  int seed = atoi(argv[1]);
  string dic_file = argv[2];
  int max_length = atoi(argv[3]);

  cout<< "seed is " << seed<< " dict is "<<dic_file<<endl;


  ifstream infile(dic_file);
  string line;
  vector<string> words;

  // read in all the words
  while(infile >> line){
    words.push_back(line);
  }

  srand(seed);


  string answer = words[rand() % words.size()];


  // sort the read in list of words
  sort(words.begin(), words.end());



  

  new_guess(answer, guess);

  // do error checking here to see if the lenght of the guess 
  //word is differnt from the answer word
  

}
