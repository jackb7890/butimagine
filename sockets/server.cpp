#include "winsock2.h"

int main() {

    // create server socket

    bool runLoop = true;

    while (runLoop) {
        // listen on the port for a client, loop until you find one.
        // I guess there is some way in the socket functions to tell its the client from our game
        // maybe we can hardcode a passcode in the client, and pass that on its port to this server socket

        // once you have a connection,
        // wait for it to send you data
        // probably just keyboard input for now

        // once you have the keyboard input,
        // we can use the event processing code thats in the eventful-walls branch
        // + Map, Player, and Wall code in the main branch to move a player on the map, and then return the map back to the client

        // then the client can display the map, and send another keyboard input to the server and repeat

        // server.cpp will have the map, player, and wall objects to keep track of the state of the game
        // it will also have the logic to parse the keyboard input from the client
        // then it will send back a the new state of the game for the player to display

        // ORRRRRRR

        // i just got different idea
        // we could have the map, player, walls, etc on the client side and the server side
        // so when we do like WASD on the client to move around, we could
            // 1) process the keyboard input on the client side
            // 2) update the map with the new player location
            // 3) send the keyboard input to the server as well
            // 4) and the server can use the keyboard input to make sure its version of the map
            //      is updated in the same way
            // 5) The server will only have to send this data to other clients
            //      so that they are aware that another client has moved
            // 
            // I like this because the size of the data being transferred is a lot smaller
            // in the first one, its the entire world map, every keystroke
            // in this one, its just the single keystroke
    }

    return 0;
}