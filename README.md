# TFTP-server-client
### Ivan Golikov (xgolik00)

## Task
The task is to implement a client and server application for File Transfer via TFTP ([Trivial File Transfer Protocol](https://datatracker.ietf.org/doc/html/rfc1350)) exactly according to the corresponding RFC specification of the given protocol.

In addition, the resulting solutions must comply with the following extensions of the basic specification of the TFTP protocol:

- [TFTP Option Extension](https://datatracker.ietf.org/doc/html/rfc2347)
- [TFTP Blocksize Option](https://datatracker.ietf.org/doc/html/rfc2348)
- [TFTP Timeout Interval and Transfer Size Options](https://datatracker.ietf.org/doc/html/rfc2349)

## Compilation

Compilation is possible by means of a make file using the command:
`make` - compilate all
`make client` - compilate only client size
`make server` - compilate only server size

## Run

The input parameters for the server are as follows:

> tftp-server [-p port] root_dirpath
> * -p the local port on which the server will wait for an incoming connection
> * root_dirpath - the path to the directory under which incoming files will be stored

The input parameters for the client are as follows:

> tftp-client -h hostname [-p port] [-f filepath] -t filepath
> * -h IP address/domain name of the remote server
> * -p port of remote server
>   * if not specified default is assumed according to specification
> * -f path to the downloaded file on the server (download)
>   * if not specified, stdin (upload)content is used
> * -t the path under which the file will be stored on a remote server/locally

