#include "FServer.h"

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
		strcpy(login, msg_s.c_str());
		strcpy(password, msg_s.c_str());
	};


	Command command;
	char msg[256];
	char login[32];
	char password[32];

};

int main()
{
	enum class Group
	{
		online
	};

	using group = Server<Pocket>::Number_of_group;

	Server <Pocket> sv("127.0.0.1", 1111);

	sv.start();

	std::pair <Pocket, int> data;

	std::map  < std::string, std::string > data_u;

	std::map  <int, bool> is_login_u;

	sv.add_group( static_cast<int>(Group::online) );

	while (!sv.is_server_stoped())
	{
		sv >> data;
		auto [pocket, id] = data;

		switch (pocket.command)
		{
			case(Pocket::Command::Register):
			{
				auto is_be = data_u.find(pocket.login);
				if (is_be == data_u.end())
				{
					data_u[pocket.login] = pocket.password;
					sv << std::pair{ Pocket(Pocket::Command::Ok,"","",""),id };
					break;
				}

				sv << std::pair{ Pocket{Pocket::Command::Error,"Login exsist","",""},id };

			} break;
			case(Pocket::Command::Login):
			{
				auto password = data_u.find(pocket.login);
				if (password != data_u.end())
				{
					if (password->second == pocket.password)
					{
						is_login_u[id] = true;
						sv.add_to_group(static_cast<int>(Group::online), id);
						sv << std::pair{ Pocket(Pocket::Command::Ok,"","",""),id };
						break;
					}
				}

				sv << std::pair{ Pocket{Pocket::Command::Error,"uncorrect login or password","",""}, id };

			} break;
			case(Pocket::Command::Send_All):
			{
				auto connect = is_login_u.find(id);
				if (connect != is_login_u.end())
				{
					if (connect->second)
					{
						sv << std::pair{ Pocket(Pocket::Command::Msg,pocket.msg,"",""), 
														group(static_cast<int>(Group::online)) };
						break;
					}
				}

				sv << std::pair{ Pocket{Pocket::Command::Error,"You don't auntification","",""}, id };

			} break;
		}
	}

	sv.stop();

}
