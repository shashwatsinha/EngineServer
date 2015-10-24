#include <SDL/SDL.h>
#include <GL/glew.h>
#include <SDL\SDL_net.h>
#include <cstring>
#include<vector>
#include <iostream>
using namespace std;

struct data
{
	TCPsocket socket;
	Uint32 timeout;
	int id;
	data(TCPsocket sock, Uint32 t, int i)
	{
		socket = sock;
		timeout = t;
		id = i;
	}
};

int main(int argc, char **argv)
{
	//Initialize SDL Net
	SDLNet_Init();

	TCPsocket server;
	SDLNet_SocketSet sockets;
	int curID;//ID of the player
	int playerNum;//total number of player connected
	std::vector<data> socketVector;
	SDL_Event event;
	IPaddress ip;
	char tmp[1400];
	curID = 0;//ID of the player
	playerNum = 0;//total number of player connected


	SDLNet_ResolveHost(&ip, NULL, 1234);
	bool running = true;
	sockets = SDLNet_AllocSocketSet(30); //Reserve max of 30 socket connections
	SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	server = SDLNet_TCP_Open(&ip);

	



	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
			{
				running = false;
			}

		}

		TCPsocket tmpSocket = SDLNet_TCP_Accept(server);
		if (tmpSocket)
		{
			if (playerNum < 30)
			{
				SDLNet_TCP_AddSocket(sockets, tmpSocket);
				socketVector.push_back(data(tmpSocket, SDL_GetTicks(), curID));
				playerNum++;
				sprintf_s(tmp, "0 %d \n", curID);
				curID++;
				std::cout << "New Connetion" << curID << endl;
			}

			else
			{
				sprintf_s(tmp, "3 \n");
			}

			SDLNet_TCP_Send(tmpSocket, tmp, strlen(tmp) + 1);
		}
		//check for incoming data

		while (SDLNet_CheckSockets(sockets, 0) > 0)
		{
			for (int i = 0; i < socketVector.size(); i++)
			{
				if (SDLNet_SocketReady(socketVector[i].socket))
				{
					socketVector[i].timeout = SDL_GetTicks();
					SDLNet_TCP_Recv(socketVector[i].socket, tmp, 1400);
					int num = tmp[0] - '0';
					int j = 1;

					while (tmp[j] >= '0' && tmp[j] <= '9')
					{
						num *= 10;
						num += tmp[j] - '0';
						j++;
					}

					if (num == 1)
					{
						for (int k = 0; k < socketVector.size(); k++)
						{
							if (k == i)
								continue;
							SDLNet_TCP_Send(socketVector[k].socket, tmp, strlen(tmp) + 1); //Sends data from one socket to different sockets
						}
					}

					else if (num == 2)
					{
						for (int k = 0; k < socketVector.size(); k++)
						{
							if (k == i)
								continue;
							SDLNet_TCP_Send(socketVector[k].socket, tmp, strlen(tmp) + 1); //Sends data from one socket to different sockets, in this case for deletion
						}
						SDLNet_TCP_DelSocket(sockets, socketVector[i].socket);
						SDLNet_TCP_Close(socketVector[i].socket);
						socketVector.erase(socketVector.begin() + i);
						playerNum--;
					}

					else if (num == 3)
					{
						int tmpVar;
						sscanf_s(tmp, "3 %d", &tmpVar);
						for (int k = 0; k < socketVector.size(); k++)
						{
							if (socketVector[k].id == tmpVar)
							{
								SDLNet_TCP_Send(socketVector[k].socket, tmp, strlen(tmp) + 1);
								break;
							}
						}
					}
				}
			}
		}


		//disconnect after timeout
		for (int j = 0; j < socketVector.size(); j++)
		{
			sprintf_s(tmp, "2 %d \n", socketVector[j].id);
			if (SDL_GetTicks() - socketVector[j].timeout > 5000)
			{
				for (int k = 0; k < socketVector.size(); k++)
				{

					SDLNet_TCP_Send(socketVector[k].socket, tmp, strlen(tmp) + 1); //Sends data from one socket to different sockets, in this case for deletion
				}
				SDLNet_TCP_DelSocket(sockets, socketVector[j].socket);
				SDLNet_TCP_Close(socketVector[j].socket);
				socketVector.erase(socketVector.begin() + j);
				playerNum--;
			}
		}

		SDL_Delay(1);
	}
	for (int i = 0; i < socketVector.size(); i++)
	{
		SDLNet_TCP_Close(socketVector[i].socket);
	}
	SDLNet_FreeSocketSet(sockets);
	SDLNet_TCP_Close(server);
	SDLNet_Quit();
	SDL_Quit();

	return 0;
}