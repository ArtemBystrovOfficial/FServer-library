#include "FClient.h"

struct Pocket
{

	enum class Command
	{
		Register,
		Login,
		Msg,
		Send_All,
		Ok,
		Error
	};

	Pocket() {};

	Pocket(Command&& com, const std::string& msg_s,
		const std::string& login_s, const std::string& password_s) : command(com)
	{
		strcpy(msg, msg_s.c_str());
		strcpy(login, login_s.c_str());
		strcpy(password, password_s.c_str());
	};


	Command command;
	char msg[256];
	char login[32];
	char password[32];

};

int main()
{
	Client <Pocket> cl("127.0.0.1", 1111);

	cl.connect_to_server();

	int mode = 0;

	Pocket pocket;

	std::string msg, login, password;

	while (!cl.is_client_disconected())
	{
		std::cout << "0 - for Reg, 1 - for Login" << std::endl;
		std::cin >> mode;
		std::cout << "Login:\n";
		std::cin >> login;
		std::cout << "Password:\n";
		std::cin >> password;

		if (login.size() > 254 || password.size() > 254)
			throw std::exception("Login or pasword so long");

		if (mode)
		{//Login

			// Server -> client client -> Server
			cl << Pocket(Pocket::Command::Login, "", login, password);
			cl >> pocket;

			switch (pocket.command)
			{
				case(Pocket::Command::Error):
				{
					std::cout << "Error: " << pocket.msg << std::endl;
				} break;
				case(Pocket::Command::Ok):
				{
					std::cout << "Accept. Welcome\n";
					Sleep(1000);
					system("cls");

					std::thread send([&]() {

						Pocket pocket_in;

						while (!cl.is_client_disconected())
						{
							cl >> pocket_in;

							switch (pocket_in.command)
							{
								case(Pocket::Command::Msg):
								{
									if(!cl.is_client_disconected())
									std::cout << pocket_in.msg << std::endl;
								}
							}
						}
					});

					while (!cl.is_client_disconected())
					{
						Pocket data;
						std::cin >> data.msg;
						data.command = Pocket::Command::Send_All;
						cl << data;
					}
					send.join();
				} break;
			}
		}
		else
		{//Registr

			// Server -> client client -> Server
			cl << Pocket(Pocket::Command::Register, "", login, password);
			cl >> pocket;

			switch (pocket.command)
			{
				case(Pocket::Command::Error):
				{
					std::cout << "Error: " << pocket.msg << std::endl;
				} break;
				case(Pocket::Command::Ok):
				{
					std::cout << "Registered" << std::endl;
				} break;
			}
		}
	}

	cl.disconnect();
}
