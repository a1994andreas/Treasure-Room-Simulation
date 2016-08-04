# Treasure-Room-Simulation

On this game there is a treasure room (server) and two teams (clients), team A and team B.The perpuse of this game is to pick all the coins from the treasure room.

You insert the number of the coins available on the console
  example:  ./server 200
  
The server creates a pointer on a shared memory and then each team attempt to enter.The client generate two random numbers between 1-10. the first number is the time the team waits until they attempt to enter the room and the second number is the seconds they will stay in the room. 
For each second a team is in the room they pick a coin.
Only one team can be in the room at a time.

Treasure_room:
- Generate_key ()
- Room key is the shared memory segment key
While (coins > 0 )
- Listen teams attempting to enter room
 - If anyone makes it, pass the key
 - Check number of coins
Say who the winner is and end game.


Team_a , team_b:
- Αsk for key ( first time only )
- If room is empty attempt to enter
lock room
coins --
 unlock room
 else -> wait
