# Simple File Server 
> Made for a college project
----------------------------------------------------------
When you need to transfer a file from a linux computer to another you now have a simple file sharing server using the HTTP protocol, made in c and easy to use.


## Get started
Compile the server using:
```shell
gcc -pthread httpS.c -o http
```
Then run:
```shell
sudo ./http
```

## Usage
The server can share all your files in the current directory.

Just go in another computer's browser and type the IP of your computer running the server plus "/filename.ext".
