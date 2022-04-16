#include "FClient.h"

struct Pocket
{
	// empty struct for NONE pockets
};

int main()
{
	Client <Pocket> cl("127.0.0.1", 1111);

	cl.connect_to_server(); // connect

	cl.set_path_download("C:\\Users\\Temas\\Desktop\\Client2\\"); // set dowload directory

	cl << "C:\\Users\\Temas\\Desktop\\Client\\Death+Valley-0066.jpg"; // send file

	cl.wait_file(); // wait file

	cl.disconnect(); // disconnect
}
