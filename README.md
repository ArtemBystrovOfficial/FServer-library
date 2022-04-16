# FServer-library C++17
## About
**FServer-library** - this library will allow users to quickly, conveniently and practically create their own servers together with the client
___
Use the library with maximum simplicity all you need to communicate with clients a couple of lines of code and something else
```C++
struct Pocket;

Server <Pocket> sv("192.128.234.122",1228);
sv.start();

sv << std::pair { Pocket{"Ok","Login"}, id };

sv.stop();
```
## Documentation
Easy to start<br/>
:spiral_notepad:
[***FServer_doc.docx***](https://docs.google.com/document/d/1XeIXLJ9op7A7yuGaVC4ZtFfJ6ryyomB7/edit?usp=sharing&ouid=114316868734239124935&rtpof=true&sd=true)
## Installation
Download headers and use in projects:
Server project
```C++
#include "FServer.h"
```
Client project
```C++
#include "FClient.h"
```
## Examples of code
- :open_file_folder:
  - :file_folder: [Example 1 (Easy classic server with reg)](https://drive.google.com/drive/folders/1oQyTh73ImCRDRsXq-KhkqRJYRacEwZ3Y?usp=sharing)
  - :file_folder: [Example 2 (Work with files)](https://drive.google.com/drive/folders/1j_NFBW2qQFidXhFTgdaZ7HtJpGm6oNiK?usp=sharing)
## Feedback
:email: bystrov.official.one@gmail.com
