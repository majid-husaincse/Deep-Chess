import json
import numpy as np
import copy  # use it for deepcopy if needed
import math  # for math.inf
import logging

logging.basicConfig(format='%(levelname)s - %(asctime)s - %(message)s', datefmt='%d-%b-%y %H:%M:%S',
                    level=logging.INFO)

# Global variables in which you need to store player strategies (this is data structure that'll be used for evaluation)
# Mapping from histories (str) to probability distribution over actions
strategy_dict_x = {}
strategy_dict_o = {}


class History:
    def __init__(self, history=None):
        """
        # self.history : Eg: [0, 4, 2, 5]
            keeps track of sequence of actions played since the beginning of the game.
            Each action is an integer between 0-8 representing the square in which the move will be played as shown
            below.
              ___ ___ ____
             |_0_|_1_|_2_|
             |_3_|_4_|_5_|
             |_6_|_7_|_8_|

        # self.board
            empty squares are represented using '0' and occupied squares are either 'x' or 'o'.
            Eg: ['x', '0', 'x', '0', 'o', 'o', '0', '0', '0']
            for board
              ___ ___ ____
             |_x_|___|_x_|
             |___|_o_|_o_|
             |___|___|___|

        # self.player: 'x' or 'o'
            Player whose turn it is at the current history/board

        :param history: list keeps track of sequence of actions played since the beginning of the game.
        """
        if history is not None:
            self.history = history
            self.board = self.get_board()
        else:
            self.history = []
            self.board = ['0', '0', '0', '0', '0', '0', '0', '0', '0']
        self.player = self.current_player()

    def current_player(self):
        """ Player function
        Get player whose turn it is at the current history/board
        :return: 'x' or 'o' or None
        """
        total_num_moves = len(self.history)
        if total_num_moves < 9:
            if total_num_moves % 2 == 0:
                return 'x'
            else:
                return 'o'
        else:
            return None

    def get_board(self):
        """ Play out the current self.history and get the board corresponding to the history in self.board.

        :return: list Eg: ['x', '0', 'x', '0', 'o', 'o', '0', '0', '0']
        """
        board = ['0', '0', '0', '0', '0', '0', '0', '0', '0']
        for i in range(len(self.history)):
            if i % 2 == 0:
                board[self.history[i]] = 'x'
            else:
                board[self.history[i]] = 'o'
        return board

    def is_win(self):
        x = []
        o = []
        for i in range(len(self.history)):
            if i%2 == 0:
                x.append(self.history[i])
            else:
                o.append(self.history[i])
        return ( (0 in x and 1 in x and 2 in x) or (3 in x and 4 in x and 5 in x) or (6 in x and 7 in x and 8 in x) or (0 in x and 3 in x and 6 in x) or (4 in x and 1 in x and 7 in x) or (5 in x and 8 in x and 2 in x) or (0 in x and 4 in x and 8 in x) or (4 in x and 6 in x and 2 in x) or (0 in o and 1 in o and 2 in o) or (3 in o and 4 in o and 5 in o) or (6 in o and 7 in o and 8 in o) or (0 in o and 3 in o and 6 in o) or (4 in o and 1 in o and 7 in o) or (5 in o and 8 in o and 2 in o) or (0 in o and 4 in o and 8 in o) or (4 in o and 6 in o and 2 in o) )


        # check if the board position is a win for either players
        # Feel free to implement this in anyway if needed

    def is_draw(self):
        if('0' not in self.board):
            return not self.is_win()
        return 0
        # check if the board position is a draw
        # Feel free to implement this in anyway if needed

    def get_valid_actions(self):
        arr = []
        for i in range(9):
            if(self.board[i] == '0'):
                arr.append(i)
        return arr
        # get the empty squares from the board
        # Feel free to implement this in anyway if needed

    def is_terminal_history(self):
        return (self.is_win() or self.is_draw())
        # check if the history is a terminal history
        # Feel free to implement this in anyway if needed

    def get_utility_given_terminal_history(self):
        if(self.is_draw()):
            return 0
        if(len(self.history)%2 == 1):
            return 1
        return -1
        # Feel free to implement this in anyway if needed
        pass

    def update_history(self, action):
        new = copy.deepcopy(self)
        new.board[action] = self.current_player()
        new.history = self.history + [action]
        new.player = new.current_player()
        return new



        # In case you need to create a deepcopy and update the history obj to get the next history object.
        # Feel free to implement this in anyway if needed


def backward_induction(history_obj):
    global strategy_dict_x, strategy_dict_o

    if(history_obj.is_terminal_history()):
        return history_obj.get_utility_given_terminal_history()
    actions = history_obj.get_valid_actions()
    history_str = ""
    for i in range(len(history_obj.history)):
        history_str = history_str + str(history_obj.history[i])
    child_utility = {}
    for i in actions:
        child_utility[i] = backward_induction(history_obj.update_history(i)) 
    if(history_obj.player == 'x'):
        x_score = 0
        for i in actions: 
            if(child_utility[i] == 1):
                x_score = x_score + 100
                if(history_str not in strategy_dict_x):
                    strategy_dict_x[history_str] = {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0,"6": 0, "7": 0, "8": 0}
                    strategy_dict_x[history_str][str(i)] = 1
        for i in actions:
            if(child_utility[i] == 0):
                x_score = x_score + 10
                if(history_str not in strategy_dict_x):
                    strategy_dict_x[history_str] = {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0,"6": 0, "7": 0, "8": 0}
                    strategy_dict_x[history_str][str(i)] = 1
        for i in actions:
            if(child_utility[i] == -1):
                if(history_str not in strategy_dict_x):
                    strategy_dict_x[history_str] = {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0,"6": 0, "7": 0, "8": 0}
                    strategy_dict_x[history_str][str(i)] = 1
        return (1 if x_score >= 100 else 0 if x_score >= 10 else -1)
    if(history_obj.player == 'o'):
        o_score = 0
        for i in actions:
            if(child_utility[i] == -1):
                o_score = o_score+100
                if(history_str not in strategy_dict_o):
                    strategy_dict_o[history_str] = {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0,"6": 0, "7": 0, "8": 0}
                    strategy_dict_o[history_str][str(i)] = 1
        for i in actions:
            if(child_utility[i] == 0):
                o_score = o_score + 10
                if(history_str not in strategy_dict_o):
                    strategy_dict_o[history_str] = {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0,"6": 0, "7": 0, "8": 0}
                    strategy_dict_o[history_str][str(i)] = 1
        for i in actions:
            if(child_utility[i] == 1):
                if(history_str not in strategy_dict_o):
                    strategy_dict_o[history_str] = {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0,"6": 0, "7": 0, "8": 0}
                    strategy_dict_o[history_str][str(i)] = 1
        return (-1 if o_score >= 100 else 0 if o_score >= 10 else 1)
        


    """
    :param history_obj: Histroy class object
    :return: best achievable utility (float) for th current history_obj
    """
    # TODO implement
    # (1) Implement backward induction for tictactoe
    # (2) Update the global variables strategy_dict_x or strategy_dict_o which are a mapping from histories to
    # probability distribution over actions.
    # (2a)These are dictionary with keys as string representation of the history list e.g. if the history list of the
    # history_obj is [0, 4, 2, 5], then the key is "0425". Each value is in turn a dictionary with keys as actions 0-8
    # (str "0", "1", ..., "8") and each value of this dictionary is a float (representing the probability of
    # choosing that action). Example: {”0452”: {”0”: 0, ”1”: 0, ”2”: 0, ”3”: 0, ”4”: 0, ”5”: 0, ”6”: 1, ”7”: 0, ”8”:
    # 0}}
    # (2b) Note, the strategy for each history in strategy_dict_x and strategy_dict_o is probability distribution over
    # actions. But since tictactoe is a PIEFG, there always exists an optimal deterministic strategy (SPNE). So your
    # policy will be something like this {"0": 1, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0, "6": 0, "7": 0, "8": 0} where
    # "0" was the one of the best actions for the current player/history.
    # TODO implement


def solve_tictactoe():
    backward_induction(History())
    with open('./policy_x.json', 'w') as f:
        json.dump(strategy_dict_x, f)
    with open('./policy_o.json', 'w') as f:
        json.dump(strategy_dict_o, f)
    return strategy_dict_x, strategy_dict_o


if __name__ == "__main__":
    logging.info("Start")
    solve_tictactoe()
    logging.info("End")
