// Tic tac toe game 
the initial version #random_tictactoe been implemented to encourage any possible winning in the next move avoid losing there.
{it becomes still beatable when there comes two positions for the losing situation}

the next version #normal_tictactoe is a more enhanced version of the random version
* weights allocation : instead of choosing in random from all the empty places we will find the preferable empty places and call it the best empty places
* Weights allocation {reverse biasing} : this update in weights leads to prefer the places where even oponent would want to land in the next move {from our best_empty places} calling them the best places
* lookout weights :  this leads to one step ahead biasing { from best places we will shortlist those which will minimize the max(weights) allocated which will avoid possible winning in next to next move}

  the pattern building is faster and effecient than any brute force method/
![image](https://github.com/user-attachments/assets/d58c4683-4576-4560-bc66-f478b4ec3b85)
Reviewed and tested the patterns for bot's moves  , Penalty of 30ms while reducing the loss of the beginner by 100%.

optimal_version :
![image](https://github.com/user-attachments/assets/da852dd6-8692-4f74-bd31-7da6535f8847)
