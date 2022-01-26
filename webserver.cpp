#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

const int port = 80;
const int maxConnectionRequests = 500;
const bool cacheIndexHTML = true;

std::string logTime(){
    std::time_t result = std::time(nullptr);
    return '[' + static_cast<std::string>(std::ctime(&result)).substr(4, 20) + "] ";
}

std::string findContentType(std::string filepath){
    while(filepath.find("?") != std::string::npos){
            filepath.erase(filepath.find("?"), filepath.length() );
        }

    if(filepath.substr(filepath.find(".") + 1, filepath.length() - 1) == "png"  ){
        return "Content-Type: image/png;";
    }
    else if(filepath.substr(filepath.find(".") + 1, filepath.length() - 1) == "jpg"){
        return "Content-Type: image/jpeg;";
    }
    else if(filepath.substr(filepath.find(".") + 1, filepath.length() - 1) == "mp4"){
        return "Content-Type: video/mp4;";
    }
    else if(filepath.substr(filepath.find(".") + 1, filepath.length() - 1) == "html" || filepath == "/"  ){
        return "text/html; charset=UTF-8";
    }
    else if(filepath.substr(filepath.find(".") + 1, filepath.length() - 1) == "txt"  ){
        return "text/plain; charset=UTF-8";
    }
    return "application/octet-stream; charset=UTF-8";
}

std::string readFile(std::string filename){
    std::stringstream filedata;
    if(filename == "/"){
        filename = "webserver/index.html"; //index.html is used as the homepage
        std::ifstream file(filename.c_str());
        filedata << file.rdbuf();
    }
    else{
        if(filename.at(0) == '/'){
            filename.erase(0, 1);
        }
        
        while(filename.find("%20") != std::string::npos){
            filename.replace(filename.find("%20"), 3, " ");
        }
        
        while(filename.find("%5F") != std::string::npos){
            filename.replace(filename.find("%5F"), 3, "_");
        }

        while(filename.find("%2D") != std::string::npos){
            filename.replace(filename.find("%2D"), 3, "-");
        }

        while(filename.find("%28") != std::string::npos){
            filename.replace(filename.find("%28"), 3, "(");
        }
        
        while(filename.find("%29") != std::string::npos){
            filename.replace(filename.find("%29"), 3, ")");
        }

        while(filename.find("?") != std::string::npos){
            filename.erase(filename.find("?"), filename.length() );
        }
        filename = "webserver/" + filename; //all public files should be inside the /webserver folder
        std::ifstream file(filename.c_str());
        filedata << file.rdbuf();

        if(filedata.str().empty()){
        std::ifstream file("404.html");
        filedata << file.rdbuf();
        }
    }
    return filedata.str();
}

std::string respondToGET(std::string filepath, bool cacheIndexHTML, std::string* index){
    std::string response = "HTTP/1.1 ";
    std::string file;

    if(cacheIndexHTML && filepath == "/"){
        file = *index;
    }
    else {
        file = readFile(filepath);
    }

    if (file.empty()){
        std::cout << "404 Not Found" << std::endl;
        response += "404 Not Found";
    }
    else{
        std::cout << "200 OK" << std::endl;
        response += "200 OK\n";
        response += "Content-Type: ";
        response += findContentType(filepath);
        response += "\r\n";

        int length = file.size();
        response += "Content-Length: ";
        response += std::to_string(length);
        response += "\n\n";
        response += file;
    }
    return response;
}

int main(){
    std::cout << logTime() <<  "Starting server\n";
    int server_fd = 0;
    int newSocket = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; // htonl converts a long integer (e.g. address) to a network representation INADDR_ANY chooses any IP, WiFi or Ethernet. 
    address.sin_port = htons(port); /* htons converts a short integer (e.g. port) to a network representation */ 
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ //AF_INET = IPv4
        std::cerr << logTime() << "Cannot create socket\n";
        return 1;
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        std::cerr << logTime() << "Cannot bind socket\n";
        return 2;
    }
    
    if(listen(server_fd, maxConnectionRequests) != 0){
        std::cerr << logTime() << "Cannot listen to socket\n";
        return 3;
    }
    std::string* index = new std::string;

    if(cacheIndexHTML){
        std::string temp = readFile("/"); //read index
        *index = temp;
    }
    else{
        delete index;
    }

    while (true){
        if ((newSocket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
            std::cerr << logTime() << "Connection refused\n";
            return 4;
        }	
        const size_t buffersize = 524288;	//somehow larger requests stop at 524288 bytes
        char* buffer = new char[buffersize] {0};
        int usedbuffersize = recv(newSocket, buffer, buffersize, 0);
        std::string request = std::string(buffer);
        std::string response = "HTTP/1.1 ";
        std::cout << "incoming request: " << request <<  " with size: " << usedbuffersize << std::endl;
        
        if (request.length() > 4){ //make sure that the request is long enough
            std::string filepath = request.substr(4, request.find(" ", 4) - 4);
            std::cout << logTime() << "Requested: " << filepath << " Responding: ";

            if(request.substr(0,3) == "GET"){
                std::string response = respondToGET(filepath, cacheIndexHTML, index);
                send(newSocket, response.c_str(), response.length(), 0);
            }
            else {
                std::cout << "501 Unsupported" << std::endl;
                response += "501 Unsupported";
                send(newSocket, response.c_str(), response.length(), 0);
            }
            delete buffer;
            close (newSocket);
        }
    }
}
