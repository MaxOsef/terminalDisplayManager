#!/bin/sh

session="dev_Ncurses" 

path=$1
projectName=$2

#start tmux server
tmux start-server

# create a new tmux session and windows. 
tmux new-session -d -s $session -n Code # default window #1

#select window 1
tmux select-window -t $session:1

tmux selectp -t 1
tmux splitw -h -p 10 #split first pane horizontaly
tmux selectp -t 1
tmux splitw -v -p 10 #split first pane verticaly
tmux selectp -t 3 
tmux splitw -v -p 10 #split first pane verticaly

tmux selectp -t 1
tmux send-keys "mkdir ~/Dev_Ncurses/projects/$path/$projectName" C-m C-l
tmux send-keys "cd ~/Dev_Ncurses/projects/$path/$projectName" C-m C-l
tmux send-keys "touch main.c" C-m C-l
tmux send-keys "vim main.c" C-m C-l

tmux selectp -t 2
tmux send-keys "htop" C-m C-l

tmux selectp -t 3
tmux send-keys "cd ~/Dev_Ncurses/projects/$path/$projectName" C-m C-l
tmux send-keys "ncbnr main.c $projectName"

tmux selectp -t 4 
tmux send-keys "watch sensors" C-m C-l

tmux selectp -t 1

tmux rename-session -t $session "dev_Ncurses_$projectName"

# Finished setup, attache to tmux session.
tmux attach-session -t $session
