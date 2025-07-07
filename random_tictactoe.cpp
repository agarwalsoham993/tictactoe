#include<iostream>
#include<ctime>
#include<cstdlib>

using namespace std;

int game_draw(int (*values)[9]){
    for(int i = 0;i<9;i++){
        if((*values)[i]==2){
            return 0;
        }
    }
    return 1;
}

int check_grid(int (*values)[9]){

    for(int i=0;i<3;i++){ // row check
        if ((*values)[i*3]!=2 && (*values)[i*3]==(*values)[i*3+1] && (*values)[i*3+2]==(*values)[i*3+1]){
            if((*values)[i*3]==0) return 0; //player 0 wins
            else return 1;
        }
    }

    for(int i=0;i<3;i++){ // column check
        if ((*values)[i]!=2 && (*values)[i]==(*values)[i+3] && (*values)[i]==(*values)[i+6]){
            if((*values)[i]==0) return 0; //player 0 wins
            else return 1;
        }
    }

    if ((*values)[0]!=2 && (*values)[0]==(*values)[4] && (*values)[0]==(*values)[8]){ // \diagonal
        if((*values)[0]==0) return 0; //player 0 wins
        else return 1;
    }

    if ((*values)[4]!=2 && (*values)[2]==(*values)[4] && (*values)[4]==(*values)[6]){ // /diagonal
        if((*values)[0]==0) return 0; //player 0 wins
        else return 1;
    }

    if (game_draw(values)==1) return 3;
    return 2;
}

void print_grid(int (*value)[9]){
    string print1;string print2;string print3;
    for(int i=1;i<=12;i++){
        if(i%4==1 || i%4==3){
            cout << "     |     |     ";
        }
        else if (i%4==2){
            if((*value)[(i/4)*3]==2) print1 = " ";
            else if ((*value)[(i/4)*3]==0) print1 = "O";
            else print1 = "X";
            if((*value)[(i/4)*3+1]==2) print2 = " ";
            else if ((*value)[(i/4)*3+1]==0) print2 = "O";
            else print2 = "X";
            if((*value)[(i/4)*3+2]==2) print3 = " ";
            else if ((*value)[(i/4)*3+2]==0) print3 = "O";
            else print3 = "X";

            cout << "  " << print1 << "  " << "|" << "  " << print2 << "  " << "|" << "  " << print3;
        }
        else if(i%4==0 && i/4!=3){
            cout << "_ _ _ _ _ _ _ _ _";
        }
        cout << "\n";
    }
}

void take_user_input(int (*values)[9],int n){
    int i;
    cout << "enter row number(1,2,3,4,5,6,7,8,9) =";
    cin >> i;

    if((*values)[i-1]==2) (*values)[i-1] = n;
    else take_user_input(values,n);
    print_grid(values);
    return;
}

int randint(int j,int k){
    int i;
    i = (rand()%(k-j)+j); 
    return i;
}

int to_win(int (*values)[9],int n){ // n by which we play , if 0 we play 0

    int empty[9], count = 0;
    for(int i = 0; i < 9; i++) {
        if((*values)[i] == 2) empty[count++] = i;
    }

    int values_copy[9];

    for (int i=0;i<count;i++){
        for(int j =0;j<9;j++){
            values_copy[j] = (*values)[j];
        }
        values_copy[empty[i]]=n;
        if (check_grid(&values_copy)==n) return empty[i];
    }
    return 100;
}

void random_move(int (*values)[9],int n){
    int t = to_win(values,0);
    if(t!=100){    // if can win ,then winning is firstly imp 
        (*values)[t] = 0;
        return;
    }
    t = to_win(values,1);
    if(t!=100){ 
        (*values)[t] = 0;
        return;
    }
    int empty[9], count = 0;
    for(int i = 0; i < 9; i++) {
        if((*values)[i] == 2) empty[count++] = i;
    }
    if(count == 0) return;
    
    int ind = empty[randint(0, count)];
    (*values)[ind] = n;
    return;
}

void play_random(int (*values)[9]){
    print_grid(values);
    int t = check_grid(values);
    if(t==0){
        cout << "\n" << "bot wins" << "\n";
        return;
    }
    else if (t==3){
        cout << "\n" << "game draw" << "\n";
        return;
    }
    take_user_input(values,1);
    t =check_grid(values);
    if(t==1){
        cout << "\n" << "you win" << "\n";
        return;
    }
    else if(t==3){
        cout << "\n" << "game draw" << "\n";
        return;
    }
    random_move(values,0);
    play_random(values);
}

int both_random(int (*values)[9]){
    int t = check_grid(values);
    if(t!=2){
        return t;
    }
    random_move(values,1);
    t =check_grid(values);
    if(t!=2){
        return t;
    }
    random_move(values,0);
    return both_random(values);
}

int main(){
    srand((unsigned)time(0)); 
    // int values[9] = {2,2,2,2,2,2,2,2,2};
    // int values[9] = {2,2,0,2,2,0,2,2,2};
    int win_x = 0;int lose_x =0;int draw = 0;int t;
    for(int i=0;i<1000;i++){
        int values[9] = {2,2,2,2,2,2,2,2,2}; // <-- Reset here for each game
        t = both_random(&values);
        if(t==1) win_x++;
        else if (t==0) lose_x++;
        else if (t==3) draw++;
        // print_grid(&values);
    }

    cout << "\n no of wins who started" << win_x << "\n no of loss who started" << lose_x << "\n no of draws" << draw << "\n";
}
