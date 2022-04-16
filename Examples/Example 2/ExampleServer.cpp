#include "FServer.h"

struct Pocket
{
	// empty struct for NONE pockets
};

int main()
{

	Server <Pocket> sv("127.0.0.1", 1111); 

	sv.start(); 

	Server <Pocket>::info_pack_signal file; // for file buffer

	sv.set_path_download("C:\\Users\\Temas\\Desktop\\TestS\\"); // set download read dictory

	sv.wait_file(); //sinhrone wait files

	sv >> file;  // read file info

	std::cout << file.from << " " << file.name << " " << file.size_bytes;

	////////////////////////////////////////
	// Work with file and return to client//
	////////////////////////////////////////

	sv << std::pair{ file.name , file.from }; // send edited file

	sv.stop();

}
