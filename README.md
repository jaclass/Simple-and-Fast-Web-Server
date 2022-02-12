# Efficient CGI Implementation

This project presents an optimized way to implement efficient concurrent CGI protocol. The seamless CGI protocol use a fork-and-execute model to fork an independent process, run the CGI scirpts, and redirect the STDOUT to the file descriptor of the accepted socket. The overhead of fork and exec will be significant when there are hundreds of concurrent CGI requests.

Unlike [FastCGI](https://fastcgi-archives.github.io/FastCGI_A_High-Performance_Web_Server_Interface_FastCGI.html) protocol, which requires user to write persistent CGI script, any script written in C with a main() function can be used by server in this project. My implementation apply dynamic linking and prefork thread to achieve efficiency. The perfomance of my CGI implementation is evaluated against a baseline implementation using multi threads fork-and-execute model

## Structure
```
tiny/                               PART1: a baseline implementation using multi threads fork-and-execute model [REF](https://csapp.cs.cmu.edu/3e/ics3/code/netp/tiny/tiny.c)
fast/                               PART2: an optimized CGI implementation 
eval/                               PART3: evaluation script based on [ApacheBench](https://httpd.apache.org/docs/2.2/programs/ab.html)
```

## Usage
1. run the baseline server 
```bash
cd tiny
make
./tiny <port>
```
2. run the optimizied server 
```bash
cd fast
make
./server <port>
```
3. evaluation
Run one of the server listed above, and then
```bash
./eval.sh <port> <path_to_result>
```

## Optimization
1. Linux execv() will first load the executable binary from disk to memory, which would be a big overhead if the CGI scripts is repeatedly executed. To eliminate this cost, I use dynamic link to cache all .so into server's memory when the server is launched. This part is implmented in initialize() function of fast/server.c. 
2. Fork() a new process for every incoming CGI request can be expensive. Thus, the server prefork several persistent processes which continuously call accept() and handle the new request. In my design, serveral process accept one shared listenning socket, but it will not cause concurrency issue when SO_REUSEPORT flag is set in setsockopt() (fast/server.c, line 770). This flag will allow one listening scoket be shared among several processes/threads, and the kernel will automatically balance the load between different processes/threads. This approach can be more efficient than having one main accepting process that distribute the requests to a pool of processes, since the overhead of Intra-Process Communication and additional synchronization are eliminated.
Relative information about SO_REUSEPORT flag in Linux man page:
> SO_REUSEPORT (since Linux 3.9)
> Permits multiple AF_INET or AF_INET6 sockets to be bound
> to an identical socket address.  This option must be set
> on each socket (including the first socket) prior to
> calling bind(2) on the socket.  To prevent port hijacking,
> all of the processes binding to the same address must have
> the same effective UID.  This option can be employed with
> both TCP and UDP sockets.
> 
> For TCP sockets, this option allows accept(2) load
> distribution in a multi-threaded server to be improved by
> using a distinct listener socket for each thread.  This
> provides improved load distribution as compared to
> traditional techniques such using a single accept(2)ing
> thread that distributes connections, or having multiple
> threads that compete to accept(2) from the same socket.
> 
> For UDP sockets, the use of this option can provide better
> distribution of incoming datagrams to multiple processes
> (or threads) as compared to the traditional technique of
> having multiple processes compete to receive datagrams on
> the same socket.
3. I apply multiple preforked processes instead of preforked threads / thread pool, since I want to keep the CGI scripts API the same as baseline. In the baseline server, the input parameters is passed through environment parameters, and the output is redirected from STDOUT to socket. The only feasible way to isolate IO redirections between different CGI request is to use different processes.

## Evaluation
1. Benchmark Machine
'''
vendor_id       : GenuineIntel
cpu family      : 6
model           : 158
model name      : Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz
cpu MHz         : 2801.000
cache size      : 256 KB
siblings        : 8
cpu cores       : 4
memory size     : 16GB
'''
2. Local Evaluation
I do all the evaluations locally. The main overhead I want to eliminate is the resource waste of fork-and-execute model for every CGI request, which is irrelevant to network transmission. Thus, local evaluation with simple CGI script and high concurrency makes sense in the context of this project.
3. Evaluation method: 
After the launch the server, the evaluation request will generate workload to the server with increasing concurrency (from 1 - 120), and the latency (waiting time for every request) and throughtput (amount of data per second) measured on the client side will be recorded.
4. Results:
Evaluation has be done on baseline server and optimized server. Shown in eval/plot_latency.png and eval/plot_throughput.png, the optimized CGI server scales well on the number of concurrency. For optimizied server, I have compare the performance of 4 preforked process and 8 preforked process, and I find that the server with 4 preforked processes has better perfromance. This makes sense since my benchmark machines has 4 cores, and there is very little IO blocking during the benchmark (The CGI scripts are called from the memory & server are busy handling huge number of concurrent requets).

## Limitation
1. The server only cache all the CGI scripts through dynamic linking in the initialization. This means all the script adding to /cgi-bin after the server is launched cannot be found by the server.
2. The evaluation is only done on my local machine, and the requests will be dropped if there are more than 200 concurrent clients. 