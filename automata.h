// TODO: Solve error when using true_type and false_type constructors simultaneously

#ifndef AUTOMATA_H
#define AUTOMATA_H

#include <iostream>
#include <iomanip>

#include <vector>
#include <map>
#include <set>
#include <string>
#include <stack>

#include "state.h"
#include "transition.h"

using namespace std;

struct Traits {
    typedef string S;
    typedef int T;
    typedef int Al; // Alphabet
};

template <typename Tr>
class Automata{
    public:
        typedef typename Tr::S S;
        typedef typename Tr::T T;
        typedef typename Tr::Al Al;
        typedef Automata<Tr> self;
        typedef State<self> state;
        typedef Transition<self> transition;
        typedef map<S, state*> StateSeq;
        typedef set<S> FinalStateSeq;
        typedef vector<transition*> TransitionSeq;
        typedef typename StateSeq::iterator StateIte;
        typedef typename TransitionSeq::iterator TransitionIte;

    private:
        StateSeq states; // Map: <key, value> = <state.name, *state>
        FinalStateSeq finalStates;
        S initialState="-";
        int sizeOfAutomata[2] = {0,0}; // [0]: # states -  [1]: # transitions
        set<Al> alphabet = {0,1};
        bool default_alphabet=true;

        // Auxiliary methods
        void addTransition(transition sometransition, bool inverse=false);
        TransitionIte removeTransition(state* sinitial, state* sfinal, int symbol);
        bool setStateType(S state_name, int new_type);
        void printDefaultAlphabet();
        void printAnyAlphabet();

    public:
        // Constructors and destructor
        Automata(){};
        Automata(int size);
        Automata(StateSeq somestates, bool inverse=false); // Automata from vector of states
        Automata(const Automata &other); // copy constructor
        ~Automata();

        // Debugging methods
        void print();
        void formalPrint();

        // Access methods
        int* size();

        // Modifiers
        bool addState(S state_name, bool isInitial=false, bool isFinal=false);
        bool addTransition(S sinitial, S sfinal, T symbol);

        bool removeState(S state_name);
        bool removeTransition(S sinitial_name, S sfinal_name, int symbol);

        bool makeInitial(S state_name);
        bool makeFinal(S state_name);
        bool makeIntermediate(S state_name);
        void clearTypes();

        void setAlphabet(vector<Al> alpha);

        // Overloads
        self& operator=(const self& other);

        // Extras
        bool validateAFD();

        // Algorithms
        self* transpuesto();
        self AFNtoAFD();
        void Brzozowski();
};
typedef Automata<Traits> automata;


/*
Explicit template specializations
(explicits specialization must go before implicit)
*/


template<>
automata::Automata(int size) {
    sizeOfAutomata[0] = size;
    state* newstate;
    for (int i=0;i<=size;++i){
        newstate=new state(string(1, i+65));
        states.insert(pair <S, state*> (string(1, i+65), newstate));
    }
};

template<>
automata::~Automata(){
    auto it = states.begin();
    while (!states.empty()){
        delete (*it).second;
        it = states.erase(it);
    }
};

template<>
automata::self& automata::operator=(const self& other){
    for (auto& thestate: other.states){
        state* pstate = new state(*thestate.second);
        states.insert(pair<S, state*> (thestate.first, pstate));
    }
    sizeOfAutomata[0] = other.sizeOfAutomata[0];
    sizeOfAutomata[1] = other.sizeOfAutomata[1];
    default_alphabet = other.default_alphabet;
    alphabet = other.alphabet;
    return *this;
};

template<>
automata::TransitionIte automata::removeTransition(state* sinitial, state* sfinal, int symbol){
    automata::TransitionIte it;
    for (auto it_transition = sinitial->transitions.begin(); it_transition!=sinitial->transitions.end(); ++it_transition){
        if ((*it_transition)->states[1]==sfinal && (*it_transition)->getSymbol()==symbol) {
            delete *it_transition; // TODO: delete after erase causes segmentation fault. Why?
            it = sinitial->transitions.erase(it_transition);
            return it;
        }
    }
    return sinitial->transitions.end();
};

template<>
bool automata::addState(S state_name, bool isInitial, bool isFinal){
    if (states.find(state_name)!=states.end()) return false; // getName taken
    state* newstate = new state(state_name, isInitial, isFinal);
    states.insert(pair<S, state*> (state_name, newstate));
    ++sizeOfAutomata[0];
    return true;
};


/*
Implicit template specializations
*/

template<>
automata::Automata(StateSeq somestates, bool inverse){
     // Automata from vector of states
    for (auto& pairStates : somestates) {
        bool temp_initial=false, temp_final=false;
        if (inverse) {
            if (pairStates.second->isInitial){
                temp_final=true;
                finalStates.insert(pairStates.first);
            }
            if (pairStates.second->isFinal) temp_initial=true;
        }
        addState(pairStates.first, temp_initial, temp_final);
    }
}


template<>
automata::Automata(const Automata &other) {
    // copy constructor
    *this = other;
};


template<>
bool automata::removeTransition(S sinitial_name, S sfinal_name, int symbol){
    if (states.find(sinitial_name)==states.end() || states.find(sfinal_name)==states.end()) return false; // not found
    state* sinitial = states[sinitial_name];
    state* sfinal = states[sfinal_name];

    if (!*removeTransition(sinitial, sfinal, symbol)) return false;

    --sizeOfAutomata[1];
    return true;
};

// Destructor above

template<>
void automata::printDefaultAlphabet(){
    cout << "\n Alphabet |";
    for (auto& symbol: alphabet) cout << "\t"<< symbol << "\t" << "|";
    cout<<"\n----------|---------|---------|";
    for (auto& thestates : states){
        cout <<endl;
        if (thestates.second->isFinal) cout << "*";
        else cout << " ";
        if (thestates.second->isInitial) cout << "->";
        else cout << "  ";

        cout<< "State " << thestates.first << " |";
        auto ittrans=thestates.second->transitions.begin();
        while (! (ittrans==thestates.second->transitions.end() || (*ittrans)->getSymbol()!=0)){
            cout <<" " <<(*ittrans)->states[1]->getName();
            ++ittrans;
        }
        cout << "\t|";
        while (! (ittrans==thestates.second->transitions.end() || (*ittrans)->getSymbol()!=1)){
            cout <<" " <<(*ittrans)->states[1]->getName();
            ++ittrans;
        }
        cout << "\t|";
    }
};

template<>
void automata::printAnyAlphabet(){
    cout << "\nAlphabet: ";
    for (auto& symbol: alphabet) cout << symbol << " ";
    for (auto& thestates : states){
        cout<< "\nState " << thestates.first << ": ";
        for (auto& thetransition: thestates.second->transitions){
            cout << "["<< thetransition->getSymbol() << "]->";
            cout << thetransition->states[1]->getName() <<"  ";
        }
    }
};

template<>
void automata::print(){
    if (!sizeOfAutomata[0]) return; // empty
    if (default_alphabet) printDefaultAlphabet();
    else printAnyAlphabet();

};

template<>
void automata::formalPrint(){
    cout << sizeOfAutomata[0] <<" "<< initialState <<" "<< finalStates.size();
    for (auto& thefinalstate : finalStates) cout <<" "<< thefinalstate;
    for (auto& thestate : states)
        for (auto& thetrans : thestate.second->transitions)
            cout << endl <<thetrans->states[0]->getName() <<" "<< thetrans->getSymbol()<<" "<<thetrans->states[1]->getName();
}

template<>
int* automata::size(){ return sizeOfAutomata; };

// addState() above

template<>
bool automata::addTransition(S sinitial, S sfinal, T symbol){
    // Check if states exist
    if (!(states.count(sinitial) && states.count(sfinal)) || alphabet.find(symbol)==alphabet.end())
        return false;

    state* initial_state=states[sinitial];
    state* final_state=states[sfinal];

    transition* new_transtition = new transition(initial_state,final_state,symbol);
    auto transition_in_transitions = initial_state->transitions.begin();

    // Keep transitions in order
    while (transition_in_transitions!=initial_state->transitions.end() && *new_transtition>**transition_in_transitions) ++transition_in_transitions;

    if (initial_state->transitions.empty() || *transition_in_transitions==*initial_state->transitions.end()) initial_state->transitions.push_back(new_transtition); // el nuevo transition debe ir al final
    else if (*new_transtition==**transition_in_transitions) return false; // repeated transition
    else initial_state->transitions.insert(transition_in_transitions, new_transtition);

    ++sizeOfAutomata[1];
    return true;
};

template<>
void automata::addTransition(transition sometransition, bool inverse){
    int a=0, b=1;
    if (inverse) a=1, b=0;
    addTransition(sometransition.states[a]->getName(), sometransition.states[b]->getName(), sometransition.getSymbol());
};

// removeTransition() above

template<>
bool automata::removeState(S state_name){
    if (states.find(state_name)==states.end()) return false; // not found
    state* to_remove = states[state_name];

    auto it_transition = to_remove->transitions.begin();
    while (it_transition!=to_remove->transitions.end()){
        it_transition = removeTransition((*it_transition)->states[0], (*it_transition)->states[1], (*it_transition)->getSymbol());
    }
    delete to_remove;
    states.erase(state_name);


    --sizeOfAutomata[0];
    return true;
};

template<>
bool automata::setStateType(S state_name, int new_type){
    if (states.find(state_name)==states.end() || // not found
        (states[state_name]->isInitial && new_type==1) ||
        (states[state_name]->isFinal && new_type==2)) return false; // already set
    switch (new_type){
        case 0: states[state_name]->isInitial=false; states[state_name]->isFinal=false; break;
        case 1: states[state_name]->isInitial=true; break;
        case 2: states[state_name]->isFinal=true;
    }
    return true;
};

template<>
bool automata::makeInitial(S state_name){
    for (auto& thestate: states)
        if (thestate.second->isInitial==true) { thestate.second->isInitial=false; break; }

    if (!setStateType(state_name, 1)) return false;
    return true;
};

template<>
bool automata::makeFinal(S state_name){
    if (!setStateType(state_name, 2)) return false;
    finalStates.insert(state_name);
    return true;
};

template<>
bool automata::makeIntermediate(S state_name){
    if (states[state_name]->isInitial) initialState="-";
    if (states[state_name]->isFinal) finalStates.erase(state_name);
    return setStateType(state_name, 0);
};

template<>
void automata::clearTypes(){
    for (auto& thestate: states) thestate.second->isFinal=thestate.second->isInitial=false;
    initialState = "-";
    finalStates.clear();
};


template<>
void automata::setAlphabet(vector<Al> alpha){
    copy(alpha.begin(), alpha.end(), inserter(alphabet, alphabet.end()));
    default_alphabet = false;
};

template<>
bool automata::validateAFD(){
    return sizeOfAutomata[1]==sizeOfAutomata[0]*alphabet.size() &&
           initialState!="-" && finalStates.size()>0;
}

template<>
automata::self* automata::transpuesto(){
  self* transpuesto = new self(states, 1);
  for (auto& thestate : states){
    for (auto& thetransition : (thestate.second)->transitions){
      transpuesto->addTransition(*thetransition, true);
    }
  }
  transpuesto->print();
  return transpuesto;
}

template<>
automata::self automata::AFNtoAFD(){
  self* transpuesto = this->transpuesto();
  self AFD;

  AFD.addState("-");
  AFD.addTransition("-", "-", 0);
  AFD.addTransition("-", "-", 1);
  vector<string> list;
  for (auto& thestate : transpuesto->states){
    if ((thestate.second)->isInitial==true && AFD.states.find(thestate.first)==AFD.states.end()){

      string superstate0=thestate.first;
      string superstate1=thestate.first;
      bool existe0=true;
      bool existe1=true;
      int count=0;

      AFD.addState(thestate.first, true);
      list.push_back(thestate.first);
      while (!list.empty()){
        count++;
        bool aux1=false;
        bool aux0=false;
        bool fin0=false;
        bool fin1=false;
        existe0=false;
        existe1=false;
        superstate0.clear();
        superstate1.clear();

        string top=list.front();

        for (int i=0; i<top.length(); i++){
          for (auto& thestate2 : transpuesto->states){
            char aaa=top[i];
            string letraa(1,aaa);
            if (letraa==thestate2.first){
              for (auto& thetransition : (thestate2.second)->transitions){
                if ((thetransition)->getSymbol()==1){
                  superstate1+=thetransition->states[1]->getName();
                  existe1=true;
                  if (thetransition->states[1]->isFinal){
                    fin1=true;
                  }
                }
                if ((thetransition)->getSymbol()==0){
                  superstate0+=thetransition->states[1]->getName();
                  existe0=true;
                  if (thetransition->states[1]->isFinal){
                    fin0=true;
                  }
                }
              }
            }
          }
        }
        sort(superstate0.begin(),superstate0.end());
        sort(superstate1.begin(),superstate1.end());
        if (AFD.states.find(superstate0)==AFD.states.end() && existe0){
          AFD.addState(superstate0, false, fin0);
          list.push_back(superstate0);
        }
        if (AFD.states.find(superstate1)==AFD.states.end() && existe1){
          AFD.addState(superstate1, false, fin1);
          list.push_back(superstate1);
        }
        if (existe0){
          AFD.addTransition(list.front(), superstate0, 0);
        }
        else{
          AFD.addTransition(list.front(), "-", 0);
        }
        if (existe1){
          AFD.addTransition(list.front(), superstate1, 1);
        }
        else{
          AFD.addTransition(list.front(), "-", 1);
        }
        list.erase(list.begin());
      }
    }
  }
  AFD.print();
  return AFD;
}

template<>
void automata::Brzozowski(){
  transpuesto();
  AFNtoAFD();
  transpuesto();
  AFNtoAFD();
}


#endif
