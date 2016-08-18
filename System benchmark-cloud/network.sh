#!/bin/sh

./client tcp $1 45002 1 1
./client tcp $1 45002 1024 1
./client tcp $1 45002 65500 1
./client tcp $1 45002 1 2 
./client tcp $1 45002 1024 2 
./client tcp $1 45002 65500 2 
./client tcp $1 45002 1 4 
./client tcp $1 45002 1024 4  
./client tcp $1 45002 65500 4 
./client udp $1 45002 1 1
./client udp $1 45002 1024 1
./client udp $1 45002 65500 1
./client udp $1 45002 1 2 
./client udp $1 45002 1024 2 
./client udp $1 45002 65500 2 
./client udp $1 45002 1 4 
./client udp $1 45002 1024 4  
./client udp $1 45002 65500 4 
